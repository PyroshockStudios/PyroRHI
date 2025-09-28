#pragma once
#include <PyroCommon/Core.hpp>
#include <PyroRHI/Info.hpp>

namespace PyroshockStudios {
    inline namespace RHI {
        typedef void(PYRO_CDECL* PFN_GetCustomRHIInfo)(RHIInfo* pInfo);
        typedef void(PYRO_CDECL* PFN_CreateRHIContext)(const RHICreateInfo* pCreateInfo, RHIContextApiInfo* pApi);
        typedef void(PYRO_CDECL* PFN_DestroyRHIContext)(RHIContextApiInfo* pApi);
    }
}