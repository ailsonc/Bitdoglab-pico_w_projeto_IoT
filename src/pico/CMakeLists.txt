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
# Arquivo de projeto Cmake Pico gerado

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Inicialize o pico_sdk a partir do local de instalação
# (observe que isso pode vir do ambiente, do cache do CMake etc)

# == NÃO EDITE AS SEGUINTES LINHAS para que a extensão Raspberry Pi Pico do VS Code funcione ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 1.5.1)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Faça o pull do SDK Raspberry Pi Pico (deve ser feito antes do projeto)
include(pico_sdk_import.cmake)

# Inicie o FreeRTOS
include($ENV{FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)


project(main C CXX ASM)

# Inicialize o SDK do Raspberry Pi Pico
pico_sdk_init()

# Adicionar executável. O nome padrão é o nome do projeto, versão 0.1
add_executable(main 
    main.c 
    VL53L0X.c
    TCS34725.c
    LED_RGB.c
    inc/ssd1306_i2c.c
)

pico_set_program_name(main "Main")
pico_set_program_version(main "0.1")

# Modifique as linhas abaixo para habilitar/desabilitar a saída via UART/USB
pico_enable_stdio_uart(main 0)
pico_enable_stdio_usb(main 1)

# Adicione a biblioteca padrão à compilação
target_link_libraries(main
        pico_stdlib 
        hardware_adc 
        hardware_i2c
        hardware_pwm
        FreeRTOS-Kernel 
        FreeRTOS-Kernel-Heap4
)

# Adicione os arquivos de inclusão padrão à compilação
target_include_directories(main PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(main)
