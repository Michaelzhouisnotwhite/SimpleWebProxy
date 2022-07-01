project(ProxyServer)

add_library(${PROJECT_NAME} proxy_server.c proxy_server.h lib4proxy.c lib4proxy.h)
add_library(lib::proxy_server ALIAS ${PROJECT_NAME})

# LIBS
list(APPEND LIBS lib::common)
list(APPEND LIBS pthread)
list(APPEND LIBS lib::lib4str)

# INCLUDES
list(APPEND INCLUDES ${Common_SOURCE_DIR})
list(APPEND INCLUDES "${SimpleWebProxy_SOURCE_DIR}/TryCatch")
list(APPEND INCLUDES ${StrUtils_SOURCE_DIR})

target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        ${INCLUDES}
)
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})

message(STATUS "[LOG] lib::proxy_server_includePath = ${INCLUDES}")
message(STATUS "[LOG] lib::proxy_server_includeLibs = ${LIBS}")
