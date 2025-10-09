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
#include "Core.hpp"
#include <PyroCommon/Version.hpp>
#include <PyroRHI/Context.hpp>

namespace PyroshockStudios {
    namespace RHIDX12 {
        class D3DContext;
        extern D3DContext* gDx12Context;
        class D3DDevice;
        struct D3DContextArgs {
            bool bDebug = false;
            bool bWarpDriver = false;
        };
        class D3DContext : public RHIContext {
        public:
            D3DContext(const D3DContextArgs& args, ILogStream* logSink, ILogStream* debugSink);
            ~D3DContext();
            void GetHardwareAdapter(
                IDXGIFactory1* pFactory,
                IDXGIAdapter1** ppAdapter,
                bool bWarp,
                i32 deviceIndex = -1);
            IDevice* CreateDevice() override;
            const RHIProperties& Properties() override;
            IShaderFeatureSet* ShaderFeatureSet() override;

            void InjectLogger( ILogStream* stream) override;

            PYRO_FORCEINLINE void FlushDebugMessages() {
                if (!mInfoQueue) return;

                InternalFlushDebugMessages();
            }
        private:
            void InternalFlushDebugMessages();

            ComPtr<ID3D12InfoQueue> mInfoQueue =nullptr;
            D3DDevice* mDevice = nullptr;
            HMODULE mPixRuntimeDll = {};
            ILogStream* mDebugSink = nullptr;
        };
    }
} // namespace PyroshockStudios