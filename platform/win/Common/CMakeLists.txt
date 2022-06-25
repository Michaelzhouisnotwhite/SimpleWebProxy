project(Common)
add_library(${PROJECT_NAME} SocketUtils.c)
add_library(lib::common ALIAS ${PROJECT_NAME})