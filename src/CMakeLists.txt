cmake_minimum_required(VERSION 3.20)
add_library("${the_library}" STATIC
    "pngpp.cpp"
)
set_property(TARGET "${the_library}" PROPERTY CXX_STANDARD 20)
target_include_directories("${the_library}" INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries("${the_library}" INTERFACE
    "png"
)
