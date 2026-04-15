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
#include <RHIDX12/Api/Device.hpp>
#include <comdef.h>
#include <iostream>
#include <libassert/assert.hpp>
#include <wrl.h>

PFN_BeginEventOnCommandList gPixBeginEventOnCommandListFn = nullptr;
PFN_EndEventOnCommandList gPixEndEventOnCommandListFn = nullptr;
PFN_SetMarkerOnCommandList gPixSetMarkerOnCommandListFn = nullptr;

eastl::string GetCurrentDllDirectory() {
    HMODULE hModule = NULL;
    // Get a handle to *this* DLL
    GetModuleHandleExA(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCSTR)&GetCurrentDllDirectory, // Address of a function in this DLL
        &hModule);

    char path[MAX_PATH];
    GetModuleFileNameA(hModule, path, MAX_PATH);

    eastl::string dllPath = path;
    size_t lastSlash = dllPath.find_last_of("\\/");
    if (lastSlash != eastl::string::npos) {
        return dllPath.substr(0, lastSlash);
    }
    return "";
}

namespace PyroshockStudios::RHIDX12 {
    D3DContext* gDx12Context = nullptr;
    using namespace ::Microsoft::WRL;
    D3DContext::D3DContext(const D3DContextArgs& args, ILogStream* logSink, ILogStream* debugSink) : mDebugSink(debugSink),
                                                                                                     bWarpDriver(args.bWarpDriver),
                                                                                                     bDebug(args.bDebug),
                                                                                                     mOverrideDeviceIndex(args.deviceIndex) {
        gDx12Context = this;
        D3DContext::InjectLogger(logSink);

        ID3D12SDKConfiguration1* cfg = nullptr;
        ID3D12DeviceFactory* pDeviceFactory = nullptr;

        HRESULT result = D3D12GetInterface(CLSID_D3D12SDKConfiguration, IID_PPV_ARGS(&cfg));
        if (FAILED(result)) {
            Logger::Warn(gDX12Sink, "Failed to get D3D12 SDK Configuration interface!"
                                    " Are your drivers and system up to date? Error: {0} ({1}). Skipping independent devices, make sure your executable can find the agility SDK!",
                _com_error(result).ErrorMessage(), (long)result);
        } else {
            result = cfg->CreateDeviceFactory(args.sdkVersion, (GetCurrentDllDirectory() + args.sdkDllRelativePath).c_str(), IID_PPV_ARGS(&pDeviceFactory));
            if (FAILED(result)) {
                Logger::Fatal(gDX12Sink, "Failed to create D3D12 Device Factory!"
                                         " Are the D3D12 SDK libraries under the D3D12 directory? Error: {0} ({1})",
                    _com_error(result).ErrorMessage(), (long)result);
                return;
            }
        }
        mPixRuntimeDll = LoadLibraryA("WinPixEventRuntime.dll");
        if (mPixRuntimeDll) {
            gPixBeginEventOnCommandListFn = (PFN_BeginEventOnCommandList)GetProcAddress(mPixRuntimeDll, "PIXBeginEventOnCommandList");
            gPixEndEventOnCommandListFn = (PFN_EndEventOnCommandList)GetProcAddress(mPixRuntimeDll, "PIXEndEventOnCommandList");
            gPixSetMarkerOnCommandListFn = (PFN_SetMarkerOnCommandList)GetProcAddress(mPixRuntimeDll, "PIXSetMarkerOnCommandList");

            Logger::Info(gDX12Sink, "Found PIX Debugger");
        }

        UINT dxgiFactoryFlags = 0;
        if (args.bDebug) {
            Logger::Trace(gDX12Sink, "Requesting Debug Layer");
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&mDebugController)))) {
                mDebugController->EnableDebugLayer();

                ID3D12Debug1* dbg1;
                if (SUCCEEDED(mDebugController->QueryInterface(IID_PPV_ARGS(&dbg1)))) {
                    // dbg1->SetEnableGPUBasedValidation(TRUE);
                    dbg1->SetEnableSynchronizedCommandQueueValidation(TRUE);
                }

                // Enable additional debug layers.
                dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
                Logger::Info(gDX12Sink, "Enabled Debug Layer");
            } else {
                Logger::Warn(gDX12Sink, "Failed to enable Debug Layer, skipping ID3D12Debug...");
            }
        }

        CheckD3DResult(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&mFactory)), "Failed to CreateDXGIFactory2!");
        D3DSetDebugName(mFactory, "DXGI Factory 4");
    }
    D3DContext::~D3DContext() {
        for (auto& info : mPhysicalDevices) {
            delete info.driverVersion;
            delete info.deviceName;
            // info.vendorName is a string literal!
        }
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
            while (!adapter) {
                for (
                    UINT adapterIndex = 0;
                    SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                        adapterIndex,
                        deviceIndex == -1 ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                        IID_PPV_ARGS(&adapter)));
                    ++adapterIndex) {
                    if (deviceIndex != -1 && deviceIndex != adapterIndex)
                        continue;

                    DXGI_ADAPTER_DESC1 desc;
                    adapter->GetDesc1(&desc);

                    // Check to see whether the adapter supports Direct3D 12, but don't create the
                    // actual device yet.
                    if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
                        break;
                    }
                }
                if (!adapter) {
                    deviceIndex = -1;
                }
            }
        }

        if (adapter.Get() == nullptr) {
            for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex) {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

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

    eastl::span<RHIPhysicalDeviceInfo> D3DContext::QueryPhysicalDevices() {
        if (!mPhysicalDevices.empty()) {
            return mPhysicalDevices;
        }

        if (!mFactory) {
            return {};
        }

        ComPtr<IDXGIAdapter1> pAdapter;
        for (UINT adapterIndex = 0;
            mFactory->EnumAdapters1(adapterIndex, &pAdapter) != DXGI_ERROR_NOT_FOUND;
            ++adapterIndex) {
            DXGI_ADAPTER_DESC1 desc;
            if (FAILED(pAdapter->GetDesc1(&desc))) {
                continue;
            }
            RHIPhysicalDeviceInfo info = {};

            int bufferSize = WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, nullptr, 0, nullptr, nullptr);
            char* deviceNameStr = new char[bufferSize];
            WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, deviceNameStr, bufferSize, nullptr, nullptr);
            info.deviceName = deviceNameStr;

            switch (desc.VendorId) {
            case 0x10DE:
                info.vendorName = "NVIDIA";
                break;
            case 0x1002:
                info.vendorName = "AMD";
                break;
            case 0x8086:
                info.vendorName = "Intel";
                break;
            case 0x13B5:
                info.vendorName = "ARM";
                break;
            case 0x5143:
                info.vendorName = "Qualcomm";
                break;
            case 0x1414:
                info.vendorName = "Microsoft";
                break;
            default:
                info.vendorName = "Unknown";
                break;
            }
            info.deviceID = desc.DeviceId;
            info.vendorID = desc.VendorId;
            LARGE_INTEGER umdVersion = {};
            HRESULT hr = pAdapter->CheckInterfaceSupport(__uuidof(IDXGIDevice), &umdVersion);
            if (SUCCEEDED(hr)) {
                u64 q = static_cast<u64>(umdVersion.QuadPart);
                unsigned major = static_cast<unsigned>((q >> 48) & 0xFFFF);
                unsigned minor = static_cast<unsigned>((q >> 32) & 0xFFFF);
                unsigned sub = static_cast<unsigned>((q >> 16) & 0xFFFF);
                unsigned build = static_cast<unsigned>(q & 0xFFFF);

                char* driverVersionStr = new char[32];
                snprintf(driverVersionStr, 32, "%u.%u.%u.%u", major, minor, sub, build);
                info.driverVersion = driverVersionStr;
            }
            info.deviceLUID = eastl::bit_cast<u64>(desc.AdapterLuid);
            mPhysicalDevices.push_back(info);
        }

        return mPhysicalDevices;
    }

    IDevice* D3DContext::CreateDevice(const RHIDeviceCreateInfo& createInfo) {
        ComPtr<IDXGIAdapter1> adapter;

        if (bWarpDriver) {
            mFactory->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
            if (!adapter) {
                Logger::Fatal(gDX12Sink, "Failed to get the DX12 WARP device!");
                return nullptr;
            }
            D3DSetDebugName(adapter, "Warp Adaptor");
        } else {
            GetHardwareAdapter(mFactory.Get(), &adapter, mOverrideDeviceIndex == -1 ? createInfo.deviceIndex : mOverrideDeviceIndex);
            D3DSetDebugName(adapter, "Hardware Adaptor");
        }

        ComPtr<ID3D12Device> device;
        CheckD3DResult(D3D12CreateDevice(
                           adapter.Get(),
                           D3D_FEATURE_LEVEL_11_0,
                           IID_PPV_ARGS(&device)),
            "Failed to create D3D12 Device with D3D_FEATURE_LEVEL = 11_0");
        D3DSetDebugName(device, "D3D12 Device (Feature level 11_0)");


        if (bDebug) {
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

                mInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
                mInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
                // pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
            }
        }
        ComPtr<IDXGIFactory4> factoryCopy = mFactory;
        mDevice = new D3DDevice(eastl::move(device), eastl::move(factoryCopy), eastl::move(adapter));
        mDevice->SetShaderModel(0x60);
        return mDevice;
    }

    const RHIProperties& D3DContext::Properties() const {
        const static RHIProperties set{
            .bBufferDeviceAddress = false,
            .bDrawIndirectCount = false,
            .bUint8IndexBuffer = false,
            .bTesselationShader = true,
            .bGeometryShader = true,
            .bBCnTextureCompression = true,
            .bEnhancedUndefinedLayoutTransitions = false,
            .viewportConvention = RHIViewportConvention::LeftHanded_OriginBottomLeft,
        };
        return set;
    }

    const IShaderFeatureSet* D3DContext::ShaderFeatureSet() const {
        return this;
    }

    ShaderCompileTarget D3DContext::GetTarget() const {
        return mDevice->mActiveShaderModel == 0x51 ? ShaderCompileTarget::Dxbc : ShaderCompileTarget::Dxil;
    }

    const char* D3DContext::GetProfileName(ShaderStage shaderStage) const {
        u32 sm = mDevice->mActiveShaderModel;
        u32 maj = (sm & 0xF0) >> 4;
        u32 min = (sm & 0x0F) >> 0;

        ASSERT(maj <= 9 && min <= 9, "Badly formatted version!");
        ASSERT(sm <= 0x70, "Application bug! If this is a legitimate shader model, the application has not added support yet!");
        switch (sm) {
        case 0x51:
            return "sm_5_1";
        case 0x60:
            return "sm_6_0";
        case 0x61:
            return "sm_6_1";
        case 0x62:
            return "sm_6_2";
        case 0x63:
            return "sm_6_3";
        case 0x64:
            return "sm_6_4";
        case 0x65:
            return "sm_6_5";
        case 0x66:
            return "sm_6_6";
        case 0x67:
            return "sm_6_7";
        case 0x68:
            return "sm_6_8";
        case 0x69:
            return "sm_6_9";
        case 0x70:
            return "sm_7_0";
        default:
            return nullptr;
        }
        return nullptr;
    }

    const char* D3DContext::GetFileExtension() const {
        return mDevice->mActiveShaderModel == 0x51 ? "cso" : "dxil";
    }

    const ShaderFeatureInfo& D3DContext::Features() const {
        static auto features = ShaderFeatureInfo{
            .bDescriptorIndexing = true,
            .bBufferDeviceAddress = false,
            .bDrawParameters = false,
            .bGLSL = false,
        };
        return features;
    }

    u32 D3DContext::GetMinimumShaderModelFeatureTier(ShaderModelFeatureFlags shaderModelFeatures) const {
        u32 minVersion = 0x51;

        if (shaderModelFeatures & ShaderModelFeatureBits::CONSERVATIVE_RASTERIZATION) {
            minVersion = std::max(minVersion, 0x60u);
        }
        if (shaderModelFeatures & ShaderModelFeatureBits::MESH_SHADER) {
            minVersion = std::max(minVersion, 0x65u);
        }
        if (shaderModelFeatures & ShaderModelFeatureBits::RAY_QUERY) {
            minVersion = std::max(minVersion, 0x65u);
        }
        if (shaderModelFeatures & ShaderModelFeatureBits::RAY_TRACING) {
            minVersion = std::max(minVersion, 0x63u);
        }
        if (shaderModelFeatures & ShaderModelFeatureBits::SHADER_FLOAT16) {
            minVersion = std::max(minVersion, 0x60u);
        }
        if (shaderModelFeatures & ShaderModelFeatureBits::SHADER_INT64) {
            minVersion = std::max(minVersion, 0x60u);
        }
        if (shaderModelFeatures & ShaderModelFeatureBits::SHADER_INT64_ATOMICS) {
            minVersion = std::max(minVersion, 0x66u);
        }
        if (shaderModelFeatures & ShaderModelFeatureBits::SUBGROUP_OPERATIONS) {
            minVersion = std::max(minVersion, 0x60u);
        }
        if (shaderModelFeatures & ShaderModelFeatureBits::VARIABLE_RATE_SHADING) {
            minVersion = std::max(minVersion, 0x64u);
        }

        return minVersion;
    }

    const eastl::span<eastl::pair<const char*, const char*>>& D3DContext::GlobalPreprocessorDefines() const {
        static eastl::vector<eastl::pair<const char*, const char*>> preprocesor = {
            { "PYRO_SHADER_FLAG_RHI_D3D12", "1" }
        };
        static auto span = eastl::span(preprocesor.data(), preprocesor.size());
        return span;
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

            bool bDred = message->Severity == D3D12_MESSAGE_SEVERITY_CORRUPTION || message->Severity == D3D12_MESSAGE_SEVERITY_ERROR;
            if (bDred) {
                ComPtr<ID3D12DeviceRemovedExtendedData> pDred;
                mDevice->InternalDevice()->QueryInterface(IID_PPV_ARGS(&pDred));
                D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT DredAutoBreadcrumbsOutput;
                pDred->GetAutoBreadcrumbsOutput(&DredAutoBreadcrumbsOutput);
            }

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
