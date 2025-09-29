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
#include <PyroCommon/Types.hpp>
#include <PyroCommon/GUID.hpp>
#include <PyroCommon/Version.hpp>

#define PYRO_RHI_NAME_MAX_LENGTH (32)
#define PYRO_RHI_AUTH_MAX_LENGTH (32)
#define PYRO_RHI_SHORTHAND_MAX_LENGTH (8)
#define PYRO_RHI_MAX_OPTIONS (16)
#define PYRO_RHI_OPTION_NAME_MAX_LENGTH (16)

namespace PyroshockStudios {
    inline namespace RHI {
        class RHIContext;

        enum struct RHIOptionValueType : u32 {
            Flag = 0,
            Boolean,
            Integer,
            String,
            Array
        };
        struct RHIOptionInfo {
            char name[PYRO_RHI_OPTION_NAME_MAX_LENGTH] = "";
            RHIOptionValueType valueType = {};
        };

        struct RHIOptionData {
            i32 optionIndex = -1;

            char** strValueArray = nullptr;
            usize numStrValues = 0;
            bool boolValue = false;
            i64 intValue = 0;
        };

        // It is REQUIRED to have the following options:
        // -debug [valueType = Flag], enables any minimally available debug layers (e.g. vulkan Validation layers)
        // -device [valueType = Integer], overrides default device selection if possible
        struct RHIInfo {
            GUID guid = GUID::Invalid();
            // Used for command line arguments
            char shorthand[PYRO_RHI_SHORTHAND_MAX_LENGTH] = "";
            char name[PYRO_RHI_NAME_MAX_LENGTH] = "";
            char author[PYRO_RHI_AUTH_MAX_LENGTH] = "";
            RHIOptionInfo availableOptions[PYRO_RHI_MAX_OPTIONS] = {};
        };

        struct RHICreateInfo {
            const char* appName = "";
            const char* engineName = "";
            u32 appVersion = {};
            u32 engineVersion = {};
            RHIOptionData options[PYRO_RHI_MAX_OPTIONS] = {};
            const ILogStream* pLoggerSink = nullptr;
        };

        struct RHIContextApiInfo {
            RHIContext* loadedContext = nullptr;
            void* userData = nullptr;
        };
    }
}