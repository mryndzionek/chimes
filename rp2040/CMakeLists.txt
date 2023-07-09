cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(pico_chimes)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

include_directories(../include)

add_executable(chimes
        main.c
        ../src/chimes.c
        )

target_link_libraries(chimes
        pico_multicore
        pico_stdlib
        pico_rand
        hardware_dma
        hardware_pio
        hardware_timer
        hardware_clocks
        hardware_pwm)

pico_add_extra_outputs(chimes)




