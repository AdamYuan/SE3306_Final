################################################################################
##
##   Copyright (C) 2016-2023 Keith O'Hara
##
##   This file is part of the GCE-Math C++ library.
##
##   Licensed under the Apache License, Version 2.0 (the "License");
##   you may not use this file except in compliance with the License.
##   You may obtain a copy of the License at
##
##       http://www.apache.org/licenses/LICENSE-2.0
##
##   Unless required by applicable law or agreed to in writing, software
##   distributed under the License is distributed on an "AS IS" BASIS,
##   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
##   See the License for the specific language governing permissions and
##   limitations under the License.
##
################################################################################

cmake_minimum_required(VERSION 3.1)
project(gcem)

set(GCEM_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

#

file(STRINGS "${GCEM_INCLUDE_DIR}/gcem_incl/gcem_options.hpp" gcem_version_defines
     REGEX "#define GCEM_VERSION_(MAJOR|MINOR|PATCH)")

foreach(ver ${gcem_version_defines})
    if(ver MATCHES "#define GCEM_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$")
        set(GCEM_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif()
endforeach()

set(${PROJECT_NAME}_VERSION
    ${GCEM_VERSION_MAJOR}.${GCEM_VERSION_MINOR}.${GCEM_VERSION_PATCH})
message(STATUS "GCE-Math version ${${PROJECT_NAME}_VERSION}")

#

if(NOT MSVC)
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
    
    if(COMPILER_SUPPORTS_CXX11)
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has C++11 support.")
    else()
        message(FATAL_ERROR "Unsupported compiler ${CMAKE_CXX_COMPILER} "
            "GCEM requires a C++11-compatible compiler.")
    endif()
endif()

add_library(gcem INTERFACE)
target_include_directories(gcem INTERFACE ${GCEM_INCLUDE_DIR})
