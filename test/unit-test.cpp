#include <catch2/catch.hpp>
#include <GLSLCompiler.h>


SCENARIO("Compile a Vertex Shader")
{
    glslang::InitializeProcess();

    gnl::GLSLCompiler compiler;

    auto spv = compiler.compileFromFile(CMAKE_SOURCE_DIR "/data/vertexShader.vert");

    REQUIRE( spv.size() > 0);

    glslang::FinalizeProcess();
}


SCENARIO("Compile a Fragment Shader")
{
    glslang::InitializeProcess();

    gnl::GLSLCompiler compiler;

    auto spv = compiler.compileFromFile(CMAKE_SOURCE_DIR "/data/fragmentShader.frag");

    REQUIRE( spv.size() > 0);

    glslang::FinalizeProcess();
}

SCENARIO("Compile a Compute Shader")
{
    glslang::InitializeProcess();

    gnl::GLSLCompiler compiler;

    auto spv = compiler.compileFromFile(CMAKE_SOURCE_DIR "/data/genBRDF.comp");

    REQUIRE( spv.size() > 0);

    glslang::FinalizeProcess();
}

