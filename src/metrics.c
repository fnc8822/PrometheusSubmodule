#include "metrics.h"

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
/** Define el tamaño del arreglo de variables que se extrae de /proc/meminfo  */
#define MEMORY_EXTRACT_VARIABLES 3
/** Define el tamaño del arreglo de variables que se extrae de /proc/diskstats  */
#define DISK_IO_EXTRACT_VARIABLES 3
/** Define el tamaño del arreglo de variables que se extrae de /proc/net/dev  */
#define NETWORK_STATS_EXTRACT_VARIABLES 4

unsigned long* get_memory_usage()
{
    static unsigned long memory_info[MEMORY_EXTRACT_VARIABLES];
    FILE* fp = fopen("/proc/meminfo", "r");
    if (fp == NULL)
    {
        perror("Error opening /proc/meminfo");
        return NULL;
    }

    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), fp))
    {
        if (sscanf(line, "MemTotal: %lu kB", &memory_info[TOTAL]) == 1)
        {
            continue;
        }
        if (sscanf(line, "MemFree: %lu kB", &memory_info[FREE]) == 1)
        {
            continue;
        }
        if (sscanf(line, "MemAvailable: %lu kB", &memory_info[AVAIL]) == 1)
        {
            continue;
        }
    }

    fclose(fp);
    return memory_info;
}

unsigned long* get_disk_io()
{
    static unsigned long disk_io[DISK_IO_EXTRACT_VARIABLES]; // Array to hold read, write, and io_currently_in_progress
    FILE* fp = fopen("/proc/diskstats", "r");
    if (fp == NULL)
    {
        perror("Error opening /proc/diskstats");
        return NULL;
    }

    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), fp))
    {
        unsigned long r, w, io_in_progress;
        if (sscanf(line, "%*d %*d %*s %lu %*u %*u %*u %lu %*u %*u %*u %lu", &r, &w, &io_in_progress) == 3)
        {
            disk_io[READS] += r;
            disk_io[WRITES] += w;
            disk_io[IO_IN_PROGRESS] += io_in_progress;
        }
    }

    fclose(fp);
    return disk_io;
}

unsigned long* get_network_stats()
{
    static unsigned long
        network_stats[NETWORK_STATS_EXTRACT_VARIABLES]; // Array to hold rx_errors, tx_errors, rx_drops, and tx_drops
    FILE* fp = fopen("/proc/net/dev", "r");
    if (fp == NULL)
    {
        perror("Error opening /proc/net/dev");
        return NULL;
    }

    char line[BUFFER_SIZE];
    // Skip the first two lines
    if (fgets(line, sizeof(line), fp) == NULL)
    {
        fclose(fp);
        return NULL;
    }
    if (fgets(line, sizeof(line), fp) == NULL)
    {
        fclose(fp);
        return NULL;
    }

    while (fgets(line, sizeof(line), fp))
    {
        unsigned long rx_errors, tx_errors, rx_drops, tx_drops;
        if (sscanf(line, "%*s %*u %*u %lu %*u %*u %*u %*u %*u %*u %*u %lu %*u %*u %*u %*u %*u %lu %lu", &rx_errors,
                   &tx_errors, &rx_drops, &tx_drops) == 4)
        {
            network_stats[RX_ERRORS] += rx_errors;
            network_stats[TX_ERRORS] += tx_errors;
            network_stats[RX_DROPS] += rx_drops;
            network_stats[TX_DROPS] += tx_drops;
        }
    }

    fclose(fp);
    return network_stats;
}

double get_cpu_usage()
{
    static unsigned long long prev_user = 0, prev_nice = 0, prev_system = 0, prev_idle = 0, prev_iowait = 0,
                              prev_irq = 0, prev_softirq = 0, prev_steal = 0;
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    unsigned long long totald, idled;
    double cpu_usage_percent;

    // Abrir el archivo /proc/stat
    FILE* fp = fopen("/proc/stat", "r");
    if (fp == NULL)
    {
        perror("Error al abrir /proc/stat");
        return -1.0;
    }

    char buffer[BUFFER_SIZE * 4];
    if (fgets(buffer, sizeof(buffer), fp) == NULL)
    {
        perror("Error al leer /proc/stat");
        fclose(fp);
        return -1.0;
    }
    fclose(fp);

    // Analizar los valores de tiempo de CPU
    int ret = sscanf(buffer, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu", &user, &nice, &system, &idle, &iowait,
                     &irq, &softirq, &steal);
    if (ret < 8)
    {
        fprintf(stderr, "Error al parsear /proc/stat\n");
        return -1.0;
    }

    // Calcular las diferencias entre las lecturas actuales y anteriores
    unsigned long long prev_idle_total = prev_idle + prev_iowait;
    unsigned long long idle_total = idle + iowait;

    unsigned long long prev_non_idle = prev_user + prev_nice + prev_system + prev_irq + prev_softirq + prev_steal;
    unsigned long long non_idle = user + nice + system + irq + softirq + steal;

    unsigned long long prev_total = prev_idle_total + prev_non_idle;
    unsigned long long total = idle_total + non_idle;

    totald = total - prev_total;
    idled = idle_total - prev_idle_total;

    if (totald == 0)
    {
        fprintf(stderr, "Totald es cero, no se puede calcular el uso de CPU!\n");
        return -1.0;
    }

    // Calcular el porcentaje de uso de CPU
    cpu_usage_percent = ((double)(totald - idled) / totald) * 100.0;

    // Actualizar los valores anteriores para la siguiente lectura
    prev_user = user;
    prev_nice = nice;
    prev_system = system;
    prev_idle = idle;
    prev_iowait = iowait;
    prev_irq = irq;
    prev_softirq = softirq;
    prev_steal = steal;

    return cpu_usage_percent;
}

