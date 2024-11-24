#include "expose_metrics.h"

extern bool cpu_enabled;
extern bool memory_enabled;
extern bool battery_enabled;
extern bool avg_load_enabled;
extern bool cpu_temp_enabled;
extern bool cpu_speed_enabled;
extern bool processes_enabled;
extern bool sys_calls_enabled;
extern bool disk_io_enabled;
extern bool network_enabled;

/** Mutex para sincronización de hilos */
pthread_mutex_t lock;

/** Magic number enums for stat recognition */
enum diskstats
{
    READS,
    WRITES,
    IO_IN_PROGRESS
};
enum networkstats
{
    RX_ERRORS,
    TX_ERRORS,
    RX_DROPS,
    TX_DROPS
};
enum processstats
{
    PROCESSES,
    CONTEXT_SWITCHES
};
enum memstats
{
    TOTAL,
    FREE,
    AVAIL
};

/** Métrica de Prometheus para el uso de CPU */
static prom_gauge_t* cpu_usage_metric;
/** Métrica de Prometheus para el uso porcentual de memoria */
static prom_gauge_t* memory_usage_metric;
/** Métrica de Prometheus para la memoria usada */
static prom_gauge_t* memory_used_metric;
/** Métrica de Prometheus para la memoria disponible */
static prom_gauge_t* memory_available_metric;
/** Metrica de Prometheeus para el nivel de bateria */
static prom_gauge_t* battery_level_metric;
/** Metrica de Prometheus para el numero de sys calls del proceso*/
static prom_counter_t* sys_calls_metric;
/** Metrica de Prometheus para la temperatura de CPU */
static prom_gauge_t* cpu_temp_metric;
/** Metrica de Prometheus para el numero de procesos en ejecucion en el sistema */
static prom_gauge_t* number_of_processes_metric;
/** Metrica de Prometheus para la carga promedio del sistema */
static prom_gauge_t* avg_load_metric;
/** Metrica de Prometheus para la velocidad de la CPU en MHz*/
static prom_gauge_t* cpu_speed_metric;
/** Metrica de Prometheus para la cantidad de lecturas de disco */
static prom_gauge_t* disk_reads_metric;
/** Metrica de Prometheus para la cantidad de escrituras de disco */
static prom_gauge_t* disk_writes_metric;
/** Metrica de Prometheus para la cantidad de operaciones de I/O en progreso */
static prom_gauge_t* disk_io_inprogress_metric;
/** Metrica de Prometheus para la cantidad de paquetes transmitidos descartados */
static prom_gauge_t* network_tx_drops_metric;
/** Metrica de Prometheus para la cantidad de errores en paquetes transmitidos */
static prom_gauge_t* network_tx_errors_metric;
/** Metrica de Prometheus para la cantidad de paquetes recibidos descartados */
static prom_gauge_t* network_rx_drops_metric;
/** Metrica de Prometheus para la cantidad de errores en paquetes recibidos */
static prom_gauge_t* network_rx_errors_metric;
/** Metrica de Prometheus para la cantidad de cambios de contexto */
static prom_gauge_t* context_changes_metric;

