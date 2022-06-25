project(main)
list(APPEND LIBS lib::proxy_server)

list(APPEND INCLUDES ${ProxyServer_SOURCE_DIR})

list(APPEND INCLUDES ${PROJECT_SOURCE_DIR})

message(STATUS "main_includePath = ${INCLUDES}")
message(STATUS "main_includeLibs = ${LIBS}")
add_executable(${PROJECT_NAME} main.c)

target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        ${INCLUDES}
)