cmake_minimum_required(VERSION 3.15)
project(SE3306_Final)

set(TEXTURE_SRC
        tumbler.png
        floor.png
)

set(TEXTURE_INCLUDE_PATH ${CMAKE_CURRENT_LIST_DIR}/include/texture)

add_executable(texture_generator generator/generator.cpp)

function(add_texture INPUT_FILE)
    add_custom_command(
            OUTPUT ${TEXTURE_INCLUDE_PATH}/${INPUT_FILE}.u8
            COMMAND $<TARGET_FILE_DIR:texture_generator>/texture_generator src/${INPUT_FILE} ${TEXTURE_INCLUDE_PATH}/${INPUT_FILE}.u8
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS texture_generator ${CMAKE_CURRENT_SOURCE_DIR}/src/${INPUT_FILE}
    )
endfunction()

foreach (X IN LISTS TEXTURE_SRC)
    add_texture(${X})
endforeach ()

set(TEXTURE_U8)
foreach (X IN LISTS TEXTURE_SRC)
    list(APPEND TEXTURE_U8 ${TEXTURE_INCLUDE_PATH}/${X}.u8)
endforeach ()

add_custom_target(
        texture_u8 ALL DEPENDS
        ${TEXTURE_U8}
)
add_library(texture INTERFACE)
add_dependencies(texture texture_u8)
target_include_directories(texture INTERFACE include)
