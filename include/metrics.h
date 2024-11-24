/**
 * @file metrics.h
 * @brief Funciones para obtener el uso de CPU y memoria desde el sistema de archivos /proc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief Tamaño del buffer para leer archivos.
 */
#define BUFFER_SIZE 256

/**
 * @brief Obtiene el porcentaje de uso de memoria desde /proc/meminfo.
 *
 * Lee los valores de memoria total y disponible desde /proc/meminfo y calcula
 * el porcentaje de uso de memoria.
 *
 * @return Poninter a arreglo de 3 elementos con la memoria total, libre y disponible, o NULL en caso de error.
 */
unsigned long* get_memory_usage();

/**
 * @brief Obtiene el uso de disco desde /proc/diskstats.
 *
 * Lee los valores de lecturas y escrituras desde /proc/diskstats y los devuelve.
 *
 * @return Pointer a arreglo de 3 elementos con las lecturas, escrituras y operaciones en progreso, o NULL en caso de
 * error.
 */
unsigned long* get_disk_io();

/**
 * @brief Obtiene las estadísticas de red desde /proc/net/dev.
 *
 * Lee los valores de bytes recibidos y enviados desde /proc/net/dev y los devuelve.
 *
 * @return Pointer a arreglo de 4 elementos con los errores y drops de transmisión y recepción.
 */
unsigned long* get_network_stats();

/**
 * @brief Obtiene el porcentaje de uso de CPU desde /proc/stat.
 *
 * Lee los tiempos de CPU desde /proc/stat y calcula el porcentaje de uso de CPU
 * en un intervalo de tiempo.
 *
 * @return Uso de CPU como porcentaje (0.0 a 100.0), o -1.0 en caso de error.
 */
double get_cpu_usage();

/**
 * @brief Obtiene el nivel de batería del sistema.
 *
 * Lee el nivel de batería del sistema y lo devuelve como un porcentaje.
 *
 * @return Nivel de batería como porcentaje (0.0 a 100.0), o -1.0 en caso de error.
 */
double get_battery_level();

/**
 * @brief Obtiene la temperatura de la CPU.
 *
 * Lee la temperatura de la CPU y la devuelve en grados Celsius.
 *
 * @return Temperatura de la CPU en grados Celsius, o -1.0 en caso de error.
 */
double get_cpu_temp();

/**
 * @brief Obtiene el número de procesos en ejecución.
 *
 * Lee el número de procesos en ejecución y lo devuelve.
 *
 * @return Pointer a arreglo de 3 elementos con las lecturas, escrituras y operaciones en progreso, o NULL en caso de
 * error
 */
unsigned long* get_number_of_processes();

/**
 * @brief Obtiene la velocidad de la CPU.
 *
 * Lee la velocidad de la CPU y la devuelve en MHz.
 *
 * @return Velocidad de la CPU en MHz, o -1.0 en caso de error.
 */
double get_cpu_speed();

/**
 * @brief Obtiene la carga promedio del sistema.
 *
 * Lee la carga promedio del sistema y la devuelve.
 *
 * @return Carga promedio del sistema en el ultimo minuto, el valor esta entre 0 y 1 por core, o -1.0 en caso de error.
 */
double get_avg_load();

/** @brief Obtiene un contador de las sys calls del proceso
 *
 * Lee el número de sys calls realizadas por el proceso y lo devuelve.
 *
 *  @return Número de sys calls realizadas por el proceso, o -1 en caso de error.
 */
double get_sys_calls();
