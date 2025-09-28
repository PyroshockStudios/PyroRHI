#pragma once
#include "Core.hpp"
#include <PyroCommon/Version.hpp>
#include <PyroRHI/Context.hpp>

namespace PyroshockStudios {
    namespace RHIDX12 {
        class D3DDevice;
        struct D3DContextArgs {
            bool bDebug = false;
        };
        class D3DContext : public RHIContext {
        public:
            D3DContext(const D3DContextArgs& args);
            ~D3DContext();
            void GetHardwareAdapter(
                IDXGIFactory1* pFactory,
                IDXGIAdapter1** ppAdapter,
                i32 deviceIndex = -1);
            IDevice* CreateDevice() override;
            const RHIProperties& Properties() override;
            IShaderFeatureSet* ShaderFeatureSet() override;

        private:
            D3DDevice* mDevice = nullptr;
            HMODULE mPixRuntimeDll = {};
        };
    } // namespace RHIDX12
} // namespace PyroshockStudios