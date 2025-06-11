set(TARGET mcp)

add_library(${TARGET} STATIC
    ${CMAKE_CURRENT_LIST_DIR}/include/mcp_client.h
    ${CMAKE_CURRENT_LIST_DIR}/src/mcp_message.cpp
    ${CMAKE_CURRENT_LIST_DIR}/include/mcp_message.h
    ${CMAKE_CURRENT_LIST_DIR}/src/mcp_resource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/include/mcp_resource.h
    ${CMAKE_CURRENT_LIST_DIR}/src/mcp_server.cpp
    ${CMAKE_CURRENT_LIST_DIR}/include/mcp_server.h
    ${CMAKE_CURRENT_LIST_DIR}/src/mcp_tool.cpp
    ${CMAKE_CURRENT_LIST_DIR}/include/mcp_tool.h
    ${CMAKE_CURRENT_LIST_DIR}/src/mcp_stdio_client.cpp
    ${CMAKE_CURRENT_LIST_DIR}/include/mcp_stdio_client.h
    ${CMAKE_CURRENT_LIST_DIR}/src/mcp_sse_client.cpp
    ${CMAKE_CURRENT_LIST_DIR}/include/mcp_sse_client.h
)

target_link_libraries(${TARGET} PUBLIC ${CMAKE_THREAD_LIBS_INIT})

# If OpenSSL is found, link the OpenSSL libraries
if(OPENSSL_FOUND)
    target_link_libraries(${TARGET} PUBLIC ${OPENSSL_LIBRARIES})
endif()
