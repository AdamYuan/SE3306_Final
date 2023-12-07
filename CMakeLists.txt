cmake_minimum_required(VERSION 3.26)
project(SE3306_Final)

set(CMAKE_CXX_STANDARD 20)

if (CMAKE_SYSTEM_NAME STREQUAL Windows)
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-DNOMINMAX)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
endif ()

if (MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
endif ()

add_subdirectory(dep)
add_subdirectory(shader)

add_executable(
        Tumbler
        src/main.cpp
        src/Mesh.cpp
        src/Voxel.cpp
        src/CameraBuffer.cpp
        src/MeshLoader.cpp
        src/GPUMesh.cpp
        src/Animation.cpp
        src/GBuffer.cpp
        src/ShadowMap.cpp
        src/Playground.cpp
        src/ParticleSystem.cpp
)
target_link_libraries(Tumbler PRIVATE shader dep)

install(TARGETS Tumbler RUNTIME DESTINATION)
