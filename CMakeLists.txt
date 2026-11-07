cmake_minimum_required(VERSION 3.10)
project(webserver VERSION 0.0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

add_library(webserver STATIC)

target_sources(webserver
    PRIVATE
    src/webserver.c
    src/content_types.c
    src/util/file_util.c
)

target_include_directories(webserver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(webserver PUBLIC OpenSSL::SSL OpenSSL::Crypto Threads::Threads)

# -----------------------------
# Testing setup
# -----------------------------
if(PROJECT_IS_TOP_LEVEL)
    include(CTest)
    enable_testing()

    file(GLOB TEST_SOURCES "tests/test_*.c")

    foreach(TEST_FILE ${TEST_SOURCES})

        # Get test name from file name (without extension)
        get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)

        add_executable(${TEST_NAME} ${TEST_FILE})

        # Link the webserver library to the test executable
        target_link_libraries(${TEST_NAME} PRIVATE webserver)

        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

    endforeach()

endif()