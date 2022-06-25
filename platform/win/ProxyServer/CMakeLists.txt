project(ProxyServer)

add_library(${PROJECT_NAME} proxy_server.c)
add_library(lib::proxy_server ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE lib::common)

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../Common
)
