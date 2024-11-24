/**
 * @file expose_metrics.h
 * @brief Programa para leer el uso de CPU y memoria y exponerlos como métricas de Prometheus.
 */

#include "metrics.h"
#include <errno.h>
#include <prom.h>
#include <promhttp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // Para sleep
#include <stdbool.h>
/**
 * @brief Tamaño del buffer para leer archivos.
 */
#define BUFFER_SIZE 256
/**
 * @brief Function to register any metric in the default collector registry
 * @param prom_gauge The metric to register as prom_gauge_t*
 * @return A non-zero integer value upon failure
 */
int prom_collector_registry_must_register(prom_gauge_t* prom_gauge);
/**
 * @brief Actualiza la métrica de uso de CPU.
 */
void update_cpu_gauge();

/**
 * @brief Actualiza las métrica de uso de memoria.
 */
void update_memory_metrics();

/**
 * @brief Actualiza la métrica de nivel de batería.
 */
void update_battery_gauge();

/**
 * @brief Actualiza la métrica de la temperatura de cpu.
 */
void update_cpu_temp_gauge();

/**
 * @brief Actualiza la métrica de la velocidad de la CPU.
 */
void update_cpu_speed_gauge();

/**
 * @brief Actualiza la métrica de sys calls.
 */
void update_sys_calls_gauge();
/**
 * @brief Actualiza la metrica de carga promedio
 */
void update_avg_load_gauge();

/**
 * @brief Actualiza la métrica de número de procesos en ejecución.
 */
void update_processes_gauges();

/**
 * @brief Actualiza la métrica de sys calls.
 */
void update_disk_io_gauge();

/**
 * @brief Actualiza las metricas de red.
 */
void update_network_gauge();

/**
 * @brief Función del hilo para exponer las métricas vía HTTP en el puerto 8000.
 * @param arg Argumento no utilizado.
 * @return NULL
 */
void* expose_metrics(void* arg);

/**
 * @brief Inicializar mutex y métricas.
 */
int init_metrics();

/**
 * @brief Destructor de mutex
 */
void destroy_mutex();
