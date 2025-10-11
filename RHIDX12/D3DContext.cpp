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

#include "D3DContext.hpp"
#include <EASTL/vector.h>
#include <PyroCommon/Logger.hpp>
#include <PyroRHI/Shader/IShaderFeatureSet.hpp>
#include <RHIDX12/Api/Device.hpp>
#include <iostream>
#include <wrl.h>

PFN_BeginEventOnCommandList gPixBeginEventOnCommandListFn = nullptr;
PFN_EndEventOnCommandList gPixEndEventOnCommandListFn = nullptr;
PFN_SetMarkerOnCommandList gPixSetMarkerOnCommandListFn = nullptr;

namespace PyroshockStudios::RHIDX12 {
    D3DContext* gDx12Context = nullptr;
    using namespace ::Microsoft::WRL;
    D3DContext::D3DContext(const D3DContextArgs& args, ILogStream* logSink, ILogStream* debugSink) : mDebugSink(debugSink) {
        gDx12Context = this;
        D3DContext::InjectLogger(logSink);
        mPixRuntimeDll = LoadLibraryA("WinPixEventRuntime.dll");
        if (mPixRuntimeDll) {
            gPixBeginEventOnCommandListFn = (PFN_BeginEventOnCommandList)GetProcAddress(mPixRuntimeDll, "PIXBeginEventOnCommandList");
            gPixEndEventOnCommandListFn = (PFN_EndEventOnCommandList)GetProcAddress(mPixRuntimeDll, "PIXEndEventOnCommandList");
            gPixSetMarkerOnCommandListFn = (PFN_SetMarkerOnCommandList)GetProcAddress(mPixRuntimeDll, "PIXSetMarkerOnCommandList");

            Logger::Info(gDX12Sink, "Found PIX Debugger");
        }

        UINT dxgiFactoryFlags = 0;
        if (args.bDebug) {
            ComPtr<ID3D12Debug> debugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
                debugController->EnableDebugLayer();

                // Enable additional debug layers.
                dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
            }
        }

