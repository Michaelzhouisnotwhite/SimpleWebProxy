project(StrUtils)

add_library(${PROJECT_NAME} lib4str.c lib4str.h)
add_library(lib::lib4str ALIAS ${PROJECT_NAME})

#target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})
