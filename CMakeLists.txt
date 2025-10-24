cmake_minimum_required(VERSION 3.10)
project(XLCAssistant VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_WARN_UNUSED_CLI YES)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT XCODE AND NOT MSVC AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if(WIN32)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

if(MSVC)
    add_compile_options("$<$<COMPILE_LANGUAGE:C>:/utf-8>")
    add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:/utf-8>")
    add_compile_options("$<$<COMPILE_LANGUAGE:C>:/bigobj>")
    add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:/bigobj>")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找包
set(MCP_DIR "E:/github/1MyFork/cpp-mcp/install/cmake")
find_package(spdlog CONFIG REQUIRED)
find_package(Qt5 COMPONENTS Core Widgets Gui Svg Concurrent Sql Network REQUIRED)
find_package(MCP REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB_RECURSE HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC)

add_executable(${PROJECT_NAME}
    ${SRC}
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/res/resources.qrc
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    spdlog::spdlog_header_only
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::Svg
    Qt5::Concurrent
    Qt5::Sql
    Qt5::Network
    MCP::mcp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 设置日志级别
target_compile_definitions(${PROJECT_NAME} PRIVATE
    SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE
)

# 设置输出路径
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output
)

option(ENABLE_DEBUG_BORDER "启用调试边框" OFF)

if(ENABLE_DEBUG_BORDER)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_DEBUG_BORDER=1)
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_DEBUG_BORDER=0)
endif()
