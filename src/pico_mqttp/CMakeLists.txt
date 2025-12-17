cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Placa correta
set(PICO_BOARD pico_w CACHE STRING "Board type")

include(pico_sdk_import.cmake)

# FreeRTOS
include($ENV{FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(main C CXX ASM)

pico_sdk_init()

add_executable(main
    main.c
    VL53L0X.c
    TCS34725.c
    LED_RGB.c
    ${PICO_SDK_PATH}/lib/lwip/src/apps/mqtt/mqtt.c
)

pico_set_program_name(main "Main")
pico_set_program_version(main "0.1")

pico_enable_stdio_uart(main 0)
pico_enable_stdio_usb(main 1)

target_link_libraries(main
    pico_stdlib
    hardware_adc
    hardware_i2c
    hardware_pwm
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
    pico_cyw43_arch_lwip_sys_freertos
)

target_include_directories(main PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(main)
