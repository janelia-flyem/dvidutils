CMAKE_MINIMUM_REQUIRED(VERSION 3.30)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake )

SET(PROJECT_NAME "dvidutils")
project(dvidutils)

include(CheckCXXCompilerFlag)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

    # See:
    # https://github.com/llvm/llvm-project/issues/91504
    # https://github.com/exasmr/openmc/commit/62bb754ed4a2d21624bc9d67992fc58710593358#diff-1712b235e02bd4a3ad866fe26447a6dc5c1b449faeea513f449d244d46d03cf5R42
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-relaxed-template-template-args")
endif()


#-------------------------------------------------------------------------------------------------------------------
# include include files
#-------------------------------------------------------------------------------------------------------------------
#file(GLOB_RECURSE headers include/*.hxx)
include_directories(src)


# Support deduplication in draco encoding.
add_definitions("-DDRACO_ATTRIBUTE_DEDUPLICATION_SUPPORTED=1")

#-------------------------------------------------------------------------------------------------------------------
# Setup python bindings
#-------------------------------------------------------------------------------------------------------------------
# CMAKE_PREFIX_PATH is expected to be set by the caller (e.g., build script)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_PREFIX_PATH}/share/cmake/pybind11")

# Python_ROOT_DIR and Python_EXECUTABLE can also be set by the caller if needed,
# but find_package should use CMAKE_PREFIX_PATH and Python_ROOT_DIR.

# Find Python first (required by pybind11)
# CMake should find Python components using CMAKE_PREFIX_PATH and Python_ROOT_DIR
find_package(Python COMPONENTS Interpreter Development.SABIModule NumPy REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

# Explicitly add the general prefix include path for headers (like Boost)
include_directories(${CMAKE_PREFIX_PATH}/include)

# Set NumPy include dirs from the found package
include_directories(${Python_NumPy_INCLUDE_DIRS})

# boost - Removed FIND_PACKAGE as only headers are needed and assumed to be in CMAKE_PREFIX_PATH/include

#-------------------------------------------------------------------------------------------------------------------
# Add the package
#-------------------------------------------------------------------------------------------------------------------

set(DVIDUTILS_BUILD_OUTPUT_DIR "${CMAKE_BINARY_DIR}/$<CONFIG>")
set(DVIDUTILS_PACKAGE "${DVIDUTILS_BUILD_OUTPUT_DIR}/dvidutils")

# Target for the compiled module -- install to <build-dir>/dvidutils/_dvidutils.XXX
# https://github.com/pybind/pybind11/issues/3907#issuecomment-1170561643
pybind11_add_module(_dvidutils MODULE src/main.cpp) # Also locates Python, so we can use e.g. PYTHON_SITE_PACKAGES
set_target_properties(_dvidutils PROPERTIES PREFIX "")

# Link to draco
# NOTE: By default, draco doesn't build shared libs, but our conda recipe for it does.
#       If we ever switch to conda-forge's package for draco, we'll have to change this to the static libs.
if (APPLE)
	target_link_libraries(_dvidutils PRIVATE libdraco.dylib libdracoenc.dylib libdracodec.dylib)
else()
	target_link_libraries(_dvidutils PRIVATE libdraco.so libdracoenc.so libdracodec.so)
endif()

set_target_properties(_dvidutils PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${DVIDUTILS_PACKAGE}")

# Target to copy the python sources to the build output
add_custom_target(dvidutils
     ALL
     COMMAND mkdir -p "${DVIDUTILS_PACKAGE}"
     COMMAND cp -R "${CMAKE_SOURCE_DIR}/dvidutils/*" "${DVIDUTILS_PACKAGE}/"
)
add_dependencies(dvidutils _dvidutils)


#
# Tests
#
find_program(PYTEST_EXECUTABLE pytest)
add_custom_target(pytests
     COMMAND "PYTHONPATH=${DVIDUTILS_BUILD_OUTPUT_DIR}" "${PYTEST_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tests"
)
add_dependencies(pytests dvidutils)

#
# INSTALL
#
# python sources were already copied into the build output,
# so just copy the entire build output to the install dir
install(DIRECTORY "${DVIDUTILS_PACKAGE}" DESTINATION "${Python_SITEARCH}")

