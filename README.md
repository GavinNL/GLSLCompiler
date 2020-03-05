# GLSL Compiler

A very simple header-only library to help build a
run-time GLSL -> SPIRV compiler for your Vulkan Projects.

Simply copy `GLSLCompiler.h` to your project and rename the namespace to
the namespace for your project.

## Compile The Examples

Compile the examples:
```Bash
git clone https://github.com/GavinNL/GLSLCompiler.git
cd GLSLCompiler
mkdir build  && cd build

cmake ..

cmake --build .
```


## Quick Start Code

Here is Quick-start code to get you compiling GLSL code. You will need to
link with the following libraries provided by the glslang: `-lSPIRV -lglslang -lHLSL -lOGLCompiler -lOSDependent`

```C++

#include <iostream>
#include "GLSLCompiler.h"

int main(int argc, char ** argv)
{

    // must call this first to initialise the glslang compiler backend
    // it must be called once per process
    glslang::InitializeProcess();

    // Create a base compiler which targets Vulkan 1.0 and Spirv 1.0
    // Other compilers exist: eg: GLSLCompiler1115 -> targets vulkan 1.1 and spirv 1.5
    gnl::GLSLCompiler compiler;

    // Add a compile-time definition ( #define DEFAULT_COLOR vec3(1,1,1) ) to the top of any shader
    // compiled with this compiler.
    compiler.addCompleTimeDefinition("DEFAULT_COLOR", "vec3(1,1,1)");


    std::string vertexShader = R"(

    #version 450
    #extension GL_ARB_separate_shader_objects : enable

    layout(location = 0) in vec3  in_Position;

    layout(location = 0) out vec3 f_Position;

    out gl_PerVertex
    {
        vec4 gl_Position;
    };

    void main()
    {
        gl_Position  = vec4( in_Position, 1.0);
        f_Position   = in_Position;
    }

    )";


    std::string fragmentShader = R"(

    #version 450
    #extension GL_ARB_separate_shader_objects : enable

    layout(location = 0) in vec3 f_Position;

    layout(location = 0) out vec4 outColor;

    void main()
    {
        outColor   = vec4( DEFAULT_COLOR , 1);
    }
    )";

    // will throw exceptions if fails;
    try
    {
        auto vertexShaderSPV   = compiler.compile( vertexShader  , EShLangVertex);
        auto fragmentShaderSPV = compiler.compile( fragmentShader, EShLangFragment);

        assert( vertexShaderSPV.size() > 0);
        assert( fragmentShaderSPV.size() > 0);
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


```