        ComPtr<IDXGIFactory4> factory;
        CheckD3DResult(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));
        D3DSetDebugName(factory, "DXGI Factory 4");

        ComPtr<IDXGIAdapter1> adapter;

        if (args.bWarpDriver) {
            factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
            if (!adapter) {
                Logger::Fatal(gDX12Sink, "Failed to get the DX12 WARP device!");
                return;
            }
            D3DSetDebugName(adapter, "Warp Adaptor");
        } else {
            GetHardwareAdapter(factory.Get(), &adapter);
            D3DSetDebugName(adapter, "Hardware Adaptor");
        }

        ComPtr<ID3D12Device> device;
        CheckD3DResult(D3D12CreateDevice(
            adapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&device)));
        D3DSetDebugName(device, "D3D12 Device (Feature level 11_0)");

        if (args.bDebug) {
            if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&mInfoQueue)))) {
                D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO, D3D12_MESSAGE_SEVERITY_WARNING };
                // Suprress the following
                D3D12_MESSAGE_ID denyIds[] = {
                    // D3D12 WARNING: ID3D12CommandList::ClearRenderTargetView: The clear values do not match those passed to resource creation.
                    D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
                    // D3D12 WARNING: ID3D12CommandList::ClearDepthStencilView: The clear values do not match those passed to resource creation.
                    D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,
                };

                D3D12_INFO_QUEUE_FILTER filter = {};
                filter.DenyList.NumIDs = _countof(denyIds);
                filter.DenyList.pIDList = denyIds;

                mInfoQueue->AddStorageFilterEntries(&filter);
            }
        }
        mDevice = new D3DDevice(eastl::move(device), eastl::move(factory), eastl::move(adapter));
    }
    D3DContext::~D3DContext() {
        if (mPixRuntimeDll) {
            FreeLibrary(mPixRuntimeDll);
        }
        delete mDevice;
        gDx12Context = nullptr;
    }

    void D3DContext::GetHardwareAdapter(
        IDXGIFactory1* pFactory,
        IDXGIAdapter1** ppAdapter,
        i32 deviceIndex) {
        *ppAdapter = nullptr;

        ComPtr<IDXGIAdapter1> adapter;

        ComPtr<IDXGIFactory6> factory6;
        if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6)))) {
            for (
                UINT adapterIndex = 0;
                SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                    adapterIndex,
                    deviceIndex == -1 ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                    IID_PPV_ARGS(&adapter)));
                ++adapterIndex) {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                    continue;
                }
                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
                    break;
                }
            }
        }

        if (adapter.Get() == nullptr) {
            for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex) {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
                    // force WARP selection
                    continue;
                }
                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
                    break;
                }
            }
        }

        *ppAdapter = adapter.Detach();
        if (*ppAdapter == nullptr) {
            Logger::Fatal(gDX12Sink, "Failed to pick a suitable DX12 device!");
        }
    }
    IDevice* D3DContext::CreateDevice() {
        return mDevice;
    }

    const RHIProperties& D3DContext::Properties() {
        const static RHIProperties set{
            .bBufferDeviceAddress = false,
            .bDrawIndirectCount = false,
            .bUint8IndexBuffer = false,
            .bTesselationShader = true,
            .bGeometryShader = true,
            .bBCnTextureCompression = true,
            .viewportConvention = RHIViewportConvention::LeftHanded_OriginBottomLeft,
        };
        return set;
    }

    IShaderFeatureSet* D3DContext::ShaderFeatureSet() {
        struct ShaderFeatureSetD3D12 : public IShaderFeatureSet {
            ShaderFeatureSetD3D12() = default;
            ShaderCompileTarget GetTarget() const override {
                return ShaderCompileTarget::Dxbc;
            }
            const char* GetProfileName(ShaderStage shaderStage) const override {
                return "sm5_1";
            }
            const char* GetFileExtension() const override {
                return "cso";
            }
            const ShaderFeatureInfo& Features() const override {
                static auto features = ShaderFeatureInfo{
                    .bDescriptorIndexing = true,
                    .bBufferDeviceAddress = false,
                    .bScalarLayout = true,
                    .bDrawParameters = false,
                    .bGLSL = false,
                };
                return features;
            }
            const eastl::span<eastl::pair<const char*, const char*>>& GlobalPreprocessorDefines() const override {
                static eastl::vector<eastl::pair<const char*, const char*>> preprocesor = {
                    { "PYRO_SHADER_FLAG_RHI_D3D12", "1" }
                };
                static auto span = eastl::span(preprocesor.data(), preprocesor.size());
                return span;
            }
        };
        // for now
        static ShaderFeatureSetD3D12 stub{};
        return &stub;
    }
    void D3DContext::InjectLogger(ILogStream* stream) {
        gDX12Sink = stream;
    }
#undef GetMessage
    void D3DContext::InternalFlushDebugMessages() {
        const UINT64 count = mInfoQueue->GetNumStoredMessagesAllowedByRetrievalFilter();
        for (UINT64 i = 0; i < count; i++) {
            SIZE_T messageLength = 0;
            mInfoQueue->GetMessageA(i, nullptr, &messageLength);

            std::vector<char> messageData(messageLength);
            auto* message = reinterpret_cast<D3D12_MESSAGE*>(messageData.data());
            mInfoQueue->GetMessageA(i, message, &messageLength);
            switch (message->Severity) {
            case D3D12_MESSAGE_SEVERITY_CORRUPTION:
                Logger::Fatal(mDebugSink, message->pDescription);
                break;
            case D3D12_MESSAGE_SEVERITY_ERROR:
                Logger::Error(mDebugSink, message->pDescription);
                break;
            case D3D12_MESSAGE_SEVERITY_WARNING:
                Logger::Warn(mDebugSink, message->pDescription);
                break;
            case D3D12_MESSAGE_SEVERITY_INFO:
                Logger::Info(mDebugSink, message->pDescription);
                break;
            case D3D12_MESSAGE_SEVERITY_MESSAGE:
                Logger::Debug(mDebugSink, message->pDescription);
                break;
            default:
                break;
            }
        }
    }
} // namespace PyroshockStudios::RHIDX12
