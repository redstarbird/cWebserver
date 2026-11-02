cmake_minimum_required(VERSION 3.10)
project(webserver VERSION 0.0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

add_library(webserver STATIC)

target_sources(webserver
    PRIVATE
    src/webserver.c
    src/content_types.c
    src/util/file_util.c
)