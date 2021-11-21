cmake_minimum_required(VERSION 3.13)

project(fuz_coap_parse_message C)

if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    add_executable(${PROJECT_NAME}
        fuz_coap_parse_message.c
        ../../coap/er-coap-13/er-coap-13.c
        ../../examples/shared/platform.c
    )

    target_compile_options(${PROJECT_NAME} PRIVATE -O1 -fsanitize=fuzzer,address)
    target_link_options(${PROJECT_NAME} PRIVATE -O1 -fsanitize=fuzzer,address)
endif()
