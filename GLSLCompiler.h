#ifndef HEADER_ONLY_GLSLCOMPILER_HELPER_H
#define HEADER_ONLY_GLSLCOMPILER_HELPER_H


#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <fstream>
#include <iostream>
#include <filesystem>

//
// The following code is modified from:
//
// https://github.com/ForestCSharp/VkCppRenderer/blob/master/Src/Renderer/GLSL/ShaderCompiler.hpp
//

// When including this header in your projects: change the
// namespace to the namespace of your project
namespace gnl
{
/**
 * @brief The GLSLFileIncluder class
 *
 * This Includer is taken from the glslangValidator
 * source code. For some reason this class is not exported
 * by the library.
 */
class GLSLFileIncluder : public glslang::TShader::Includer
{
public:
    GLSLFileIncluder() : externalLocalDirectoryCount(0) { }

    virtual IncludeResult* includeLocal(const char* headerName,
                                        const char* includerName,
                                        size_t inclusionDepth) override
    {
        return readLocalPath(headerName, includerName, (int)inclusionDepth);
    }

    virtual IncludeResult* includeSystem(const char* headerName,
                                         const char* /*includerName*/,
                                         size_t /*inclusionDepth*/) override
    {
        return readSystemPath(headerName);
    }

    // Externally set directories. E.g., from a command-line -I<dir>.
    //  - Most-recently pushed are checked first.
    //  - All these are checked after the parse-time stack of local directories
    //    is checked.
    //  - This only applies to the "local" form of #include.
    //  - Makes its own copy of the path.
    virtual void pushExternalLocalDirectory(const std::string& dir)
    {
        directoryStack.push_back(dir);
        externalLocalDirectoryCount = static_cast<int>(directoryStack.size());
    }

    virtual void releaseInclude(IncludeResult* result) override
    {
        if (result != nullptr) {
            delete [] static_cast<tUserDataElement*>(result->userData);
            delete result;
        }
    }

    virtual ~GLSLFileIncluder() override { }

protected:
    typedef char tUserDataElement;
    std::vector<std::string> directoryStack;
    int externalLocalDirectoryCount;

    // Search for a valid "local" path based on combining the stack of include
    // directories and the nominal name of the header.
    virtual IncludeResult* readLocalPath(const char* headerName, const char* includerName, int depth)
    {
        // Discard popped include directories, and
        // initialize when at parse-time first level.
        directoryStack.resize( static_cast<size_t>(depth + externalLocalDirectoryCount) );
        if (depth == 1)
            directoryStack.back() = getDirectory(includerName);

        // Find a directory that works, using a reverse search of the include stack.
        for (auto it = directoryStack.rbegin(); it != directoryStack.rend(); ++it) {
            std::string path = *it + '/' + headerName;
            std::replace(path.begin(), path.end(), '\\', '/');
            std::ifstream file(path, std::ios_base::binary | std::ios_base::ate);
            if (file)
            {
                directoryStack.push_back(getDirectory(path));
                return newIncludeResult(path, file, static_cast<int>(file.tellg()) );
            }
        }

        return nullptr;
    }

    // Search for a valid <system> path.
    // Not implemented yet; returning nullptr signals failure to find.
    virtual IncludeResult* readSystemPath(const char* /*headerName*/) const
    {
        return nullptr;
    }

    // Do actual reading of the file, filling in a new include result.
    virtual IncludeResult* newIncludeResult(const std::string& path, std::ifstream& file, int length) const
    {
        char* content = new tUserDataElement [ length ];
        file.seekg(0, file.beg);
        file.read(content, length);
        return new IncludeResult(path, content, length, content);
    }

    // If no path markers, return current working directory.
    // Otherwise, strip file name and return path leading up to it.
    virtual std::string getDirectory(const std::string path) const
    {
        size_t last = path.find_last_of("/\\");
        return last == std::string::npos ? "." : path.substr(0, last);
    }
};


template<glslang::EShTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_0,
         glslang::EShTargetLanguageVersion TargetVersion     = glslang::EShTargetSpv_1_0>
class GLSLCompiler_t
{
    GLSLFileIncluder m_includer;
    std::string      m_log;
    std::string      m_debug;
    std::string      m_preamble;
public:

    /**
     * @brief addCompleTimeDefinition
     * @param var
     * @param value
     *
     * Adds a compile time definition to the preamble. etc
     *
     * addCompileTimeDefinition("MYVALUE", "2")
     * results in adding
     *
     * #define MYVALUE 2
     *
     * to the top of the shader source code
     */
    void addCompleTimeDefinition( const std::string var, const std::string value="")
    {
        m_preamble += "#define " + var + ' ' + value + '\n';
    }
    std::string const& getLog() const
    {
        return m_log;
    }
    std::string const& getDebugLog() const
    {
        return m_debug;
    }
    void addIncludePath( const std::string & path)
    {
        m_includer.pushExternalLocalDirectory(path);
    }

