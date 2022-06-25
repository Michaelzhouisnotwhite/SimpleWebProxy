project(Common)
add_library(${PROJECT_NAME} SocketUtils.c)
add_library(lib::common ALIAS ${PROJECT_NAME})

if (MINGW)
    target_link_libraries(${PROJECT_NAME} ws2_32)
endif()