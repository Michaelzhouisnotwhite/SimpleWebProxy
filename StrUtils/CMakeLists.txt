project(StrUtils)

add_library(${PROJECT_NAME} lib4str.c)
add_library(lib::lib4str ALIAS ${PROJECT_NAME})