    std::vector<unsigned int> compile(const std::string & InputGLSL, EShLanguage ShaderType)
    {
        auto resources = getDefaultTBuiltInResource();
        return compile(InputGLSL, ShaderType, resources);
    }


    std::vector<unsigned int> compile(const std::string & InputGLSL, EShLanguage ShaderType, TBuiltInResource const & Resources)
    {
        m_log.clear();
        m_debug.clear();

        const char* InputCString = InputGLSL.c_str();

        glslang::TShader Shader(ShaderType);

        Shader.setPreamble(m_preamble.data());
        Shader.setStrings(&InputCString, 1);

        //Set up Vulkan/SpirV Environment
        int ClientInputSemanticsVersion = 100; // maps to, say, #define VULKAN 100

        Shader.setEnvInput(glslang::EShSourceGlsl, ShaderType, glslang::EShClientVulkan, ClientInputSemanticsVersion);
        Shader.setEnvClient(glslang::EShClientVulkan, VulkanClientVersion);
        Shader.setEnvTarget(glslang::EShTargetSpv, TargetVersion);

        EShMessages messages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);

        const int DefaultVersion = 100;

        std::string PreprocessedGLSL;

        if (!Shader.preprocess(&Resources, DefaultVersion, ENoProfile, false, false, messages, &PreprocessedGLSL, m_includer))
        {
            m_log   = Shader.getInfoLog();
            m_debug = Shader.getInfoDebugLog();
            throw std::runtime_error( "Failed to preprocess" );
        }


        const char* PreprocessedCStr = PreprocessedGLSL.c_str();
        Shader.setStrings(&PreprocessedCStr, 1);

        if (!Shader.parse(&Resources, DefaultVersion, false, messages))
        {
            m_log   = Shader.getInfoLog();
            m_debug = Shader.getInfoDebugLog();

            throw std::runtime_error( "Parsing Failed" );
        }

        glslang::TProgram Program;
        Program.addShader(&Shader);

        if(!Program.link(messages))
        {
            m_log   = Shader.getInfoLog();
            m_debug = Shader.getInfoDebugLog();

            throw std::runtime_error( "Linking Failed" );
        }

        // if (!Program.mapIO())
        // {
        //      m_log   = Shader.getInfoLog();
        //      m_debug = Shader.getInfoDebugLog();
        // 	std::cout << "GLSL Linking (Mapping IO) Failed for: " << filename << std::endl;
        // 	std::cout << Shader.getInfoLog() << std::endl;
        // 	std::cout << Shader.getInfoDebugLog() << std::endl;
        // }

        std::vector<unsigned int> SpirV;
        spv::SpvBuildLogger logger;
        glslang::SpvOptions spvOptions;
        glslang::GlslangToSpv(*Program.getIntermediate(ShaderType), SpirV, &logger, &spvOptions);

        if (logger.getAllMessages().length() > 0)
        {
            m_log = logger.getAllMessages();
        }

        return SpirV;
    }

