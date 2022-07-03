project(ProxyServerTest)

add_library(${PROJECT_NAME} test_fuc.c)

add_library(lib::test ALIAS ${PROJECT_NAME})

# include

list(APPEND INCLUDES ${StrUtils_SOURCE_DIR})
list(APPEND INCLUDES ${Common_SOURCE_DIR})
list(APPEND INCLUDES ${ProxyServer_SOURCE_DIR})

list(APPEND LIBS lib::proxy_server)
list(APPEND LIBS lib::common)
list(APPEND LIBS lib::lib4str)

target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBS})