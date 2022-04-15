cmake_minimum_required(VERSION 3.10)

project( GLSLCompiler )

################################################################################
# Don't build examples if we are a submodule.
################################################################################
get_directory_property(_has_parent PARENT_DIRECTORY)
if(_has_parent)
    message("Not building examples")
    option(GLSLCOMPILER_BUILD_EXAMPLES "Build examples" OFF)
else()
    option(GLSLCOMPILER_BUILD_EXAMPLES "Build examples" ON)
endif()
################################################################################


find_package(Threads)

################################################################################
# Create a simple interface library to link with our exectuables
# The compiler provided by this repo is a header-only library, but still
# requires the glslang libraries
################################################################################
add_library( GLSLCompiler INTERFACE )

target_include_directories( GLSLCompiler
                            INTERFACE
                               "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
)

target_compile_features( GLSLCompiler
                          INTERFACE
                              cxx_std_11)

target_link_libraries( GLSLCompiler  INTERFACE Threads::Threads)
################################################################################












if(${GLSLCOMPILER_BUILD_EXAMPLES})

################################################################################
# If the conan file exists, include it
################################################################################
if( EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake )
    include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup(NO_OUTPUT_DIRS TARGETS)
    set(glslangTarget CONAN_PKG::glslang)
else()
    set(glslangTarget SPIRV glslang HLSL OGLCompiler OSDependent)
endif()
################################################################################


################################################################################
# Build the example executables
################################################################################
add_executable(        example main.cpp )
target_link_libraries( example PRIVATE GLSLCompiler ${glslangTarget} Threads::Threads)
target_compile_definitions( example PRIVATE CMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

add_executable(        quick_start quick_start.cpp )
target_link_libraries( quick_start PRIVATE GLSLCompiler ${glslangTarget} Threads::Threads)
target_compile_definitions( quick_start PRIVATE CMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")


add_executable(        example_include_paths example_include_paths.cpp )
target_link_libraries( example_include_paths PRIVATE GLSLCompiler ${glslangTarget} Threads::Threads)
target_compile_definitions( example_include_paths PRIVATE CMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
################################################################################

endif()





################################################################################
# This doesn't seem to work on Ubuntu 18.04 because the cmake files
# provided by glslang-dev are not installed in the correct path
# see: https://github.com/KhronosGroup/glslang/pull/1978
#
# find_package(glslang REQUIRED)
#
# So instead I'm manually using the libraries I need.
# on ubuntu these libraries can be installed using
#
#            sudo apt install glslang-dev
#
################################################################################

# Possibly need these??
#set(glslangTarget SPVRemapper SPIRV glslang HLSL SPVRemapper OGLCompiler OSDependent)




################################################################################













