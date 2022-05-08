cmake_minimum_required(VERSION 3.10)

project( GLSLCompiler )


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



################################################################################






get_directory_property( _is_sub_project PARENT_DIRECTORY)

if(NOT _is_sub_project)


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

    target_link_libraries( GLSLCompiler  INTERFACE
                                    ${glslangTarget}
                                    Threads::Threads
                                    )

    ################################################################################
    # Build the example executables
    ################################################################################
    add_executable(        example main.cpp )
    target_link_libraries( example PRIVATE GLSLCompiler )

    add_executable(        quick_start quick_start.cpp )
    target_link_libraries( quick_start PRIVATE GLSLCompiler )


    add_executable(        example_include_paths example_include_paths.cpp )
    target_link_libraries( example_include_paths PRIVATE GLSLCompiler)

    ################################################################################

    enable_testing()
    add_subdirectory(test)
else()

    message("GLSLCompiler folder is not the main project. Not building examples/tests")

endif()


