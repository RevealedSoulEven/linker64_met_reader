cmake_minimum_required(VERSION 3.18.1)

project("linker_extractor")

add_library(
    linker_extractor
    SHARED
    linker_extractor.cpp
)

find_library(
    log-lib
    log
)

target_link_libraries(
    linker_extractor
    ${log-lib}
)