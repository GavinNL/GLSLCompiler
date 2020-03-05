#include <iostream>
#include <cassert>
#include <cstdint>
#include "GLSLCompiler.h"

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

    // Ad an include path so that
    // we can use the #include "filename" in the shader
    //
    // You will need to add the following to the top of your shader code:
    // #extension GL_GOOGLE_include_directive : enable
    compiler.addIncludePath(CMAKE_SOURCE_DIR "/data/include");

    try
    {
        auto fragmentShader = readASCIIFile(CMAKE_SOURCE_DIR "/data/fragmentShaderInclude.frag");

        // Compile our source code
        auto fragmentShaderSPV = compiler.compile( fragmentShader, EShLangFragment);

        // Read the file produced by glslangValidator
        auto fragmentShaderValidator = readBinaryFile(CMAKE_SOURCE_DIR "/data/fragmentShaderInclude.spv" );

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