double get_battery_level()
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    double battery_level = 0.0;

    // Abrir el archivo /sys/class/power_supply/BAT0/capacity
    fp = fopen("/sys/class/power_supply/BAT0/capacity", "r");
    if (fp == NULL)
    {
        perror("Error al abrir /sys/class/power_supply/BAT0/capacity");
        return -1.0;
    }

    // Leer el nivel de batería
    if (fgets(buffer, sizeof(buffer), fp) == NULL)
    {
        perror("Error al leer el nivel de batería");
        fclose(fp);
        return -1.0;
    }
    fclose(fp);

    // Convertir el nivel de batería a un número
    battery_level = strtod(buffer, NULL);
    if (battery_level < 0)
    {
        fprintf(stderr, "Error al convertir el nivel de batería\n");
        return -1.0;
    }

    return battery_level;
}

double get_avg_load()
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    double avg_load = 0.0;

    // Abrir el archivo /proc/loadavg
    fp = fopen("/proc/loadavg", "r");
    if (fp == NULL)
    {
        perror("Error al abrir /proc/loadavg");
        return -1.0;
    }

    // Leer la carga promedio
    if (fgets(buffer, sizeof(buffer), fp) == NULL)
    {
        perror("Error al leer la carga promedio");
        fclose(fp);
        return -1.0;
    }
    fclose(fp);

    // Convertir la carga promedio a un número
    avg_load = strtod(buffer, NULL);
    if (avg_load < 0)
    {
        fprintf(stderr, "Error al convertir la carga promedio\n");
        return -1.0;
    }

    return avg_load;
}

double get_cpu_speed()
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    double cpu_speed = 0.0;

    // Abrir el archivo /proc/cpuinfo
    fp = fopen("/proc/cpuinfo", "r");
    if (fp == NULL)
    {
        perror("Error al abrir /proc/cpuinfo");
        return -1.0;
    }

    // Leer la velocidad de la CPU
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        if (strstr(buffer, "cpu MHz") != NULL)
        {
            sscanf(buffer, "cpu MHz : %lf", &cpu_speed);
            break;
        }
    }
    fclose(fp);

    // Verificar si se encontró la velocidad de la CPU
    if (cpu_speed == 0.0)
    {
        fprintf(stderr, "Error al leer la velocidad de la CPU desde /proc/cpuinfo\n");
        return -1.0;
    }

    return cpu_speed;
}

double get_sys_calls()
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    int sys_calls = 0;

    // Abrir el archivo /proc/self/status
    fp = fopen("/proc/self/status", "r");
    if (fp == NULL)
    {
        perror("Error al abrir /proc/self/status");
        return -1;
    }

    // Leer el número de sys calls
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        if (sscanf(buffer, "voluntary_ctxt_switches: %d", &sys_calls) == 1)
        {
            break; // voluntary_ctxt_switches encontrado
        }
    }
    fclose(fp);

    // Verificar si se encontró el número de sys calls
    if (sys_calls == 0)
    {
        fprintf(stderr, "Error al leer el número de sys calls desde /proc/self/status\n");
        return -1;
    }

    return sys_calls;
}

double get_cpu_temp()
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    double cpu_temp = 0.0;

    // Abrir el archivo /sys/class/thermal/thermal_zone0/temp
    fp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    if (fp == NULL)
    {
        perror("Error al abrir /sys/class/thermal/thermal_zone0/temp");
        return -1.0;
    }

    // Leer la temperatura de la CPU
    if (fgets(buffer, sizeof(buffer), fp) == NULL)
    {
        perror("Error al leer la temperatura de la CPU");
        fclose(fp);
        return -1.0;
    }
    fclose(fp);

    // Convertir la temperatura de la CPU a un número
    cpu_temp = strtod(buffer, NULL);
    if (cpu_temp < 0)
    {
        fprintf(stderr, "Error al convertir la temperatura de la CPU\n");
        return -1.0;
    }

    // Convertir la temperatura de la CPU de miligrados a grados Celsius
    cpu_temp /= 1000.0;

    return cpu_temp;
}

unsigned long* get_number_of_processes()
{
    static unsigned long processes_info[2]; // Array to hold number of processes and context switches
    FILE* fp = fopen("/proc/stat", "r");
    if (fp == NULL)
    {
        perror("Error opening /proc/stat");
        return NULL;
    }

    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), fp))
    {
        if (sscanf(line, "processes %lu", &processes_info[0]) == 1)
        {
            continue;
        }
        if (sscanf(line, "ctxt %lu", &processes_info[1]) == 1)
        {
            continue;
        }
    }

    fclose(fp);
    return processes_info;
}
