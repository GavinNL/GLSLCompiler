cmake_minimum_required(VERSION 3.10)

project( GLSLCompiler )


find_package(Threads)


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
                              cxx_std_17)

target_compile_definitions( GLSLCompiler
                                INTERFACE
                                    CMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

target_link_libraries( GLSLCompiler  INTERFACE
                                ${glslangTarget}
                                Threads::Threads
                                stdc++fs)

################################################################################








################################################################################
# Build the example executables
################################################################################
add_executable(        example main.cpp )
target_link_libraries( example PRIVATE GLSLCompiler stdc++fs )

add_executable(        quick_start quick_start.cpp )
target_link_libraries( quick_start PRIVATE GLSLCompiler stdc++fs)


add_executable(        example_include_paths example_include_paths.cpp )
target_link_libraries( example_include_paths PRIVATE GLSLCompiler stdc++fs)

################################################################################


add_subdirectory(test)
