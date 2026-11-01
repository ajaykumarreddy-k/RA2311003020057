cmake_minimum_required(VERSION 3.16)
project(UnifiedEngineeringWorkbench VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable CTest
enable_testing()

# Compiler options & Static Runtime configuration
if(MSVC)
    # Statically link C++ runtime (/MT in Release, /MTd in Debug)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    add_compile_options(/W4 /EHsc /utf-8)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(WIN32)
        # MinGW static runtime
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
    endif()
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# Core library source files
set(CORE_SOURCES
    src/core/fea.cpp
    src/core/backprop.cpp
)

add_library(workbench_core STATIC ${CORE_SOURCES})
target_include_directories(workbench_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# Core unit tests
add_executable(test_fea tests/test_fea.cpp)
target_link_libraries(test_fea PRIVATE workbench_core)
add_test(NAME test_fea COMMAND test_fea)

add_executable(test_backprop tests/test_backprop.cpp)
target_link_libraries(test_backprop PRIVATE workbench_core)
add_test(NAME test_backprop COMMAND test_backprop)