    /**
     * @brief getDefaultTBuiltInResource
     * @return
     *
     * Get the default built-in resources as defined by the glslangValidator
     */
    static TBuiltInResource getDefaultTBuiltInResource()
    {
        TBuiltInResource DefaultTBuiltInResource;

        DefaultTBuiltInResource.maxLights                                   = 32;
        DefaultTBuiltInResource.maxClipPlanes                               = 6;
        DefaultTBuiltInResource.maxTextureUnits                             = 32;
        DefaultTBuiltInResource.maxTextureCoords                            = 32;
        DefaultTBuiltInResource.maxVertexAttribs                            = 64;
        DefaultTBuiltInResource.maxVertexUniformComponents                  = 4096;
        DefaultTBuiltInResource.maxVaryingFloats                            = 64;
        DefaultTBuiltInResource.maxVertexTextureImageUnits                  = 32;
        DefaultTBuiltInResource.maxCombinedTextureImageUnits                = 80;
        DefaultTBuiltInResource.maxTextureImageUnits                        = 32;
        DefaultTBuiltInResource.maxFragmentUniformComponents                = 4096;
        DefaultTBuiltInResource.maxDrawBuffers                              = 32;
        DefaultTBuiltInResource.maxVertexUniformVectors                     = 128;
        DefaultTBuiltInResource.maxVaryingVectors                           = 8;
        DefaultTBuiltInResource.maxFragmentUniformVectors                   = 16;
        DefaultTBuiltInResource.maxVertexOutputVectors                      = 16;
        DefaultTBuiltInResource.maxFragmentInputVectors                     = 15;
        DefaultTBuiltInResource.minProgramTexelOffset                       = 8;
        DefaultTBuiltInResource.maxProgramTexelOffset                       = 7;
        DefaultTBuiltInResource.maxClipDistances                            = 8;
        DefaultTBuiltInResource.maxComputeWorkGroupCountX                   = 65535;
        DefaultTBuiltInResource.maxComputeWorkGroupCountY                   = 65535;
        DefaultTBuiltInResource.maxComputeWorkGroupCountZ                   = 65535;
        DefaultTBuiltInResource.maxComputeWorkGroupSizeX                    = 1024;
        DefaultTBuiltInResource.maxComputeWorkGroupSizeY                    = 1024;
        DefaultTBuiltInResource.maxComputeWorkGroupSizeZ                    = 64;
        DefaultTBuiltInResource.maxComputeUniformComponents                 = 1024;
        DefaultTBuiltInResource.maxComputeTextureImageUnits                 = 16;
        DefaultTBuiltInResource.maxComputeImageUniforms                     = 8;
        DefaultTBuiltInResource.maxComputeAtomicCounters                    = 8;
        DefaultTBuiltInResource.maxComputeAtomicCounterBuffers              = 1;
        DefaultTBuiltInResource.maxVaryingComponents                        = 60;
        DefaultTBuiltInResource.maxVertexOutputComponents                   = 64;
        DefaultTBuiltInResource.maxGeometryInputComponents                  = 64;
        DefaultTBuiltInResource.maxGeometryOutputComponents                 = 128;
        DefaultTBuiltInResource.maxFragmentInputComponents                  = 128;
        DefaultTBuiltInResource.maxImageUnits                               = 8;
        DefaultTBuiltInResource.maxCombinedImageUnitsAndFragmentOutputs     = 8;
        DefaultTBuiltInResource.maxCombinedShaderOutputResources            = 8;
        DefaultTBuiltInResource.maxImageSamples                             = 0;
        DefaultTBuiltInResource.maxVertexImageUniforms                      = 0;
        DefaultTBuiltInResource.maxTessControlImageUniforms                 = 0;
        DefaultTBuiltInResource.maxTessEvaluationImageUniforms              = 0;
        DefaultTBuiltInResource.maxGeometryImageUniforms                    = 0;
        DefaultTBuiltInResource.maxFragmentImageUniforms                    = 8;
        DefaultTBuiltInResource.maxCombinedImageUniforms                    = 8;
        DefaultTBuiltInResource.maxGeometryTextureImageUnits                = 16;
        DefaultTBuiltInResource.maxGeometryOutputVertices                   = 256;
        DefaultTBuiltInResource.maxGeometryTotalOutputComponents            = 1024;
        DefaultTBuiltInResource.maxGeometryUniformComponents                = 1024;
        DefaultTBuiltInResource.maxGeometryVaryingComponents                = 64;
        DefaultTBuiltInResource.maxTessControlInputComponents               = 128;
        DefaultTBuiltInResource.maxTessControlOutputComponents              = 128;
        DefaultTBuiltInResource.maxTessControlTextureImageUnits             = 16;
        DefaultTBuiltInResource.maxTessControlUniformComponents             = 1024;
        DefaultTBuiltInResource.maxTessControlTotalOutputComponents         = 4096;
        DefaultTBuiltInResource.maxTessEvaluationInputComponents            = 128;
        DefaultTBuiltInResource.maxTessEvaluationOutputComponents           = 128;
        DefaultTBuiltInResource.maxTessEvaluationTextureImageUnits          = 16;
        DefaultTBuiltInResource.maxTessEvaluationUniformComponents          = 1024;
        DefaultTBuiltInResource.maxTessPatchComponents                      = 120;
        DefaultTBuiltInResource.maxPatchVertices                            = 32;
        DefaultTBuiltInResource.maxTessGenLevel                             = 64;
        DefaultTBuiltInResource.maxViewports                                = 16;
        DefaultTBuiltInResource.maxVertexAtomicCounters                     = 0;
        DefaultTBuiltInResource.maxTessControlAtomicCounters                = 0;
        DefaultTBuiltInResource.maxTessEvaluationAtomicCounters             = 0;
        DefaultTBuiltInResource.maxGeometryAtomicCounters                   = 0;
        DefaultTBuiltInResource.maxFragmentAtomicCounters                   = 8;
        DefaultTBuiltInResource.maxCombinedAtomicCounters                   = 8;
        DefaultTBuiltInResource.maxAtomicCounterBindings                    = 1;
        DefaultTBuiltInResource.maxVertexAtomicCounterBuffers               = 0;
        DefaultTBuiltInResource.maxTessControlAtomicCounterBuffers          = 0;
        DefaultTBuiltInResource.maxTessEvaluationAtomicCounterBuffers       = 0;
        DefaultTBuiltInResource.maxGeometryAtomicCounterBuffers             = 0;
        DefaultTBuiltInResource.maxFragmentAtomicCounterBuffers             = 1;
        DefaultTBuiltInResource.maxCombinedAtomicCounterBuffers             = 1;
        DefaultTBuiltInResource.maxAtomicCounterBufferSize                  = 16384;
        DefaultTBuiltInResource.maxTransformFeedbackBuffers                 = 4;
        DefaultTBuiltInResource.maxTransformFeedbackInterleavedComponents   = 64;
        DefaultTBuiltInResource.maxCullDistances                            = 8;
        DefaultTBuiltInResource.maxCombinedClipAndCullDistances             = 8;
        DefaultTBuiltInResource.maxSamples                                  = 4;
        DefaultTBuiltInResource.limits.nonInductiveForLoops                 = 1;
        DefaultTBuiltInResource.limits.whileLoops                           = 1;
        DefaultTBuiltInResource.limits.doWhileLoops                         = 1;
        DefaultTBuiltInResource.limits.generalUniformIndexing               = 1;
        DefaultTBuiltInResource.limits.generalAttributeMatrixVectorIndexing = 1;
        DefaultTBuiltInResource.limits.generalVaryingIndexing               = 1;
        DefaultTBuiltInResource.limits.generalSamplerIndexing               = 1;
        DefaultTBuiltInResource.limits.generalVariableIndexing              = 1;
        DefaultTBuiltInResource.limits.generalConstantMatrixVectorIndexing  = 1;

        return DefaultTBuiltInResource;
    }


