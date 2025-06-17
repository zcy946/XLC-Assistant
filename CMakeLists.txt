cmake_minimum_required(VERSION 3.10)
project(xlc_mcp_client VERSION 0.0.1 LANGUAGES CXX)

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

# 设置mcp库的路径
set(MCP_ROOT "E:/github/cpp-mcp-fork/cpp-mcp/src/install")
set(MCP_INCLUDES "${MCP_ROOT}/include")
set(MCP_LIB_DEBUG "${MCP_ROOT}/mcpd.lib")
set(MCP_LIB_RELEASE "${MCP_ROOT}/mcp.lib")
set(MCP_LIBS
    debug ${MCP_LIB_DEBUG}
    optimized ${MCP_LIB_RELEASE}
)

# mcp库必备宏定义
add_compile_definitions(MCP_SSL CPPHTTPLIB_OPENSSL_SUPPORT)

# 查找包
find_package(OpenSSL COMPONENTS Crypto SSL REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Qt5 COMPONENTS Core Widgets Gui Svg Concurrent REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB_RECURSE HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC)

add_executable(${PROJECT_NAME}

    # ${SRC}
    # ${HEADERS}
    # ${CMAKE_CURRENT_SOURCE_DIR}/res/resources.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/example/agent_example.cpp

    # ${CMAKE_CURRENT_SOURCE_DIR}/example/agent_without_mcp_server.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${MCP_LIBS}
    ${OPENSSL_LIBRARIES}
    spdlog::spdlog_header_only
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::Svg
    Qt5::Concurrent
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${MCP_INCLUDES}
    ${OPENSSL_INCLUDE_DIR}
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