# minimum CMake version required
cmake_minimum_required(VERSION 3.15)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
# Project name, version and description
project(chadpp VERSION 1.0 DESCRIPTION "Chadpp, a discord bot.")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

if (CLANG_TIDY)
set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-clang-*,clang-analyzer-*,performance-*)
endif()

set(BUILD_O3 True)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/DPP)

find_package(PQXX REQUIRED)
find_package(LAME REQUIRED)

file(GLOB SRC_FILES "src/*.cpp")

macro(SUBDIRLIST result curdir)
  file(GLOB children RELATIVE ${curdir} ${curdir}/*)
  set(dirlist "")
    foreach(child ${children})
        if(IS_DIRECTORY ${curdir}/${child})
            list(APPEND dirlist ${child})
       endif()
    endforeach()
    set(${result} ${dirlist})
endmacro()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "-g -Og -Wall -Winvalid-pch -Wno-reorder -pthread -fPIC -rdynamic")
else ()
    set(CMAKE_CXX_FLAGS "-Ofast -Wall -Winvalid-pch -Wno-reorder -pthread -fPIC -rdynamic")
endif ()

include_directories(${CMAKE_SOURCE_DIR}/lib/DPP/include)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} dpp ${PQXX_LIBRARIES})

set(PREFIX_HEADER ${CMAKE_CURRENT_LIST_DIR}/src/pch.h)


if (CLANG_TIDY)
set(CLANG_PCH_OUTPUT ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${PROJECT_NAME}.dir/cmake_pch.hxx.pch)
get_property(CLANG_PCH_COMPILE_OPTIONS TARGET ${PROJECT_NAME} PROPERTY COMPILE_OPTIONS)
get_property(CLANG_PCH_INCLUDE_DIRECTORIES TARGET ${PROJECT_NAME} PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES)
set(CLANG_PCH_INCLUDE_STRING "")
foreach(dir ${CLANG_PCH_INCLUDE_DIRECTORIES})
    # Generate system includes to suppress warnings for third-party headers
    set(CLANG_PCH_INCLUDE_STRING ${CLANG_PCH_INCLUDE_STRING} -isystem ${dir})
endforeach()
add_custom_command(
    OUTPUT "${CLANG_PCH_OUTPUT}"
    COMMAND clang++ ${CLANG_PCH_COMPILE_OPTIONS} -std=gnu++20 ${CLANG_PCH_INCLUDE_STRING} -x c++-header -O3 -pthread -fPIC ${PREFIX_HEADER} -Xclang -emit-pch -o ${CLANG_PCH_OUTPUT}
    COMMENT "Creating Clang precompiled header for clang-tidy."
)
add_custom_target(clang-pch ALL
    DEPENDS ${CLANG_PCH_OUTPUT}
)
add_dependencies(${PROJECT_NAME} clang-pch)  # Create "fake" precompiled header for clang-tidy
endif()


target_precompile_headers(${PROJECT_NAME} PRIVATE ${PREFIX_HEADER})

SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR}/src)

foreach(dir ${SUBDIRS})
    file(GLOB DIR_FILES src/${dir}/*.cpp)
    get_filename_component(dirname ${dir} NAME)
    add_library(${dirname} SHARED ${DIR_FILES})
    set_target_properties(${dirname} PROPERTIES
        PREFIX ""
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
    )    
    target_precompile_headers(${dirname} REUSE_FROM ${PROJECT_NAME})
endforeach(dir SUBDIRS)

target_link_libraries(commands ${LAME_LIBRARIES})

set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")

configure_file("chadpp.sql" "chadpp.sql" COPYONLY)
configure_file("config.json" "config.json" COPYONLY)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
