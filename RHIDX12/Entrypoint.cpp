#include <RHIDX12/D3DContext.hpp>

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
using namespace PyroshockStudios::RHIDX12;

static constexpr PyroshockStudios::GUID gDX12RhiGuid = "fc8327de-876c-4239-8cc3-c01e03074d80"_guid;

enum RhiOptions : i32 {
    eDebug,
    eMAX_OPTIONS
};

static const RHIOptionInfo gRhiOptions[PYRO_RHI_MAX_OPTIONS] = {
    { .name = "debug", .valueType = RHIOptionValueType::Flag }
};

PYRO_EXPORT void PYRO_CDECL GetCustomRHIInfo(RHIInfo* pInfo) {
    pInfo->guid = gDX12RhiGuid;
    strncpy(pInfo->author, "V.O.F. Pyroshock Studios", PYRO_RHI_NAME_MAX_LENGTH);
    strncpy(pInfo->name, "DirectX 12", PYRO_RHI_AUTH_MAX_LENGTH);
    strncpy(pInfo->shorthand, "dx12", PYRO_RHI_SHORTHAND_MAX_LENGTH);

    for (i32 i = 0; i < eMAX_OPTIONS; ++i) {
        pInfo->availableOptions[i] = gRhiOptions[i];
    }
}

PYRO_EXPORT void PYRO_CDECL CreateRHIContext(const RHICreateInfo* pCreateInfo, RHIContextApiInfo* pApi) {
    D3DContextArgs contextArgs{
        //.appName = pCreateInfo->appName,
        //.engineName = pCreateInfo->engineName,
        //.appVersion = pCreateInfo->appVersion,
        //.engineVersion = pCreateInfo->engineVersion,
        //.preferredPhysicalDevice = -1,
    };

    for (u32 i = 0; i < PYRO_RHI_MAX_OPTIONS; ++i) {
        const auto& option = pCreateInfo->options[i];
        if (option.optionIndex == -1)
            break;

        switch (option.optionIndex) {
        case eDebug:
            Logger::Trace("RHI load option: Enabled debug layers");
            contextArgs.bDebug = true;
            break;
        default:
            Logger::Trace("Invalid RHI load option ignored!");
            break;
        }
    }

    pApi->loadedContext = new D3DContext(contextArgs);
}
PYRO_EXPORT void PYRO_CDECL DestroyRHIContext(RHIContextApiInfo* pApi) {
    D3DContext* ctx = static_cast<D3DContext*>(pApi->loadedContext);
    ASSERT(ctx != nullptr, "Cannot destroy a null context!");
    delete ctx;
}
