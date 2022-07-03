project(main)

# libs
list(APPEND LIBS lib::proxy_server)
list(APPEND LIBS lib::common)
list(APPEND LIBS lib::test)

# includes
list(APPEND INCLUDES ${ProxyServer_SOURCE_DIR})
list(APPEND INCLUDES ${Common_SOURCE_DIR})
list(APPEND INCLUDES ${PROJECT_SOURCE_DIR})
list(APPEND INCLUDES "${SimpleWebProxy_SOURCE_DIR}/TryCatch")
list(APPEND INCLUDES ${StrUtils_SOURCE_DIR})

list(APPEND INCLUDES ${ProxyServerTest_SOURCE_DIR})

# log
message("[LOG] main_includePath = ${INCLUDES}")
message("[LOG] main_includeLibs = ${LIBS}")

add_executable(${PROJECT_NAME} main.c)

target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        ${INCLUDES}
)