    static std::vector<uint32_t> compileFromFile(std::filesystem::path const &P, std::vector<std::filesystem::path> const & includePaths = {})
    {
        GLSLCompiler_t compiler;

        auto ext = P.extension();

        compiler.addIncludePath( P.parent_path().string() );
        for(auto & ii : includePaths)
        {
            compiler.addIncludePath( ii.string() );
        }

        std::ifstream t(P);

        if( t )
        {
            std::string srcString((std::istreambuf_iterator<char>(t)),
                             std::istreambuf_iterator<char>());

            if( ext == ".vert")
            {
                return compiler.compile( srcString, EShLangVertex);
            }
            else if( ext == ".frag")
            {
                return compiler.compile( srcString, EShLangFragment);
            }
            else if( ext == ".comp")
            {
                return compiler.compile( srcString, EShLangCompute);
            }
            else if( ext == ".tesc")
            {
                return compiler.compile( srcString, EShLangTessControl);
            }
            else if( ext == ".tese")
            {
                return compiler.compile( srcString, EShLangTessEvaluation);
            }
            else if( ext == ".geom")
            {
                return compiler.compile( srcString, EShLangGeometry);
            }
            throw  std::runtime_error("Could not determine shader language, files must have extensions: vert, frag, comp, tesc, tese, geom.");
        }
        throw  std::runtime_error("Error opening file.");
    }
};

using GLSLCompiler     = GLSLCompiler_t<glslang::EShTargetVulkan_1_0, glslang::EShTargetSpv_1_0>;

using GLSLCompiler1010 = GLSLCompiler_t<glslang::EShTargetVulkan_1_0, glslang::EShTargetSpv_1_0>;
using GLSLCompiler1011 = GLSLCompiler_t<glslang::EShTargetVulkan_1_0, glslang::EShTargetSpv_1_1>;
using GLSLCompiler1012 = GLSLCompiler_t<glslang::EShTargetVulkan_1_0, glslang::EShTargetSpv_1_2>;
using GLSLCompiler1013 = GLSLCompiler_t<glslang::EShTargetVulkan_1_0, glslang::EShTargetSpv_1_3>;
using GLSLCompiler1014 = GLSLCompiler_t<glslang::EShTargetVulkan_1_0, glslang::EShTargetSpv_1_4>;
using GLSLCompiler1015 = GLSLCompiler_t<glslang::EShTargetVulkan_1_0, glslang::EShTargetSpv_1_5>;


using GLSLCompiler1110 = GLSLCompiler_t<glslang::EShTargetVulkan_1_1, glslang::EShTargetSpv_1_0>;
using GLSLCompiler1111 = GLSLCompiler_t<glslang::EShTargetVulkan_1_1, glslang::EShTargetSpv_1_1>;
using GLSLCompiler1112 = GLSLCompiler_t<glslang::EShTargetVulkan_1_1, glslang::EShTargetSpv_1_2>;
using GLSLCompiler1113 = GLSLCompiler_t<glslang::EShTargetVulkan_1_1, glslang::EShTargetSpv_1_3>;
using GLSLCompiler1114 = GLSLCompiler_t<glslang::EShTargetVulkan_1_1, glslang::EShTargetSpv_1_4>;
using GLSLCompiler1115 = GLSLCompiler_t<glslang::EShTargetVulkan_1_1, glslang::EShTargetSpv_1_5>;

}

#endif 