void update_cpu_gauge()
{
    double usage = get_cpu_usage();
    if (usage >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(cpu_usage_metric, usage, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener el uso de CPU\n");
    }
}

void update_network_gauge()
{
    unsigned long* network_stats = get_network_stats();
    if (network_stats == NULL)
    {
        return;
    }

    unsigned long rx_errors = network_stats[RX_ERRORS];
    unsigned long tx_errors = network_stats[TX_ERRORS];
    unsigned long rx_drops = network_stats[RX_DROPS];
    unsigned long tx_drops = network_stats[TX_DROPS];

    prom_gauge_set(network_rx_errors_metric, rx_errors, NULL);
    prom_gauge_set(network_tx_errors_metric, tx_errors, NULL);
    prom_gauge_set(network_rx_drops_metric, rx_drops, NULL);
    prom_gauge_set(network_tx_drops_metric, tx_drops, NULL);
}

void update_memory_metrics()
{
    unsigned long* memory_info = get_memory_usage();
    if (memory_info == NULL)
    {
        return;
    }

    unsigned long total_memory = memory_info[TOTAL];
    unsigned long free_memory = memory_info[FREE];
    unsigned long available_memory = memory_info[AVAIL];
    unsigned long used_memory = total_memory - free_memory;
    unsigned long used_memory_percentage = used_memory * 100 / total_memory;

    prom_gauge_set(memory_usage_metric, used_memory_percentage, NULL);
    prom_gauge_set(memory_used_metric, used_memory, NULL);
    prom_gauge_set(memory_available_metric, available_memory, NULL);
}

void update_disk_io_gauge()
{
    unsigned long* disk_io = get_disk_io();
    if (disk_io == NULL)
    {
        return;
    }
    unsigned long reads = disk_io[0];
    unsigned long writes = disk_io[1];
    unsigned long ioinprogress = disk_io[2];
    prom_gauge_set(disk_reads_metric, reads, NULL);
    prom_gauge_set(disk_writes_metric, writes, NULL);
    prom_gauge_set(disk_io_inprogress_metric, ioinprogress, NULL);
}

void update_battery_gauge()
{
    double battery_level = get_battery_level();
    if (battery_level >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(battery_level_metric, battery_level, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener el nivel de batería\n");
    }
}

void* expose_metrics(void* arg)
{
    (void)arg; // Argumento no utilizado

    // Aseguramos que el manejador HTTP esté adjunto al registro por defecto
    promhttp_set_active_collector_registry(NULL);

    // Iniciamos el servidor HTTP en el puerto 8000
    struct MHD_Daemon* daemon = promhttp_start_daemon(MHD_USE_SELECT_INTERNALLY, 8000, NULL, NULL);
    if (daemon == NULL)
    {
        fprintf(stderr, "Error al iniciar el servidor HTTP\n");
        return NULL;
    }

    // Mantenemos el servidor en ejecución
    while (1)
    {
        sleep(1);
    }

    // Nunca debería llegar aquí
    MHD_stop_daemon(daemon);
    return NULL;
}

int init_metrics()
{
    // Inicializamos el mutex
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        fprintf(stderr, "Error al inicializar el mutex\n");
        return EXIT_FAILURE;
    }

    // Inicializamos el registro de coleccionistas de Prometheus
    if (prom_collector_registry_default_init() != 0)
    {
        fprintf(stderr, "Error al inicializar el registro de Prometheus\n");
        return EXIT_FAILURE;
    }

    // Creamos la métrica para el uso de CPU
    cpu_usage_metric = prom_gauge_new("cpu_usage_percentage", "Porcentaje de uso de CPU", 0, NULL);
    if (cpu_usage_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de uso de CPU\n");
        return EXIT_FAILURE;
    }

    // Creamos las métrica para el uso de memoria
    memory_usage_metric = prom_gauge_new("memory_usage_metric", "Porcentaje de memoria en uso", 0, NULL);
    if (memory_usage_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de en uso\n");
        return EXIT_FAILURE;
    }
    memory_available_metric = prom_gauge_new("memory_available", "Memoria disponible", 0, NULL);
    if (memory_available_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de memoria disponible\n");
        return EXIT_FAILURE;
    }
    memory_used_metric = prom_gauge_new("memory_used", "Memoria usada", 0, NULL);
    if (memory_used_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de memoria usada\n");
        return EXIT_FAILURE;
    }

    battery_level_metric = prom_gauge_new("battery_level", "Nivel de batería", 0, NULL);
    if (battery_level_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de nivel de batería\n");
        return EXIT_FAILURE;
    }

    sys_calls_metric = prom_counter_new("sys_calls", "Número de sys calls", 0, NULL);
    if (sys_calls_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de sys calls\n");
        return EXIT_FAILURE;
    }

    cpu_temp_metric = prom_gauge_new("cpu_temp", "Temperatura de la CPU", 0, NULL);
    if (cpu_temp_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de temperatura de la CPU\n");
        return EXIT_FAILURE;
    }

    number_of_processes_metric = prom_gauge_new("number_of_processes", "Número de procesos en ejecución", 0, NULL);
    if (number_of_processes_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de número de procesos en ejecución\n");
        return EXIT_FAILURE;
    }

    avg_load_metric = prom_gauge_new("avg_load", "Carga promedio", 0, NULL);
    if (avg_load_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de carga promedio\n");
        return EXIT_FAILURE;
    }

    cpu_speed_metric = prom_gauge_new("cpu_speed", "Velocidad de la CPU", 0, NULL);
    if (cpu_speed_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de velocidad de la CPU\n");
        return EXIT_FAILURE;
    }
    network_tx_drops_metric = prom_gauge_new("network_tx_drops", "Paquetes transmitidos descartados", 0, NULL);
    if (network_tx_drops_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de paquetes transmitidos descartados\n");
        return EXIT_FAILURE;
    }
    network_tx_errors_metric = prom_gauge_new("network_tx_errors", "Errores en paquetes transmitidos", 0, NULL);
    if (network_tx_errors_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de errores en paquetes transmitidos\n");
        return EXIT_FAILURE;
    }
    network_rx_drops_metric = prom_gauge_new("network_rx_drops", "Paquetes recibidos descartados", 0, NULL);
    if (network_rx_drops_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de paquetes recibidos descartados\n");
        return EXIT_FAILURE;
    }
    network_rx_errors_metric = prom_gauge_new("network_rx_errors", "Errores en paquetes recibidos", 0, NULL);
    if (network_rx_errors_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de errores en paquetes recibidos\n");
        return EXIT_FAILURE;
    }
    disk_writes_metric = prom_gauge_new("disk_writes", "Escrituras de disco", 0, NULL);
    if (disk_writes_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de escrituras de disco\n");
        return EXIT_FAILURE;
    }
    disk_reads_metric = prom_gauge_new("disk_reads", "Lecturas de disco", 0, NULL);
    if (disk_reads_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de lecturas de disco\n");
        return EXIT_FAILURE;
    }
    disk_io_inprogress_metric = prom_gauge_new("disk_io_inprogress", "Operaciones de disco en progreso", 0, NULL);
    if (disk_io_inprogress_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de operaciones de disco en progreso\n");
        return EXIT_FAILURE;
    }
    context_changes_metric = prom_gauge_new("context_changes", "Cambios de contexto", 0, NULL);
    if (context_changes_metric == NULL)
    {
        fprintf(stderr, "Error al crear la métrica de cambios de contexto\n");
        return EXIT_FAILURE;
    }
    // Registramos las métricas en el registro por defecto
    if(cpu_enabled){
        prom_collector_registry_must_register(cpu_usage_metric);
    }
    if(memory_enabled){
        prom_collector_registry_must_register(memory_usage_metric);
        prom_collector_registry_must_register(memory_used_metric);
        prom_collector_registry_must_register(memory_available_metric);
    }
    if(cpu_speed_enabled){
        prom_collector_registry_must_register(cpu_speed_metric);
    }
    if(avg_load_enabled){
        prom_collector_registry_must_register(avg_load_metric);
    }
    if(cpu_temp_enabled){
        prom_collector_registry_must_register(cpu_temp_metric);
    }
    if(processes_enabled){
        prom_collector_registry_must_register(number_of_processes_metric);
        prom_collector_registry_must_register(context_changes_metric);
    }
    if(sys_calls_enabled){
        prom_collector_registry_must_register(sys_calls_metric);
    }
    if(battery_enabled){
        prom_collector_registry_must_register(battery_level_metric);
    }
    if(disk_io_enabled){
        prom_collector_registry_must_register(disk_reads_metric);
        prom_collector_registry_must_register(disk_writes_metric);
        prom_collector_registry_must_register(disk_io_inprogress_metric);
    }
    if(network_enabled){
        prom_collector_registry_must_register(network_tx_drops_metric);
        prom_collector_registry_must_register(network_tx_errors_metric);
        prom_collector_registry_must_register(network_rx_drops_metric);
        prom_collector_registry_must_register(network_rx_errors_metric);
    }
    return 0;
}
// Function for prom collector registry
// creo una funcion para no repetir el codigo para cada metrica, ademas cambio la verificacion de error a == null porque
// segun la documentacion la funcion devuelve el prom_metric_t* registrado
int prom_collector_registry_must_register(prom_gauge_t* prom_gauge)
{
    if (prom_collector_registry_must_register_metric(prom_gauge) == NULL)
    {
        fprintf(stderr, "Error al registrar las métricas\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void update_cpu_temp_gauge()
{
    double cpu_temp = get_cpu_temp();
    if (cpu_temp >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(cpu_temp_metric, cpu_temp, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener la temperatura de la CPU\n");
    }
}

void update_cpu_speed_gauge()
{
    double cpu_speed = get_cpu_speed();
    if (cpu_speed >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(cpu_speed_metric, cpu_speed, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener la velocidad de la CPU\n");
    }
}

void update_avg_load_gauge()
{
    double avg_load = get_avg_load();
    if (avg_load >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(avg_load_metric, avg_load, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener la carga promedio\n");
    }
}

void update_processes_gauges()
{
    unsigned long* processes_info = get_number_of_processes();
    if (processes_info == NULL)
    {
        return;
    }

    unsigned long processes = processes_info[0];
    unsigned long context_switches = processes_info[1];
    pthread_mutex_lock(&lock);
    prom_gauge_set(number_of_processes_metric, processes, NULL);
    prom_gauge_set(context_changes_metric, context_switches, NULL);
    pthread_mutex_unlock(&lock);
}
void update_sys_calls_gauge()
{
    double sys_calls = get_sys_calls();
    if (sys_calls >= 0)
    {
        pthread_mutex_lock(&lock);
        prom_counter_inc(sys_calls_metric, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error al obtener el número de sys calls\n");
    }
}

void destroy_mutex()
{
    pthread_mutex_destroy(&lock);
}
