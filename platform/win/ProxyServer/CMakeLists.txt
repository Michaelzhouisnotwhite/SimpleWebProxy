project(ProxyServer)

add_library(${PROJECT_NAME} proxy_server.c)
add_library(lib::proxy_server ALIAS ${PROJECT_NAME})

# LIBS
list(APPEND LIBS lib::common)
list(APPEND LIBS pthread)
list(APPEND LIBS lib::lib4str)

# INCLUDES
list(APPEND INCLUDES  ${Common_SOURCE_DIR})
list(APPEND INCLUDES "${SimpleWebProxy_SOURCE_DIR}/TryCatch")
list(APPEND INCLUDES ${StrUtils_SOURCE_DIR})

message(STATUS "[LOG] lib::proxy_server_includePath = ${INCLUDES}")
message(STATUS "[LOG] lib::proxy_server_includeLibs = ${LIBS}")

target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})
target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
       ${INCLUDES}
)
