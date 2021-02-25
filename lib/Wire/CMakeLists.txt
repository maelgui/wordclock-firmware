add_library(wire twi.c Wire.cpp)
target_include_directories(wire INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
