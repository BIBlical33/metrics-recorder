cmake_minimum_required(VERSION 3.20)
project(metrics_recorder VERSION 0.0.1)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    foreach(flag_var
            CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        string(REPLACE "/GR" "" ${flag_var} "${${flag_var}}")
        string(REPLACE "/W3" "" ${flag_var} "${${flag_var}}")
    endforeach()
    add_compile_options(/GR- /W4)
    add_compile_options(/Zi /DEBUG)
else()
    add_compile_options(-fno-rtti -Wall -Wextra -Wpedantic)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)

FetchContent_Declare(
  libcds
  URL https://github.com/khizmax/libcds/archive/refs/heads/master.zip
)
FetchContent_MakeAvailable(libcds)

# Main header-only library
add_library(metrics_recorder INTERFACE)
target_include_directories(metrics_recorder
    INTERFACE  
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
        ${libcds_SOURCE_DIR}
)

target_link_libraries(metrics_recorder INTERFACE cds)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

if(NOT googletest_POPULATED)
    message(WARNING "Googletest not available, tests will not be built")
    return()
endif()

enable_testing()
add_executable(metrics_recorder_test
    test/metrics_recorder_test.cc
)

target_link_libraries(metrics_recorder_test
    PRIVATE
        metrics_recorder
        GTest::gtest_main
        cds
)

target_include_directories(metrics_recorder_test
    PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${libcds_SOURCE_DIR}
)

include(GoogleTest)
gtest_discover_tests(metrics_recorder_test)