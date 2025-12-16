# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
# Início da configuração automática do VS Code
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# Fim da configuração automática

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- MUDANÇA IMPORTANTE 1: Definir placa como PICO_W ---
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Puxa o SDK (Certifique-se que o arquivo pico_sdk_import.cmake existe na pasta)
include(pico_sdk_import.cmake)

# Inicie o FreeRTOS
include($ENV{FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(main C CXX ASM)

# Inicializa o SDK
pico_sdk_init()

# Adiciona o executável com SEUS arquivos de sensores + main.c
add_executable(main 
    main.c 
    ${PICO_SDK_PATH}/lib/lwip/src/apps/mqtt/mqtt.c
)

pico_set_program_name(main "Main")
pico_set_program_version(main "0.1")

# Habilita saída USB (Serial)
pico_enable_stdio_uart(main 0)
pico_enable_stdio_usb(main 1)

# --- MUDANÇA IMPORTANTE 2: Adicionar bibliotecas de Rede/WiFi ---
target_link_libraries(main
        pico_stdlib 
        hardware_adc 
        hardware_i2c
        hardware_pwm
        FreeRTOS-Kernel 
        FreeRTOS-Kernel-Heap4
        # ESTA É A BIBLIOTECA CORRETA PARA FREERTOS + WIFI
        pico_cyw43_arch_lwip_sys_freertos
)

# Includes
target_include_directories(main PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/inc
)

pico_add_extra_outputs(main)