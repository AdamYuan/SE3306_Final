cmake_minimum_required(VERSION 3.26)
project(SE3306_Final)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)

if (CMAKE_SYSTEM_NAME STREQUAL Windows)
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-DNOMINMAX)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
endif ()

if (MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT IPO_SUPPORTED)
    if (IPO_SUPPORTED)
        message("Enable IPO")
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif ()
endif ()

add_subdirectory(dep)
add_subdirectory(shader)
add_subdirectory(texture)

add_executable(
        Tumbler
        src/main.cpp
        src/Mesh.cpp
        src/MeshLoader.cpp
        src/Animation.cpp
        src/Playground.cpp
        src/ParticleSystem.cpp

        src/GPUMesh.cpp
        src/GPUMeshInstance.cpp
        src/GPUAnimation.cpp

        src/rg/ARenderGraph.cpp
        src/rg/GBufferPass.cpp
        src/rg/ShadowMapPass.cpp
)
target_link_libraries(Tumbler PRIVATE shader texture dep)

install(TARGETS Tumbler RUNTIME DESTINATION)
