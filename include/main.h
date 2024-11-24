/**
 * @file main.h
 * @brief Header file for main.c
 * includes global variables and function prototypes
 */

extern bool cpu_enabled = false;
extern bool memory_enabled = true;
extern bool battery_enabled = false;
extern bool avg_load_enabled = false;
extern bool cpu_temp_enabled = false;
extern bool cpu_speed_enabled = false;
extern bool processes_enabled = false;
extern bool sys_calls_enabled = false;
extern bool disk_io_enabled = false;
extern bool network_enabled = false;


/**
 * @brief read the configuration file and set the global variables
 * file: ../config.json
 */
void read_config(const char *config_file_path);