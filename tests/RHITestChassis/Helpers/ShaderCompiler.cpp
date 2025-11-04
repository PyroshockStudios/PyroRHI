// MIT License
//
// Copyright (c) 2025 Pyroshock Studios
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "ShaderCompiler.hpp"

#include <slang-com-helper.h>

#include <EASTL/hash_set.h>
#include <PyroCommon/Logger.hpp>
#include <PyroCommon/Serialization/BinarySerializer.hpp>
#include <PyroCommon/Stream/IStreamReader.hpp>
#include <PyroCommon/Stream/IStreamWriter.hpp>
#include <PyroRHI/Shader/IShaderFeatureSet.hpp>

#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>

#include <PyroCommon/Stream/FileStream.hpp>
#include <libassert/assert.hpp>

using namespace Slang;

using namespace PyroshockStudios;
using namespace PyroshockStudios::Types;
using namespace PyroshockStudios::RHI;
ShaderCompiler::ShaderCompiler(const IShaderFeatureSet* featureSet)
    : mFeatureSet(featureSet) {
    SlangGlobalSessionDesc desc{};
    desc.structureSize = sizeof(desc);
    desc.apiVersion = SLANG_API_VERSION;
    desc.minLanguageVersion = SLANG_LANGUAGE_VERSION_2025;
    desc.enableGLSL = featureSet->Features().bGLSL;

    SlangResult result = slang::createGlobalSession(&desc, &mGlobalSession);
    ASSERT(result == 0);
}

ShaderCompiler::~ShaderCompiler() {
    mGlobalSession->release();
    mGlobalSession = nullptr;
}

ShaderObject ShaderCompiler::CompileShaderFromSource(const eastl::string& code_, const ShaderCompilationInfo& info, const eastl::string& virtualSourcePath) {
    if (code_.empty()) {
        Logger::Error(mSink, "Slang received empty code! Returning...");
        return NULL_SHADER_OBJ;
    }

    eastl::vector<slang::PreprocessorMacroDesc> macros = {};
    for (const auto& macro : mFeatureSet->GlobalPreprocessorDefines()) {
        macros.emplace_back(macro.first, macro.second);
    }

    const char* profileName = mFeatureSet->GetProfileName(info.stage);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = (SlangCompileTarget)mFeatureSet->GetTarget();
    targetDesc.profile = mGlobalSession->findProfile(profileName);
    targetDesc.forceGLSLScalarBufferLayout = mFeatureSet->Features().bScalarLayout;

    eastl::vector<const char*> searchPaths = { };

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;
    sessionDesc.searchPaths = searchPaths.data();
    sessionDesc.searchPathCount = searchPaths.size();
    sessionDesc.preprocessorMacros = macros.data();
    sessionDesc.preprocessorMacroCount = static_cast<SlangInt>(macros.size());
    sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
    using namespace slang;

    slang::ISession* session = nullptr;

    SlangResult result = {};
    result = mGlobalSession->createSession(sessionDesc, &session);
    ASSERT(result == 0);

    Slang::ComPtr<SlangCompileRequest> slangRequest = nullptr;
    result = session->createCompileRequest(slangRequest.writeRef());
    ASSERT(result == 0);

    if (slangRequest == nullptr) {
        Logger::Error(mSink, "Slang failed to create a valid compiler request!");
        return NULL_SHADER_OBJ;
    }
    slangRequest->setTargetProfile(0, targetDesc.profile);

    eastl::vector<const char*> args = {
        // https://github.com/shader-slang/slang/issues/3532
        // Disables warning for aliasing bindings.
        // clang-format off
                "-warnings-disable", "39001",
                "-O0",
                "-g2",
        // clang-format on
    };
    result = slangRequest->processCommandLineArguments(args.data(), args.size());
    ASSERT(result == 0);

    for (const auto& [key, value] : info.defines) {
        slangRequest->addPreprocessorDefine(key.c_str(), value.c_str());
    }

    int translationUnitIndex = slangRequest->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, info.name.c_str());
    slangRequest->addTranslationUnitSourceString(translationUnitIndex, virtualSourcePath.empty() ? "PyroShader.slang" : virtualSourcePath.c_str(), code_.c_str());
    SlangStage stage = static_cast<SlangStage>(info.stage);
    slangRequest->addEntryPoint(translationUnitIndex, info.entryPoint.c_str(), stage);
    result = slangRequest->compile();
    const char* diagnostics = slangRequest->getDiagnosticOutput();
    if (diagnostics && strlen(diagnostics) > 0) {
        if (SLANG_FAILED(result)) {
            Logger::Error(mSink, "Slang failed to compile a shader! ERROR CODE: {}, Diagnostics: {}", SLANG_GET_RESULT_CODE(result), diagnostics ? diagnostics : "NO DIAGNOSTICS GENERTED!");
        } else {
            Logger::Warn(mSink, "Slang compiled shader successfully, but generated diagnostics: {}", diagnostics ? diagnostics : "NO DIAGNOSTICS GENERTED!");
        }
    }
    ASSERT(result == 0);
    if (SLANG_FAILED(result)) {
        return NULL_SHADER_OBJ;
    }

    Slang::ComPtr<slang::IModule> shaderModule = {};
    slangRequest->getModule(translationUnitIndex, shaderModule.writeRef());

    Slang::ComPtr<slang::IEntryPoint> entryp;
    result = shaderModule->getDefinedEntryPoint(0, entryp.writeRef());
    ASSERT(result == 0);
    Slang::ComPtr<slang::IBlob> bytecode;
    result = slangRequest->getEntryPointCodeBlob(0, 0, bytecode.writeRef());

    ShaderProgram program{};
    program.bytecode.resize(bytecode->getBufferSize());
    memcpy(program.bytecode.data(), bytecode->getBufferPointer(), bytecode->getBufferSize());
    session->release();
    return eastl::make_shared<ShaderProgram>(eastl::move(program));
}
