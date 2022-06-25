project(ProxyServer)

add_library(${PROJECT_NAME} proxy_server.c)
add_library(lib::proxy_server ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE lib::common)

list(APPEND INCLUDES  ${Common_SOURCE_DIR})

message(STATUS "lib::proxy_server_includePath = ${INCLUDES}")
# message(STATUS "Temp = ${Common_SOURCE_DIR}")
message(STATUS "lib::proxy_server_includeLibs = ${LIBS}")

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
       ${INCLUDES}
)
