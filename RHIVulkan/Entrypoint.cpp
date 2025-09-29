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

#include <RHIVulkan/VkContext.hpp>

#include <PyroCommon/Core.hpp>
#include <PyroCommon/GUID.hpp>
#include <PyroCommon/Logger.hpp>
#include <PyroRHI/Context.hpp>
#include <PyroRHI/Exports.hpp>
#include <PyroRHI/Info.hpp>


#include <cstring>
#include <libassert/assert.hpp>

using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::RHIVulkan;

static constexpr PyroshockStudios::GUID gVulkanRhiGuid = "303e392f-9d80-4e96-88ca-57f4f760dee5"_guid;

enum RhiOptionIndices {
    eValidation = 0,
    eDevice = 1,
    eMAX_OPTIONS
};
static const RHIOptionInfo gRhiOptions[PYRO_RHI_MAX_OPTIONS] = {
    { .name = "debug" },
    { .name = "device", .valueType = RHIOptionValueType::Integer }
};

PYRO_EXPORT void PYRO_CDECL GetCustomRHIInfo(RHIInfo* pInfo) {
    pInfo->guid = gVulkanRhiGuid;
    strncpy(pInfo->author, "V.O.F. Pyroshock Studios", PYRO_RHI_NAME_MAX_LENGTH);
    strncpy(pInfo->name, "Vulkan 1.3", PYRO_RHI_AUTH_MAX_LENGTH);
    strncpy(pInfo->shorthand, "vk13", PYRO_RHI_SHORTHAND_MAX_LENGTH);

    for (i32 i = 0; i < eMAX_OPTIONS; ++i) {
        pInfo->availableOptions[i] = gRhiOptions[i];
    }
}
PYRO_EXPORT void PYRO_CDECL CreateRHIContext(const RHICreateInfo* pCreateInfo, RHIContextApiInfo* pApi) {
    VulkanContextArgs contextArgs{
        .appName = pCreateInfo->appName,
        .engineName = pCreateInfo->engineName,
        .appVersion = pCreateInfo->appVersion,
        .engineVersion = pCreateInfo->engineVersion,
        .preferredPhysicalDevice = -1,
    };

    for (u32 i = 0; i < PYRO_RHI_MAX_OPTIONS; ++i) {
        const auto& option = pCreateInfo->options[i];
        if (option.optionIndex == -1)
            break;

        switch (option.optionIndex) {
        case eValidation:
            Logger::Trace(pCreateInfo->pLoggerSink, "RHI load option: Enabled validation layers");
            contextArgs.bEnableValidation = true;
            break;
        case eDevice:
            Logger::Trace(pCreateInfo->pLoggerSink, "RHI load option: Chose physical device index {}", option.intValue);
            contextArgs.preferredPhysicalDevice = option.intValue;
            break;
        default:
            Logger::Warn(pCreateInfo->pLoggerSink, "Invalid RHI load option ignored");
            break;
        }
    }

    pApi->loadedContext = new VulkanContext(contextArgs, pCreateInfo->pLoggerSink);
}
PYRO_EXPORT void PYRO_CDECL DestroyRHIContext(RHIContextApiInfo* pApi) {
    VulkanContext* ctx = static_cast<VulkanContext*>(pApi->loadedContext);
    ASSERT(ctx != nullptr, "Cannot destroy a null context!");
    delete ctx;
}
