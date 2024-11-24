/**
 * @brief Entry point of the system
 * version usada para exposicion de metricas en lab2
 */
#define PATH_MAX 256
#define VARIABLE_PATH_MAX 40
#include "expose_metrics.h"
#include "main.h" 
#include <stdbool.h>
#include <cjson/cJSON.h>
int sleep_time = 1;
bool cpu_enabled = false;
bool memory_enabled = true;
bool battery_enabled = false;
bool avg_load_enabled = false;
bool cpu_temp_enabled = false;
bool cpu_speed_enabled = false;
bool processes_enabled = false;
bool sys_calls_enabled = false;
bool disk_io_enabled = false;
bool network_enabled = false;

void read_config(const char *config_file_path){
        char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        exit(1);
    }

    // Increase the buffer size for config_file_path
    FILE *file = fopen(config_file_path, "r");
    if (file == NULL) {
        perror("fopen");
        exit(1);
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the file content
    char *data = malloc(length + 1);
    if (data == NULL) {
        perror("malloc");
        fclose(file);
        exit(1);
    }

    // Read the file content into the buffer
    size_t read_size = fread(data, 1, length, file);
    if (read_size != length) {
        perror("fread");
        free(data);
        fclose(file);
        exit(1);
    }
    data[length] = '\0'; // Null-terminate the string

    fclose(file);

    // Parse the JSON content
    cJSON *json = cJSON_Parse(data);
    if (!json) {
        fprintf(stderr, "Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        free(data);
        return;
    }

    cJSON *interval = cJSON_GetObjectItem(json, "sampling_interval");
    if(cJSON_IsNumber(interval)){
        sleep_time = interval->valueint;
    }

    cJSON *metrics = cJSON_GetObjectItem(json, "metrics");
    if (cJSON_IsArray(metrics)){
        cJSON *metric;
        cJSON_ArrayForEach(metric, metrics){
            if(cJSON_IsString(metric)){
                if(strcmp(metric->valuestring, "cpu") == 0){
                    cpu_enabled=true;
                } else if(strcmp(metric->valuestring, "memory") == 0){
                    memory_enabled=true;
                } else if(strcmp(metric->valuestring, "battery") == 0){
                    battery_enabled=true;
                } else if(strcmp(metric->valuestring, "avg_load") == 0){
                    avg_load_enabled=true;
                } else if(strcmp(metric->valuestring, "cpu_temp") == 0){
                    cpu_temp_enabled=true;
                } else if(strcmp(metric->valuestring, "cpu_speed") == 0){
                    cpu_speed_enabled=true;
                } else if(strcmp(metric->valuestring, "processes") == 0){
                    processes_enabled=true;
                } else if(strcmp(metric->valuestring, "sys_calls") == 0){
                    sys_calls_enabled=true;
                } else if(strcmp(metric->valuestring, "disk") == 0){
                    disk_io_enabled=true;
                } else if(strcmp(metric->valuestring, "network") == 0){
                    network_enabled=true;
                }
            }
        }
    }
    cJSON_Delete(json);
    free(data);
}

int main(int argc, char* argv[])
{
    (void)argc; // Argumento no utilizado
    (void)argv; // Argumento no utilizado
    const char *config_file_path = getenv("CONFIG_FILE_PATH");
    if (config_file_path == NULL) {
        fprintf(stderr, "Environment variable CONFIG_FILE_PATH is not set.\n");
        return EXIT_FAILURE;
    }

    read_config(config_file_path);
    // Create a thread to expose metrics via HTTP
    if (init_metrics() != EXIT_SUCCESS)
    {
        fprintf(stderr, "Error al inicializar las métricas\n");
        return EXIT_FAILURE;
    }
    pthread_t tid;
    if (pthread_create(&tid, NULL, expose_metrics, NULL) != 0)
    {
        fprintf(stderr, "Error creating the HTTP server thread\n");
        return EXIT_FAILURE;
    }

    // Main thread can perform other tasks here
    while (true)
    {
        // Simulate work
        sleep(sleep_time);
        if(cpu_enabled){
            update_cpu_gauge();
        }
        if(memory_enabled){
            update_memory_metrics();
        }
        if(battery_enabled){
            update_battery_gauge();
        }
        if(avg_load_enabled){
            update_avg_load_gauge();
        }
        if(cpu_temp_enabled){
            update_cpu_temp_gauge();
        }
        if(cpu_speed_enabled){
            update_cpu_speed_gauge();
        }
        if(processes_enabled){
            update_processes_gauges();
        }
        if(sys_calls_enabled){
            update_sys_calls_gauge();
        }
        if(disk_io_enabled){
            update_disk_io_gauge();
        }
        if(network_enabled){
            update_network_gauge();
        }
    }

    return EXIT_SUCCESS;
}
