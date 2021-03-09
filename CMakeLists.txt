cmake_minimum_required(VERSION 3.10)

# Board config
set(MCU   atmega328p)
set(F_CPU 16000000UL)

# Flash config
set(PROG_TYPE arduino)
set(USBPORT /dev/ttyUSB0)
set(BAUDRATE 115200)

# Compiler options
add_compile_options(-Wall -Wextra -Os)

set(CMAKE_C_FLAGS   -mmcu=${MCU})
set(CMAKE_CXX_FLAGS -mmcu=${MCU})

set(CMAKE_CXX_COMPILER avr-g++)
set(CMAKE_C_COMPILER   avr-gcc)
set(CMAKE_ASM_COMPILER avr-gcc)

add_compile_definitions(F_CPU=${F_CPU} MCU=${MCU})


# set the project name
project(firmware VERSION 1.1)

add_subdirectory(lib)
SET(SOURCE_DIR src)
add_subdirectory(${SOURCE_DIR})

add_custom_target(strip ALL     avr-strip "${SOURCE_DIR}/${PROJECT_NAME}.elf" DEPENDS ${PROJECT_NAME})
add_custom_target(hex   ALL     avr-objcopy -R .eeprom -O ihex "${SOURCE_DIR}/${PROJECT_NAME}.elf" "${SOURCE_DIR}/${PROJECT_NAME}.hex" DEPENDS strip)
add_custom_target(eeprom        avr-objcopy -j .eeprom --change-section-lma .eeprom=0 -O ihex "${SOURCE_DIR}/${PROJECT_NAME}.elf" "${SOURCE_DIR}/${PROJECT_NAME}.eeprom" DEPENDS strip)
add_custom_target(flash         avrdude -D -c ${PROG_TYPE} -p ${MCU} -b ${BAUDRATE} -P ${USBPORT} -U flash:w:${SOURCE_DIR}/${PROJECT_NAME}.hex DEPENDS hex)
