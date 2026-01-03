#
# Copyright (c) 2025 Vinnie Falco (vinnie dot falco at gmail dot com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/cppalliance/wg21-papers/affine-awaitables
#

cmake_minimum_required(VERSION 3.16)
project(affine-awaitables LANGUAGES CXX)

# Ensure MSVC builds go in build-msvc directory
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    get_filename_component(BUILD_DIR_NAME ${CMAKE_BINARY_DIR} NAME)
    if(NOT BUILD_DIR_NAME STREQUAL "build-msvc")
        message(FATAL_ERROR "MSVC builds must be configured in the 'build-msvc' directory.\n"
            "Current build directory: ${CMAKE_BINARY_DIR}\n"
            "Please run: cmake -B build-msvc -S .")
    endif()
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch beman-project/execution (P2300 implementation)
include(FetchContent)
FetchContent_Declare(
    beman_execution
    GIT_REPOSITORY https://github.com/beman-project/execution26.git
    GIT_TAG main
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(beman_execution)

# Common headers
set(COMMON_HEADERS
    affine.hpp
    small_function.hpp
    thread_pool.hpp
)

# Executable: task (demo_affine_task.cpp)
add_executable(task task.cpp ${COMMON_HEADERS})
target_include_directories(task PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Executable: custom_task (demo_my_task.cpp)
add_executable(custom_task custom_task.cpp ${COMMON_HEADERS})
target_include_directories(custom_task PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Executable: senders_task (demo_affine_task_senders.cpp)
# Note: Requires beman/execution library (P2300 implementation)
add_executable(senders_task senders_task.cpp ${COMMON_HEADERS})
target_include_directories(senders_task PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${beman_execution_SOURCE_DIR}/include
)

# Platform-specific settings
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(task PRIVATE /W4 /permissive-)
    target_compile_options(custom_task PRIVATE /W4 /permissive-)
    target_compile_options(senders_task PRIVATE /W4 /permissive-)
    
    # Optimization flags for Release and RelWithDebInfo builds
    target_compile_options(task PRIVATE 
        $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:/O2 /Ob2 /GL>
    )
    target_compile_options(custom_task PRIVATE 
        $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:/O2 /Ob2 /GL>
    )
    target_compile_options(senders_task PRIVATE 
        $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:/O2 /Ob2 /GL>
    )
    
    target_link_options(task PRIVATE 
        $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:/LTCG>
    )
    target_link_options(custom_task PRIVATE 
        $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:/LTCG>
    )
    target_link_options(senders_task PRIVATE 
        $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:/LTCG>
    )
endif()

# Threading support
find_package(Threads REQUIRED)
target_link_libraries(task PRIVATE Threads::Threads)
target_link_libraries(custom_task PRIVATE Threads::Threads)
target_link_libraries(senders_task PRIVATE Threads::Threads)

# Source groups for Visual Studio
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${COMMON_HEADERS} task.cpp custom_task.cpp senders_task.cpp)
