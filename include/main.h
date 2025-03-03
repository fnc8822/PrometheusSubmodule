/**
 * @file main.h
 * @brief Header file for main.c
 * includes global variables and function prototypes
 */

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


/**
 * @brief read the configuration file and set the global variables
 * file: ../config.json
 */
void read_config(const char *config_file_path);