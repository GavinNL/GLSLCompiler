cmake_minimum_required(VERSION 3.10)

project( GLSLCompiler )


find_package(Threads)

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
#set(glslangLibraries SPVRemapper SPIRV glslang HLSL SPVRemapper OGLCompiler OSDependent)

set(glslangLibraries SPIRV glslang HLSL OGLCompiler OSDependent)


################################################################################



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

target_compile_definitions( GLSLCompiler
                                INTERFACE
                                    CMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

target_link_libraries( GLSLCompiler  INTERFACE
                                ${glslangLibraries}
                                Threads::Threads)

################################################################################








################################################################################
# Build the example executables
################################################################################
add_executable(        example main.cpp )
target_link_libraries( example PRIVATE GLSLCompiler )

add_executable(        quick_start quick_start.cpp )
target_link_libraries( quick_start PRIVATE GLSLCompiler )


add_executable(        example_include_paths example_include_paths.cpp )
target_link_libraries( example_include_paths PRIVATE GLSLCompiler )

################################################################################
