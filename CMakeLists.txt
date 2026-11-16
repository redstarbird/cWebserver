cmake_minimum_required(VERSION 3.10)
project(webserver VERSION 0.0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(CURL REQUIRED)

add_library(webserver STATIC)

target_sources(webserver
    PRIVATE
    src/webserver.c
    src/content_types.c
    src/util/file_util.c
)

if(WIN32)
    target_sources(webserver PRIVATE src/platform/platform_windows.c)
    target_link_libraries(webserver PUBLIC ws2_32 crypt32)
    target_compile_definitions(webserver PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE)
else()
    target_sources(webserver PRIVATE src/platform/platform_posix.c)
endif()

target_include_directories(webserver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(webserver PUBLIC OpenSSL::SSL OpenSSL::Crypto Threads::Threads)

# -----------------------------
# Testing setup
# -----------------------------
if(PROJECT_IS_TOP_LEVEL)
    include(CTest)
    enable_testing()

    find_program(OPENSSL_EXECUTABLE openssl)
    if (OPENSSL_EXECUTABLE)
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/cert.pem ${CMAKE_CURRENT_BINARY_DIR}/key.pem
            COMMAND ${OPENSSL_EXECUTABLE} req -x509 -newkey rsa:2048 -keyout key.pem -out cert.pem -sha256 -days 365 -nodes -subj "/CN=localhost"
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating self-signed SSL certificates for testing..."
        )

        add_custom_target(generate_test_certs ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/cert.pem)
    else()
        message(WARNING "OpenSSL executable not found. Tests will fail if cert.pem or key.pem are missing!")
    endif()

    # Build testing version of the library including mock definitions

    get_target_property(WEBSERVER_SOURCES webserver SOURCES)
    add_library(webserver_testable STATIC ${WEBSERVER_SOURCES} tests/mocks.c)
    # Add testing compile definition to inject mocked functions
    target_compile_definitions(webserver_testable PUBLIC CWS_TESTING)

    target_include_directories(webserver_testable PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(webserver_testable PUBLIC OpenSSL::SSL OpenSSL::Crypto Threads::Threads)

    if(WIN32)
        target_link_libraries(webserver_testable PUBLIC ws2_32 crypt32)
        target_compile_definitions(webserver_testable PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE)
    endif()

    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(webserver_testable PUBLIC
            -g 
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
            -fno-sanitize-recover=undefined
            )
            target_link_options(webserver_testable PUBLIC -fsanitize=address,undefined)
        endif()

    # Create executable for every test source file

    file(GLOB TEST_SOURCES "tests/test_*.c")

    foreach(TEST_FILE ${TEST_SOURCES})

        # Get test name from file name (without extension)
        get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)

        add_executable(${TEST_NAME} ${TEST_FILE})

        # Link the webserver library to the test executable
        target_link_libraries(${TEST_NAME} PRIVATE webserver_testable CURL::libcurl)

        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

    endforeach()

endif()