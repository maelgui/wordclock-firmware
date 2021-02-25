add_library(nanopb pb_common.c pb_decode.c pb_encode.c)
target_include_directories(nanopb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
