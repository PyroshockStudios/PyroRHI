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

#pragma once
#include <PyroCommon/LoggerInterface.hpp>
#include <PyroRHI/Shader/IShaderFeatureSet.hpp>

#include <EASTL/shared_ptr.h>
#include <EASTL/string.h>
#include <EASTL/string_map.h>
#include <EASTL/vector.h>
#include <slang-com-ptr.h>

struct ShaderCompilationInfo {
    PyroshockStudios::RHI::ShaderStage stage = {};
    eastl::string entryPoint = "main";
    eastl::vector<eastl::pair<eastl::string /*macro*/, eastl::string /*value*/>> defines = {};
    eastl::string name = "PyroShader";

    inline bool operator==(const ShaderCompilationInfo& o) const noexcept {
        return stage == o.stage && entryPoint == o.entryPoint && defines == o.defines && name == o.name;
    }
    inline bool operator!=(const ShaderCompilationInfo& o) const noexcept {
        return !(*this == o);
    }
};

using ShaderObject = eastl::shared_ptr<PyroshockStudios::RHI::ShaderProgram>;
static const inline ShaderObject NULL_SHADER_OBJ = ShaderObject(nullptr);

class ShaderCompiler : public PyroshockStudios::ILoggerAware, PyroshockStudios::DeleteCopy, PyroshockStudios::DeleteMove {
public:
    ShaderCompiler(const PyroshockStudios::RHI::IShaderFeatureSet* featureSet);
    ~ShaderCompiler();

    ShaderObject CompileShaderFromSource(const eastl::string& code, const ShaderCompilationInfo& info, const eastl::string& virtualSourcePath = {});

    void InjectLogger(PyroshockStudios::ILogStream* stream) {
        mSink = stream;
    }

private:
    PyroshockStudios::ILogStream* mSink = nullptr;
    slang::IGlobalSession* mGlobalSession = nullptr;
    const PyroshockStudios::RHI::IShaderFeatureSet* mFeatureSet;
};
