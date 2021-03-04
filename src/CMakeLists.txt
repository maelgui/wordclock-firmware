# Configure executable firmware
add_executable(${PROJECT_NAME} main.cc core.cc display.cc wordclock.cc temperature.cc messages.cc)
target_link_libraries(${PROJECT_NAME} uart rtc wire dht22)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}.elf")

# Include version constantes in project
configure_file(version.h.in version.h)
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")
