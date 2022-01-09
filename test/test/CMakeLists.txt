# Create a static library for Catch2s main so that we can reduce
# compiling time. Each unit test will link to this
cmake_minimum_required(VERSION 3.13)


find_package(Catch2)

if(${Catch2_FOUND})

    add_library(${PROJECT_NAME}-catchmain STATIC ${CMAKE_CURRENT_SOURCE_DIR}/catch-main.cpp)
    target_include_directories(${PROJECT_NAME}-catchmain PUBLIC third_party)
    target_compile_features(${PROJECT_NAME}-catchmain PUBLIC cxx_std_17)
    target_link_libraries( ${PROJECT_NAME}-catchmain PUBLIC   Catch2::Catch2)
    set_coverage_flags(${PROJECT_NAME}-catchmain)

    if( ${PROJECT_NAME}_ENABLE_COVERAGE )
       set_coverage_flags(${PROJECT_NAME}-catchmain)
    endif()

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

return()
