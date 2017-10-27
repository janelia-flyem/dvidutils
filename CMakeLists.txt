cmake_minimum_required(VERSION 3.1)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake )

SET(PROJECT_NAME "dvidutils")
project(dvidutils)

include(CheckCXXCompilerFlag)

#-------------------------------------------------------------------------------------------------------------------
# check for c++ 14 support
#-------------------------------------------------------------------------------------------------------------------

# FIXME c++14 checks are broken???
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    CHECK_CXX_COMPILER_FLAG("-std=c++14" HAS_CPP14_FLAG)
    #CHECK_CXX_COMPILER_FLAG("-std=c++11" HAS_CPP11_FLAG)
    if (HAS_CPP14_FLAG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    else()
        message(FATAL_ERROR "Unsupported compiler -- multi-array  requires C++14 support!")
    endif()
endif()


if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()


#-------------------------------------------------------------------------------------------------------------------
# include include files
#-------------------------------------------------------------------------------------------------------------------
#file(GLOB_RECURSE headers include/*.hxx)
#include_directories(include)


#-------------------------------------------------------------------------------------------------------------------
# Setup python bindings
#-------------------------------------------------------------------------------------------------------------------
set(CMAKE_PREFIX_PATH $ENV{CONDA_PREFIX} CACHE PATH "")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_PREFIX_PATH}/share/cmake/pybind11")

include(pybind11Tools)
include_directories("${CMAKE_PREFIX_PATH}/include")

# numpy
set(PYBIND11_CPP_STANDARD -std=c++14)
find_package(NumPy REQUIRED)
include_directories(${NUMPY_INCLUDE_DIRS})

#-------------------------------------------------------------------------------------------------------------------
# Add The module
#-------------------------------------------------------------------------------------------------------------------
pybind11_add_module(dvidutils src/main.cpp) # Also locates Python, so we can use e.g. PYTHON_SITE_PACKAGES
install(TARGETS dvidutils LIBRARY DESTINATION "${PYTHON_SITE_PACKAGES}")
