cmake_minimum_required(VERSION 3.15)
project(SE3306_Final)

set(SHADER_HEADER
        Binding.h
        Config.h
        Texture.h
)

set(SHADER_SRC
        shadow.frag
        shadow.vert
        gbuffer.frag
        gbuffer.vert
        voxelize.frag
        voxelize.geom
        voxelize.vert
        voxel_mipmap_0.comp
        voxel_mipmap.comp
        quad.vert
        final.frag
        bloom_down_0.frag
        bloom_down.frag
        bloom_up.frag
)

set(SHADER_INCLUDE_PATH ${CMAKE_CURRENT_LIST_DIR}/include/shader)

list(TRANSFORM SHADER_HEADER PREPEND ${SHADER_INCLUDE_PATH}/)

add_executable(shader_generator generator/generator.cpp)

function(add_shader INPUT_FILE)
    add_custom_command(
            OUTPUT ${SHADER_INCLUDE_PATH}/${INPUT_FILE}.str
            COMMAND $<TARGET_FILE_DIR:shader_generator>/shader_generator src/${INPUT_FILE} ${SHADER_INCLUDE_PATH}/${INPUT_FILE}.str ${SHADER_INCLUDE_PATH}/
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS shader_generator ${CMAKE_CURRENT_SOURCE_DIR}/src/${INPUT_FILE} ${SHADER_HEADER}
    )
endfunction()

foreach (X IN LISTS SHADER_SRC)
    add_shader(${X})
endforeach ()

set(SHADER_STR)
foreach (X IN LISTS SHADER_SRC)
    list(APPEND SHADER_STR ${SHADER_INCLUDE_PATH}/${X}.str)
endforeach ()

add_custom_target(
        shader_str ALL DEPENDS
        ${SHADER_STR}
)
add_library(shader INTERFACE)
add_dependencies(shader shader_str)
target_include_directories(shader INTERFACE include)
