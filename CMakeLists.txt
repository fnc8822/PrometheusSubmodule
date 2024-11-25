cmake_minimum_required(VERSION 3.10)

# Project name
project(MetricasProject)

# Add Conan setup
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

# Add subdirectories
add_subdirectory(lib/prom-c-client/prom)
add_subdirectory(lib/prom-c-client/promhttp)

# Include directories
include_directories(include lib/prom-c-client/prom/include lib/prom-c-client/promhttp/include)

# Add library
add_library(metricas STATIC src/metrics.c src/expose_metrics.c) # Add all necessary source files

# Add executable
add_executable(metrics_executable src/main.c)

# Adjust compilation flags for the prom library
set_target_properties(prom PROPERTIES COMPILE_FLAGS "-Wno-error=pedantic -Wno-error=missing-prototypes -Wno-error=strict-prototypes -Wno-error=unused-parameter")

# Link the necessary libraries
target_link_libraries(metricas prom promhttp CONAN_PKG::cjson CONAN_PKG::libmicrohttpd)
target_link_libraries(metrics_executable metricas prom promhttp CONAN_PKG::cjson CONAN_PKG::libmicrohttpd)
