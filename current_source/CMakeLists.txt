cmake_minimum_required(VERSION 3.12)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(picocode)

pico_sdk_init()

add_executable(picocode
        current_source.c
)

target_link_libraries(picocode
        pico_stdlib
	hardware_i2c
        hardware_adc
        hardware_pwm
)

pico_enable_stdio_usb(picocode 0)
pico_enable_stdio_uart(picocode 1)

pico_add_extra_outputs(picocode)
