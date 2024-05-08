cmake_minimum_required(VERSION 3.13)

project(wakaama C)


option(ENABLE_EXAMPLES "Build the examples" ON)

if (ENABLE_EXAMPLES)
    add_subdirectory(examples)
endif()

# Enable "test" target
option(ENABLE_TESTS "Build the unit tests" ON)

if (ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/wakaama.cmake)
