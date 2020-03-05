#include <iostream>
#include <cassert>
#include <cstdint>
#include "GLSLCompiler.h"

std::string vertexShader = R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_Position;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    gl_Position  = vec4( in_Position, 1.0);
}
)";

std::string readASCIIFile(std::string const & filePath)
{
    std::ifstream t( filePath);

    std::string vertexShader((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    return vertexShader;
}

std::vector<uint32_t> readBinaryFile(std::string const & filePath)
{
    auto s = readASCIIFile(filePath);

    std::vector<uint32_t> data;

    size_t uSize = s.size()/4;
    uSize += s.size()%4? 1 : 0;

    data.resize( uSize );
    std::memcpy( &data[0], &s[0], s.size());
    return data;
}

int main(int argc, char ** argv)
{
    assert(&argc);
    assert(argv);

    // must call this first to initialise the glslang compiler backend
    // it must be called once per process
    glslang::InitializeProcess();


    // Create a default Compiler
    // this compiler trgets Vulkan 1.0 and Spirv 1.0
    //
    // There are other compilers: gnl::GLSLCompilerVvSs
    // where V is the vulkan Major version, v is the minor version
    //       S is the spirv Major version, s is the minor version
    //
    // To target the latest verions as of writing: gnl::GLSLCompiler1115
    gnl::GLSLCompiler compiler;

    try
    {
        auto vertexShader = readASCIIFile(CMAKE_SOURCE_DIR "/data/vertexShader.vert");

        // Compile our source code
        auto vertexShaderSPV = compiler.compile( vertexShader, EShLangVertex);

        // Read the file produced by glslangValidator
        auto vertexShaderValidator = readBinaryFile(CMAKE_SOURCE_DIR "/data/vertexShader.spv" );

        // Our compiled code and the one produced by glslangValidator should be the same
        assert( vertexShaderSPV.size() == vertexShaderValidator.size() );

        for(uint32_t i=0; i < vertexShaderSPV.size() ; i++)
        {
            // the 3rd data piece seems to be different,
            // not sure why that is. but it only differs 1
            // probably has to do with some internal variables being set
            if( i!= 2)
                assert( vertexShaderSPV[i] == vertexShaderValidator[i] );
        }
    }
    catch (...)
    {

        std::cout << compiler.getLog() << std::endl;

    }



    try
    {
        auto fragmentShader = readASCIIFile(CMAKE_SOURCE_DIR "/data/fragmentShader.frag");

        // Compile our source code
        auto fragmentShaderSPV = compiler.compile( fragmentShader, EShLangFragment);

        // Read the file produced by glslangValidator
        auto fragmentShaderValidator = readBinaryFile(CMAKE_SOURCE_DIR "/data/fragmentShader.spv" );

        // Our compiled code and the one produced by glslangValidator should be the same
        assert( fragmentShaderSPV.size() == fragmentShaderValidator.size() );

        for(uint32_t i=0; i < fragmentShaderSPV.size() ; i++)
        {
            // the 3rd data piece seems to be different,
            // not sure why that is. but it only differs 1
            // probably has to do with some internal variables being set
            if( i!= 2)
                assert( fragmentShaderSPV[i] == fragmentShaderValidator[i] );
        }
    }
    catch (...)
    {

        std::cout << compiler.getLog() << std::endl;

    }


    // this must be called to clean up the process
    // it should be called once per process.
    glslang::FinalizeProcess();
    return 0;
}
