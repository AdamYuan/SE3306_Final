cmake_minimum_required(VERSION 3.15)
project(SE3306_Final)

add_subdirectory(gl3w)
add_subdirectory(glfw)
add_subdirectory(mygl3)
add_subdirectory(glm)
add_subdirectory(gcem)

add_library(dep INTERFACE)
target_link_libraries(dep INTERFACE gl3w glfw mygl3 glm gcem ${CMAKE_DL_LIBS})

