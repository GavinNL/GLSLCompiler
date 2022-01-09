# Create a static library for Catch2s main so that we can reduce
# compiling time. Each unit test will link to this
cmake_minimum_required(VERSION 3.13)


find_package(Catch2)

if(${Catch2_FOUND})

    add_library(${PROJECT_NAME}-catchmain STATIC ${CMAKE_CURRENT_SOURCE_DIR}/catch-main.cpp)
    target_include_directories(${PROJECT_NAME}-catchmain PUBLIC third_party)
    target_compile_features(${PROJECT_NAME}-catchmain PUBLIC cxx_std_17)
    target_link_libraries( ${PROJECT_NAME}-catchmain PUBLIC   Catch2::Catch2)

else()
    message("**************************************************")
    message("              WARNING ")
    message("**************************************************")
    message("Could not find Catch2. Unit tests will not be built ")
    message("")
    message("Execute the following to build the unit tests")
    message("")
    message("  cd ${CMAKE_BINARY_DIR}")
    message("  conan install ${CMAKE_SOURCE_DIR}")
    message("  cmake ${CMAKE_BINARY_DIR} -DCMAKE_MODULE_PATH=$PWD")
    message("")
    message("**************************************************")
    return()
endif()


get_filename_component(folder_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" folder_name ${folder_name})


enable_testing()


file(GLOB files "unit-*.cpp")
foreach(file ${files})

    # Replace any files named test_XXXXX with XXXX
    get_filename_component(file_basename ${file} NAME_WE)
    string(REGEX REPLACE "unit-([^$]+)" "\\1" testcase ${file_basename})

    set(FILE_SUFFIX_NAME  ${testcase} )
    set(EXE_NAME     unit-${MODULE_NAME}-${FILE_SUFFIX_NAME} )
    set(TEST_NAME    test-${MODULE_NAME}-${FILE_SUFFIX_NAME} )
    set(SRC_NAME     unit-${FILE_SUFFIX_NAME}.cpp )

    set(exeCmd  ${EXE_NAME} )
    set(envVar  "" )


    add_executable( ${EXE_NAME}  ${SRC_NAME} )

    target_link_libraries( ${EXE_NAME}
                                PUBLIC
                                    ${UNIT_TEST_LINK_TARGETS}
                                    ${LINKED_TARGETS})

    target_link_libraries( ${EXE_NAME}
                                PRIVATE
                                    ${PROJECT_NAME}-catchmain
                                    ${PROJECT_NAME})

    add_test(  NAME    ${TEST_NAME}
               COMMAND ${exeCmd}
            )
    set_tests_properties(${TEST_NAME}
                            PROPERTIES
                                ENVIRONMENT
                                    "${envVar}")



    message("--------------------------------------------------")
    message("Unit Test Added: ${TEST_NAME}")
    message("--------------------------------------------------")
    message("Souce File       : ${SRC_NAME}")
    message("Working Directory: ${CMAKE_CURRENT_BINARY_DIR}")
    message("Env Variables    : ${envVar}")
    message("Command          : ${exeCmd}")
    #set_coverage_flags(${EXE_NAME})
    #set_project_warnings(${EXE_NAME})
endforeach()

return()
