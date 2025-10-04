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

#include "Device.hpp"
#include "CommandBuffer.hpp"
#include "CommandQueue.hpp"
#include "GPUResource.hpp"
#include "Pipeline.hpp"
#include "QueryPool.hpp"
#include "RenderTarget.hpp"
#include "SwapChain.hpp"
#include "Sync.hpp"
#include <PyroCommon/Logger.hpp>
#include <RHIDX12/InternalShaders.hpp>

#include <libassert/assert.hpp>


namespace PyroshockStudios {
    namespace RHIDX12 {
        constexpr UINT NUM_SAMPLERS = 2048;
        constexpr UINT NUM_CRV_SRV_UAV = 32768;
        constexpr UINT64 MAX_FRAMES_UAV_TABLE_CACHE_UNUSED_LIFETIME = 60;
        constexpr UINT64 MAX_FRAMES_LINEAR_UPLOAD_BUFFER_UNSUED_LIFETIME = 100;

        D3DDevice::D3DDevice(ComPtr<ID3D12Device>&& device, ComPtr<IDXGIFactory4>&& factory, ComPtr<IDXGIAdapter1>&& adapter)
            : mDevice(eastl::move(device)), mFactory(eastl::move(factory)), mAdapter(eastl::move(adapter)) {
            {
                D3D12_FEATURE_DATA_D3D12_OPTIONS options{};
                mDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options));
                if (options.ResourceBindingTier < 2) {
                    Logger::Fatal(gDX12Sink, "Insufficient resource binding tier! Minimum is resource binding tier 2!");
                }
            }
            {
                D3D12_FEATURE_DATA_SHADER_MODEL shaderModel{};
                mDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel));
            }
            mProperties.bufferImageRowAlignment = D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
            {
                // FIXME better querying?
                D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msaaLevels{};
                msaaLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                // msaaLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_TILED_RESOURCE;

                for (UINT s = 64U; s > 1U; s >>= 1U) {
                    msaaLevels.SampleCount = s;
                    mDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msaaLevels, sizeof(msaaLevels));
                    if (msaaLevels.NumQualityLevels > 0) {
                        mProperties.maxRenderTargetSamples = static_cast<RasterizationSamples>(s);
                        mProperties.maxShaderResourceImageSamples = mProperties.maxRenderTargetSamples;
                        break;
                    }
                }
            }


            D3D12_COMMAND_QUEUE_DESC queueDesc = {};
            queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            queueDesc.Priority = 0;
            ComPtr<ID3D12CommandQueue> commandQueue;
            CheckD3DResult(mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));

            mCommandQueue = new D3DCommandQueue(
                {
                    .flags = CommandQueueFlagBits::GRAPHICS | CommandQueueFlagBits::COMPUTE | CommandQueueFlagBits::TRANSFER,
                    .bPresentable = true,
                    .name = "Direct Queue",
                },
                eastl::move(commandQueue));
            mCommandQueueList = { static_cast<ICommandQueue*>(mCommandQueue) };

            mResourcePool = eastl::make_unique<GPUResourcePool>(this, 2048, 2048, NUM_CRV_SRV_UAV, NUM_CRV_SRV_UAV, NUM_SAMPLERS);

            // We need this device visible heap for SRVs and UAVs because of the binding model we are using.
            {
                D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
                heapDesc.NumDescriptors = NUM_CRV_SRV_UAV + Limits::MAX_UNORDERED_ACCESS_VIEW_SLOTS;
                heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

                CheckD3DResult(mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mDefaultUAVDescriptorTable.mHeap)));
                mDefaultUAVDescriptorTable.cpuDescriptor = mDefaultUAVDescriptorTable.mHeap->GetCPUDescriptorHandleForHeapStart();
                mDefaultUAVDescriptorTable.gpuDescriptor = mDefaultUAVDescriptorTable.mHeap->GetGPUDescriptorHandleForHeapStart();
                D3DSetDebugName(mDefaultUAVDescriptorTable.mHeap, "Default SRV-UAV Descriptor Heap");
            }
            // create global root signature
            {
                CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
                CD3DX12_ROOT_PARAMETER rootparams[18]{};
                // push constants
                // 32 * 1 = 32 DWORDS
                rootparams[0].InitAsConstants(32, 13);
                // "specialisation constants"
                // because dx12 doesn't support them, we have to emulate them through these views
                // 5 * 2 = 10 DWORDS
                rootparams[1].InitAsConstantBufferView(8);
                rootparams[2].InitAsConstantBufferView(9);
                rootparams[3].InitAsConstantBufferView(10);
                rootparams[4].InitAsConstantBufferView(11);
                rootparams[5].InitAsConstantBufferView(12);
                // finally, our uniform buffers
                // 8 * 2 = 16 DWORDS
                rootparams[6].InitAsConstantBufferView(0);
                rootparams[7].InitAsConstantBufferView(1);
                rootparams[8].InitAsConstantBufferView(2);
                rootparams[9].InitAsConstantBufferView(3);
                rootparams[10].InitAsConstantBufferView(4);
                rootparams[11].InitAsConstantBufferView(5);
                rootparams[12].InitAsConstantBufferView(6);
                rootparams[13].InitAsConstantBufferView(7);

                CD3DX12_DESCRIPTOR_RANGE srvDescriptorRange{};
                CD3DX12_DESCRIPTOR_RANGE samplerDescriptorRange{};
                CD3DX12_DESCRIPTOR_RANGE uavDescriptorRange{};
                srvDescriptorRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, NUM_CRV_SRV_UAV, 0, 1);
                samplerDescriptorRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, NUM_SAMPLERS, 0, 1);
                // Offset is NUM_CRV_SRV_UAV because we are copying it to end of an SRV heap.
                uavDescriptorRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, Limits::MAX_UNORDERED_ACCESS_VIEW_SLOTS, 0, 1, NUM_CRV_SRV_UAV);

                // another 3 * 1 DWORDS
                rootparams[14].InitAsDescriptorTable(1, &srvDescriptorRange);
                rootparams[15].InitAsDescriptorTable(1, &samplerDescriptorRange);
                rootparams[16].InitAsDescriptorTable(1, &uavDescriptorRange);

                // DrawID constant
                // 3 * 1 = 3 DWORDS
                // SV_DrawIndex
                rootparams[17].InitAsConstants(1, 0, 2, D3D12_SHADER_VISIBILITY_VERTEX);

                // TOTAL = 62 out of 64 DWORDS

                const D3D12_ROOT_SIGNATURE_FLAGS flags =
                    D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
                rootSignatureDesc.Init(PYRO_ARRAY_SIZE(rootparams), rootparams, 0, nullptr, flags);
                ComPtr<ID3DBlob> signature;
                ComPtr<ID3DBlob> error;
                HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, &error);
                if (error && error->GetBufferPointer()) {
                    OutputDebugStringA((const char*)error->GetBufferPointer());
                }
                CheckD3DResult(hr);
                CheckD3DResult(mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));
                D3DSetDebugName(mRootSignature, "Default Root Signature");
            }
            CheckD3DResult(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mDeferredDeleterFence)));
            D3DSetDebugName(mDeferredDeleterFence, "Deferred destroy fence");

            // create blit image root signature
            {
                CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
                CD3DX12_ROOT_PARAMETER rootparams[3]{};
                // Blit coordinates
                rootparams[0].InitAsConstants(4 * sizeof(eastl::array<f32, 2>) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
                // Blit image srv
                CD3DX12_DESCRIPTOR_RANGE srvDescriptorRange{};
                srvDescriptorRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
                rootparams[1].InitAsDescriptorTable(1, &srvDescriptorRange, D3D12_SHADER_VISIBILITY_PIXEL);
                // Blit image sampler
                CD3DX12_DESCRIPTOR_RANGE samplerDescriptorRange{};
                samplerDescriptorRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, 0);
                rootparams[2].InitAsDescriptorTable(1, &samplerDescriptorRange, D3D12_SHADER_VISIBILITY_PIXEL);

                const D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
                rootSignatureDesc.Init(PYRO_ARRAY_SIZE(rootparams), rootparams, 0, nullptr, flags);
                ComPtr<ID3DBlob> signature;
                ComPtr<ID3DBlob> error;
                HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, &error);
                if (error && error->GetBufferPointer()) {
                    OutputDebugStringA((const char*)error->GetBufferPointer());
                }
                CheckD3DResult(hr);
                CheckD3DResult(mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mBlitImageRootSignature)));
                D3DSetDebugName(mBlitImageRootSignature, "Blit Image Root Signature");
            }
            // Finally, create the sampler heap objects
            {
                D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
                heapDesc.NumDescriptors = 1;
                heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
                heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

                CheckD3DResult(mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mNearestSamplerDescriptorTable.mHeap)));
                mNearestSamplerDescriptorTable.cpuDescriptor = mNearestSamplerDescriptorTable.mHeap->GetCPUDescriptorHandleForHeapStart();
                mNearestSamplerDescriptorTable.gpuDescriptor = mNearestSamplerDescriptorTable.mHeap->GetGPUDescriptorHandleForHeapStart();
                D3DSetDebugName(mNearestSamplerDescriptorTable.mHeap, "Blit Image Nearest Sampler Descriptor Heap");

                D3D12_SAMPLER_DESC samplerDesc = {};
                samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
                samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
                samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
                samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
                samplerDesc.MinLOD = 0.0f;
                samplerDesc.MaxLOD = FLT_MAX;
                samplerDesc.MipLODBias = 0.0f;
                samplerDesc.MaxAnisotropy = 0;
                samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;

                mDevice->CreateSampler(&samplerDesc, mNearestSamplerDescriptorTable.cpuDescriptor);

                CheckD3DResult(mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mLinearSamplerDescriptorTable.mHeap)));
                mLinearSamplerDescriptorTable.cpuDescriptor = mLinearSamplerDescriptorTable.mHeap->GetCPUDescriptorHandleForHeapStart();
                mLinearSamplerDescriptorTable.gpuDescriptor = mLinearSamplerDescriptorTable.mHeap->GetGPUDescriptorHandleForHeapStart();
                D3DSetDebugName(mLinearSamplerDescriptorTable.mHeap, "Blit Image Linear Sampler Descriptor Heap");

                samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
                mDevice->CreateSampler(&samplerDesc, mLinearSamplerDescriptorTable.cpuDescriptor);
            }
            {
                D3D12_INDIRECT_ARGUMENT_DESC argDesc;
                D3D12_COMMAND_SIGNATURE_DESC cmdSigDesc = {};
                cmdSigDesc.NodeMask = 0;
                cmdSigDesc.NumArgumentDescs = 1;
                cmdSigDesc.pArgumentDescs = &argDesc;

                argDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
                cmdSigDesc.ByteStride = sizeof(DrawArgumentBuffer);
                CheckD3DResult(mDevice->CreateCommandSignature(&cmdSigDesc, nullptr, IID_PPV_ARGS(&mIndirectDrawSignature)));
                argDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
                cmdSigDesc.ByteStride = sizeof(DrawIndexedArgumentBuffer);
                CheckD3DResult(mDevice->CreateCommandSignature(&cmdSigDesc, nullptr, IID_PPV_ARGS(&mIndirectDrawIndexedSignature)));
                argDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
                cmdSigDesc.ByteStride = sizeof(DispatchArgumentBuffer);
                CheckD3DResult(mDevice->CreateCommandSignature(&cmdSigDesc, nullptr, IID_PPV_ARGS(&mIndirectDispatchSignature)));
            }
            {
                D3D12MA::ALLOCATOR_DESC allocatorDesc{};
                allocatorDesc.pDevice = mDevice.Get();
                allocatorDesc.pAllocationCallbacks = nullptr;
                allocatorDesc.pAdapter = mAdapter.Get();
                CheckD3DResult(D3D12MA::CreateAllocator(&allocatorDesc, mAllocator.GetAddressOf()));
            }
        }
        D3DDevice::~D3DDevice() {
            WaitIdle();
            CollectGarbage();
            ASSERT(mDeferredDeletes.empty(), "Command buffers must finish execution before device destruction! Deferred destruction was leaked!");
            ASSERT(mOccupiedLinearUploadBuffers.empty(), "Command buffers must finish execution before device destruction! Linear upload buffers were leaked!");
            for (auto* cmb : mPooledCommandBuffers) {
                delete cmb;
            }
            for (auto [_, buf] : mAvailableLinearUploadBuffers) {
                delete buf;
            }
            delete mCommandQueue;
        }
        bool D3DDevice::IsMemoryBlockValid(MemoryBlock handle) const {
            return handle != PYRO_NULL_MEMORY_BLOCK;
        }
        bool D3DDevice::IsBufferValid(Buffer handle) const {
            return handle != PYRO_NULL_BUFFER;
        }
        bool D3DDevice::IsImageValid(Image handle) const {
            return handle != PYRO_NULL_IMAGE;
        }
        bool D3DDevice::IsShaderResourceValid(ShaderResourceId id) const {
            return id.index != 0;
        }
        bool D3DDevice::IsUnorderedAccessValid(UnorderedAccessId id) const {
            return id.index != 0;
        }
        bool D3DDevice::IsSamplerValid(SamplerId id) const {
            return id.index != 0;
        }
        const MemoryBlockInfo& D3DDevice::GetMemoryBlockInfo(MemoryBlock memory) const {
            ASSERT(IsValid(memory));
            return mResourcePool->Get(memory).info;
        }
        const BufferInfo& D3DDevice::GetBufferInfo(Buffer buffer) const {
            ASSERT(IsValid(buffer));
            return mResourcePool->Get(buffer).info;
        }
        const ImageInfo& D3DDevice::GetImageInfo(Image image) const {
            ASSERT(IsValid(image));
            return mResourcePool->Get(image).info;
        }
        const GPUResourceInfo& D3DDevice::GetShaderResourceInfo(ShaderResourceId id) const {
            ASSERT(IsValid(id));
            return mResourcePool->mSRVHeap.GetInfo(id);
        }
        const GPUResourceInfo& D3DDevice::GetUnorderedAccessInfo(UnorderedAccessId id) const {
            ASSERT(IsValid(id));
            return mResourcePool->mUAVHeap.GetInfo(id);
        }
        const SamplerInfo& D3DDevice::GetSamplerInfo(SamplerId id) const {
            ASSERT(IsValid(id));
            return mResourcePool->mSamplerHeap.GetInfo(id);
        }
        const RenderTargetInfo& D3DDevice::GetRenderTargetInfo(RenderTarget renderTarget) const {
            return eastl::bit_cast<D3DRenderTarget*>(renderTarget)->Info();
        }
        const RasterPipelineInfo& D3DDevice::GetRasterPipelineInfo(RasterPipeline pipeline) const {
            return eastl::bit_cast<D3DRasterPipeline*>(pipeline)->Info();
        }
        const ComputePipelineInfo& D3DDevice::GetComputePipelineInfo(ComputePipeline pipeline) const {
            return eastl::bit_cast<D3DComputePipeline*>(pipeline)->Info();
        }
        const SemaphoreInfo& D3DDevice::GetSemaphoreInfo(Semaphore semaphore) const {
            return eastl::bit_cast<D3DSemaphore*>(semaphore)->Info();
        }

        DeviceAddress D3DDevice::BufferDeviceAddress(Buffer buffer) const {
            ASSERT(IsValid(buffer));
            return DeviceAddress();
        }
        u8* D3DDevice::BufferHostAddress(Buffer buffer) const {
            ASSERT(IsValid(buffer));
            return mResourcePool->Get(buffer).mappedMemory;
        }
        DeviceSize D3DDevice::ImageSizeRequirements(Image image) const {
            ASSERT(IsValid(image));

            D3D12_RESOURCE_ALLOCATION_INFO resourceAllocInfo = mDevice->GetResourceAllocationInfo(0, 1, &mResourcePool->Get(image).desc);
            return resourceAllocInfo.SizeInBytes;
        }
        u32 D3DDevice::ImageSubresourceRowPitch(Image image, ImageSlice slice, u32 rowWidth) const {
            auto& img = mResourcePool->Get(image);
            UINT dstSubresource = D3D12CalcSubresource(slice.mipLevel, slice.arrayLayer, 0, img.info.mipLevelCount, img.info.arrayLayerCount);
            D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
            UINT numRows = {};
            UINT64 rowSizesInBytes = {};
            UINT64 requiredSize = {};
            mDevice->GetCopyableFootprints(&img.desc, dstSubresource, 1, 0,
                &footprint, &numRows, &rowSizesInBytes, &requiredSize);
            return footprint.Footprint.RowPitch;
        }
        MemoryBlock D3DDevice::CreateMemoryBlock(const MemoryBlockInfo& info) {
            auto [memory, data] = mResourcePool->AllocMemoryBlock();
            data.info = info;
            D3D12_HEAP_TYPE heapType = {};
            switch (info.domain) {
            case MemoryAllocationDomain::DeviceLocal:
                heapType = D3D12_HEAP_TYPE_DEFAULT;
                break;
            case MemoryAllocationDomain::HostStaging:
            case MemoryAllocationDomain::HostRandomWrite:
                heapType = D3D12_HEAP_TYPE_UPLOAD;
                break;
            case MemoryAllocationDomain::HostReadback:
                heapType = D3D12_HEAP_TYPE_READBACK;
                break;
            default:
                ASSERT(false, "Invalid heap type!");
                break;
            }

            D3D12_HEAP_DESC heapDec = {};
            heapDec.SizeInBytes = info.size;
            heapDec.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

            heapDec.Properties = CD3DX12_HEAP_PROPERTIES(heapType);
            if (info.bufferUsage == 0) {
                heapDec.Flags |= D3D12_HEAP_FLAG_DENY_BUFFERS;
            } else {
                heapDec.Flags |= D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
            }
            mDevice->CreateHeap(&heapDec, IID_PPV_ARGS(&data.heap));
            D3DSetDebugName(data.heap, info.name.c_str());
            D3D12MA::VIRTUAL_BLOCK_DESC vblockDesc = {};
            vblockDesc.Size = info.size;
            if (info.strategy == VirtualSuballocationStrategy::AggressiveRing) {
                D3D12MA::VIRTUAL_BLOCK_DESC vblockDesc = {};
                vblockDesc.Flags |= D3D12MA::VIRTUAL_BLOCK_FLAG_ALGORITHM_LINEAR;
            }
            D3D12MA::CreateVirtualBlock(&vblockDesc, data.block.GetAddressOf());
            return memory;
        }
        Buffer D3DDevice::CreateBuffer(const BufferInfo& info) {
            auto [buffer, data] = mResourcePool->AllocBuffer();
            data.lastValidState = ToD3D12BufferResourceState(info.initialLayout);
            data.info = info;

            // Describe the buffer
            D3D12_RESOURCE_DESC bufferDesc = {};
            bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            bufferDesc.Alignment = 0;
            bufferDesc.Width = info.size;
            bufferDesc.Height = 1;
            bufferDesc.DepthOrArraySize = 1;
            bufferDesc.MipLevels = 1;
            bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
            bufferDesc.SampleDesc.Count = 1;
            bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

            if (info.usage & BufferUsageFlagBits::UNORDERED_ACCESS)
                bufferDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

            if ((info.usage & BufferUsageFlagBits::SHADER_RESOURCE) == BufferUsageFlagBits::NONE)
                bufferDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

            // Map MemoryAllocationUsage to heap type & initial state
            D3D12_RESOURCE_STATES state = ToD3D12BufferResourceState(info.initialLayout);

            // Fill buffer data
            data.range.Begin = 0;
            data.range.End = info.size;
            data.desc = bufferDesc;
            bool bMap = false;

            switch (info.memoryBlock == PYRO_NULL_MEMORY_BLOCK ? info.allocationDomain : mResourcePool->Get(info.memoryBlock).info.domain) {
            case MemoryAllocationDomain::DeviceLocal:
                break;
            case MemoryAllocationDomain::HostStaging:
            case MemoryAllocationDomain::HostRandomWrite:
                ASSERT(info.initialLayout == BufferLayout::ReadOnly || info.initialLayout == BufferLayout::TransferSrc);
                state = D3D12_RESOURCE_STATE_GENERIC_READ;
                bMap = true;
                break;
            case MemoryAllocationDomain::HostReadback:
                bMap = true;
                break;
            default:
                ASSERT(false, "Invalid heap type!");
                break;
            }

            if (info.memoryBlock == PYRO_NULL_MEMORY_BLOCK) {
                // Allocate with D3D12MA
                D3D12MA::ALLOCATION_DESC allocDesc = {};
                switch (info.allocationDomain) {
                case MemoryAllocationDomain::DeviceLocal:
                    allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
                    break;
                case MemoryAllocationDomain::HostStaging:
                case MemoryAllocationDomain::HostRandomWrite:
                    allocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;
                    break;
                case MemoryAllocationDomain::HostReadback:
                    allocDesc.HeapType = D3D12_HEAP_TYPE_READBACK;
                    break;
                default:
                    ASSERT(false, "Invalid heap type!");
                    break;
                }

                HRESULT hr = mAllocator->CreateResource(
                    &allocDesc,
                    &bufferDesc,
                    state,
                    nullptr, // optimized clear value (unused for buffers)
                    data.allocation.GetAddressOf(),
                    IID_PPV_ARGS(&data.resource));
                CheckD3DResult(hr);

            } else {
                auto& block = mResourcePool->Get(info.memoryBlock);
                D3D12MA::VIRTUAL_ALLOCATION_DESC allocDesc = {};
                D3D12_RESOURCE_ALLOCATION_INFO resourceAllocInfo = mDevice->GetResourceAllocationInfo(0, 1, &bufferDesc);
                allocDesc.Size = resourceAllocInfo.SizeInBytes;
                allocDesc.Alignment = resourceAllocInfo.Alignment;
                switch (block.info.strategy) {
                case VirtualSuballocationStrategy::AggressiveRing:
                case VirtualSuballocationStrategy::TimeEfficient:
                    allocDesc.Flags |= D3D12MA::VIRTUAL_ALLOCATION_FLAG_STRATEGY_MIN_TIME;
                    break;
                case VirtualSuballocationStrategy::SpaceEfficient:
                    allocDesc.Flags |= D3D12MA::VIRTUAL_ALLOCATION_FLAG_STRATEGY_MIN_MEMORY;
                    break;
                default:
                    break;
                }
                UINT64 offset;
                HRESULT hr = block.block->Allocate(&allocDesc, &data.virtualAlloc, &offset);
                mDevice->CreatePlacedResource(
                    block.heap.Get(), // heap from above
                    offset,           // offset of this virtual slice
                    &bufferDesc,
                    state,
                    nullptr,
                    IID_PPV_ARGS(&data.resource));
                CheckD3DResult(hr);
                ++block.debugRefs;
            }
            D3DSetDebugName(data.resource, info.name.c_str());

            if (bMap) {
                D3D12_RANGE range = { 0, info.size };
                CheckD3DResult(data.resource->Map(0, &range, reinterpret_cast<void**>(&data.mappedMemory)));
            }
            return buffer;
        }
        Image D3DDevice::CreateImage(const ImageInfo& info) {
            auto [image, data] = mResourcePool->AllocImage();
            data.info = info;

            // Track resource states for each subresource
            data.lastValidStates.resize(info.arrayLayerCount * info.mipLevelCount, D3D12_RESOURCE_STATE_COMMON);

            // Describe the texture
            D3D12_RESOURCE_DESC textureDesc = {};
            textureDesc.MipLevels = info.mipLevelCount;
            textureDesc.Format = ToDXGIFormat(info.format);
            textureDesc.Width = static_cast<UINT64>(info.size.x);
            textureDesc.Height = info.size.y;
            textureDesc.DepthOrArraySize = static_cast<UINT16>(eastl::max(info.size.z, info.arrayLayerCount));
            textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

            bool dsv = RHIUtil::FormatIsDepthStencil(info.format);

            if ((info.usage & ImageUsageFlagBits::RENDER_TARGET && !dsv) ||
                (info.usage & ImageUsageFlagBits::BLIT_DST)) {
                textureDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
            }

            if (info.usage & ImageUsageFlagBits::RENDER_TARGET && dsv) {
                textureDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
                if ((info.usage & (ImageUsageFlagBits::SHADER_RESOURCE | ImageUsageFlagBits::BLIT_SRC)) == ImageUsageFlagBits::NONE) {
                    textureDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
                }
            }

            if (info.usage & ImageUsageFlagBits::UNORDERED_ACCESS) {
                textureDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            }

            textureDesc.SampleDesc.Count = info.sampleCount;
            textureDesc.SampleDesc.Quality = 0;

            switch (info.dimensions) {
            case 1:
                textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
                break;
            case 2:
                textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                break;
            case 3:
                textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
                break;
            default:
                ASSERT(false, "Invalid texture dimension");
                break;
            }

            // Determine heap type based on memory allocation domain

            if (info.memoryBlock == PYRO_NULL_MEMORY_BLOCK) {
                D3D12MA::ALLOCATION_DESC allocDesc = {};
                allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

                HRESULT hr = mAllocator->CreateResource(
                    &allocDesc,
                    &textureDesc,
                    D3D12_RESOURCE_STATE_COMMON,
                    nullptr, // optimized clear value if needed
                    data.allocation.GetAddressOf(),
                    IID_PPV_ARGS(&data.resource));
                CheckD3DResult(hr);
            } else {
                auto& block = mResourcePool->Get(info.memoryBlock);
                D3D12MA::VIRTUAL_ALLOCATION_DESC allocDesc = {};
                D3D12_RESOURCE_ALLOCATION_INFO resourceAllocInfo = mDevice->GetResourceAllocationInfo(0, 1, &textureDesc);
                allocDesc.Size = resourceAllocInfo.SizeInBytes;
                allocDesc.Alignment = resourceAllocInfo.Alignment;
                switch (block.info.strategy) {
                case VirtualSuballocationStrategy::AggressiveRing:
                case VirtualSuballocationStrategy::TimeEfficient:
                    allocDesc.Flags |= D3D12MA::VIRTUAL_ALLOCATION_FLAG_STRATEGY_MIN_TIME;
                    break;
                case VirtualSuballocationStrategy::SpaceEfficient:
                    allocDesc.Flags |= D3D12MA::VIRTUAL_ALLOCATION_FLAG_STRATEGY_MIN_MEMORY;
                    break;
                default:
                    break;
                }
                UINT64 offset;
                HRESULT hr = block.block->Allocate(&allocDesc, &data.virtualAlloc, &offset);
                mDevice->CreatePlacedResource(
                    block.heap.Get(), // heap from above
                    offset,           // offset of this virtual slice
                    &textureDesc,
                    D3D12_RESOURCE_STATE_COMMON,
                    nullptr,
                    IID_PPV_ARGS(&data.resource));
                CheckD3DResult(hr);
                ++block.debugRefs;
            }

            data.desc = textureDesc;
            D3DSetDebugName(data.resource, info.name.c_str());

            ImageAddIfNecessaryBlitSupport(data);

            return image;
        }
        ShaderResourceId D3DDevice::CreateShaderResource(const GPUResourceInfo& info) {
            auto [handle, data] = mResourcePool->mSRVHeap.AcquireSlot();
            data = info;

            ID3D12Resource* resource;
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            if (eastl::holds_alternative<BufferResourceInfo>(info)) {
                auto& bufInfo = eastl::get<BufferResourceInfo>(info);
                auto& srcBuf = mResourcePool->Get(bufInfo.buffer);
                resource = srcBuf.resource.Get();

                srvDesc.Format = DXGI_FORMAT_UNKNOWN;
                srvDesc.Buffer.StructureByteStride = 0;
                srvDesc.Buffer.FirstElement = static_cast<UINT>(bufInfo.region.offset);
                srvDesc.Buffer.NumElements = eastl::min(static_cast<UINT>(srcBuf.info.size), static_cast<UINT>(bufInfo.region.size)) / 4;
                srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;

            } else if (eastl::holds_alternative<ImageResourceInfo>(info)) {
                auto& imgInfo = eastl::get<ImageResourceInfo>(info);
                auto& srcImg = mResourcePool->Get(imgInfo.image);
                resource = srcImg.resource.Get();
                bool bMS = srcImg.info.sampleCount > 1;

                srvDesc.Format = ToDXGIFormat(imgInfo.format == Format::Inherit ? srcImg.info.format : imgInfo.format);

                srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

                const auto& slice = imgInfo.slice;
                auto mipLevels = srcImg.info.mipLevelCount;
                auto arrayLayers = srcImg.info.arrayLayerCount;

                // Pick the correct view dimension
                switch (imgInfo.viewType) {
                case ImageViewType::e1D:
                    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
                    srvDesc.Texture1D.MostDetailedMip = slice.baseMipLevel;
                    srvDesc.Texture1D.MipLevels = mipLevels;
                    srvDesc.Texture1D.ResourceMinLODClamp = 0.0f;
                    break;

                case ImageViewType::e1DArray:
                    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                    srvDesc.Texture1DArray.MostDetailedMip = slice.baseMipLevel;
                    srvDesc.Texture1DArray.MipLevels = mipLevels;
                    srvDesc.Texture1DArray.FirstArraySlice = slice.baseArrayLayer;
                    srvDesc.Texture1DArray.ArraySize = arrayLayers;
                    srvDesc.Texture1DArray.ResourceMinLODClamp = 0.0f;
                    break;

                case ImageViewType::e2D:
                    if (bMS) {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
                    } else {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                        srvDesc.Texture2D.MostDetailedMip = slice.baseMipLevel;
                        srvDesc.Texture2D.MipLevels = mipLevels;
                        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
                    }
                    break;

                case ImageViewType::e2DArray:
                    if (bMS) {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
                        srvDesc.Texture2DMSArray.FirstArraySlice = slice.baseArrayLayer;
                        srvDesc.Texture2DMSArray.ArraySize = arrayLayers;
                    } else {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                        srvDesc.Texture2DArray.MostDetailedMip = slice.baseMipLevel;
                        srvDesc.Texture2DArray.MipLevels = mipLevels;
                        srvDesc.Texture2DArray.FirstArraySlice = slice.baseArrayLayer;
                        srvDesc.Texture2DArray.ArraySize = arrayLayers;
                        srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
                    }
                    break;

                case ImageViewType::e3D:
                    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
                    srvDesc.Texture3D.MostDetailedMip = slice.baseMipLevel;
                    srvDesc.Texture3D.MipLevels = mipLevels;
                    srvDesc.Texture3D.ResourceMinLODClamp = 0.0f;
                    break;

                case ImageViewType::eCube:
                    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                    srvDesc.TextureCube.MostDetailedMip = slice.baseMipLevel;
                    srvDesc.TextureCube.MipLevels = mipLevels;
                    srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
                    break;

                case ImageViewType::eCubeArray:
                    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
                    srvDesc.TextureCubeArray.MostDetailedMip = slice.baseMipLevel;
                    srvDesc.TextureCubeArray.MipLevels = mipLevels;
                    srvDesc.TextureCubeArray.First2DArrayFace = slice.baseArrayLayer;
                    srvDesc.TextureCubeArray.NumCubes = arrayLayers / 6; // 6 faces per cube
                    srvDesc.TextureCubeArray.ResourceMinLODClamp = 0.0f;
                    break;
                }

            } else {
                ASSERT(false, "BAD VARIANT!");
            }

            mDevice->CreateShaderResourceView(resource, &srvDesc, mResourcePool->mSRVHeap.Resolve(handle));
            WriteAllSRVDescriptorHeapCopies(resource, &srvDesc, handle);
            return ShaderResourceId{ handle };
        }
        UnorderedAccessId D3DDevice::CreateUnorderedAccess(const GPUResourceInfo& info) {
            auto [handle, data] = mResourcePool->mUAVHeap.AcquireSlot();
            data = info;

            ID3D12Resource* resource;
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            if (eastl::holds_alternative<BufferResourceInfo>(info)) {
                auto& bufInfo = eastl::get<BufferResourceInfo>(info);
                auto& srcBuf = mResourcePool->Get(bufInfo.buffer);
                resource = srcBuf.resource.Get();

                uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
                uavDesc.Buffer.StructureByteStride = 0;
                uavDesc.Buffer.FirstElement = static_cast<UINT>(bufInfo.region.offset);
                uavDesc.Buffer.NumElements = eastl::min(static_cast<UINT>(srcBuf.info.size), static_cast<UINT>(bufInfo.region.size)) / 4;
                uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
                uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

            } else if (eastl::holds_alternative<ImageResourceInfo>(info)) {
                auto& imgInfo = eastl::get<ImageResourceInfo>(info);
                auto& srcImg = mResourcePool->Get(imgInfo.image);
                resource = srcImg.resource.Get();
                bool bMS = srcImg.info.sampleCount > 1;

                uavDesc.Format = ToDXGIFormat(imgInfo.format == Format::Inherit ? srcImg.info.format : imgInfo.format);

                const auto& slice = imgInfo.slice;
                auto arrayLayers = srcImg.info.arrayLayerCount;

                // Pick the correct view dimension
                switch (imgInfo.viewType) {
                case ImageViewType::e1D:
                    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
                    uavDesc.Texture1D.MipSlice = slice.baseMipLevel;
                    break;

                case ImageViewType::e1DArray:
                    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
                    uavDesc.Texture1DArray.MipSlice = slice.baseMipLevel;
                    uavDesc.Texture1DArray.FirstArraySlice = slice.baseArrayLayer;
                    uavDesc.Texture1DArray.ArraySize = arrayLayers;
                    break;

                case ImageViewType::e2D:
                    if (bMS) {
                        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DMS;
                    } else {
                        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
                        uavDesc.Texture2D.MipSlice = slice.baseMipLevel;
                    }
                    break;

                case ImageViewType::e2DArray:
                    if (bMS) {
                        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DMSARRAY;
                        uavDesc.Texture2DMSArray.FirstArraySlice = slice.baseArrayLayer;
                        uavDesc.Texture2DMSArray.ArraySize = arrayLayers;
                    } else {
                        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                        uavDesc.Texture2DArray.MipSlice = slice.baseMipLevel;
                        uavDesc.Texture2DArray.FirstArraySlice = slice.baseArrayLayer;
                        uavDesc.Texture2DArray.ArraySize = arrayLayers;
                    }
                    break;

                case ImageViewType::e3D:
                    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
                    uavDesc.Texture3D.MipSlice = slice.baseMipLevel;
                    break;
                }

            } else {
                ASSERT(false, "BAD VARIANT!");
            }

            mDevice->CreateUnorderedAccessView(resource, nullptr, &uavDesc, mResourcePool->mUAVHeap.Resolve(handle));
            return UnorderedAccessId{ handle };
        }

        PYRO_FORCEINLINE static D3D12_FILTER ToD3D12Filter(const SamplerInfo& info) {
            if (info.enableAnisotropy) {
                return info.enableCompare
                           ? D3D12_FILTER_COMPARISON_ANISOTROPIC
                           : D3D12_FILTER_ANISOTROPIC;
            }

            const bool minLinear = info.minificationFilter != Filter::Nearest;
            const bool magLinear = info.magnificationFilter != Filter::Nearest;
            const bool mipLinear = info.mipmapFilter != Filter::Nearest;

            if (info.enableCompare) {
                if (!minLinear && !magLinear && !mipLinear)
                    return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
                else if (!minLinear && magLinear && !mipLinear)
                    return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
                else if (minLinear && !magLinear && !mipLinear)
                    return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
                else if (!minLinear && !magLinear && mipLinear)
                    return D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
                else if (minLinear && magLinear && mipLinear)
                    return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
                else
                    return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; // fallback for invalid mixes
            } else {
                if (!minLinear && !magLinear && !mipLinear)
                    return D3D12_FILTER_MIN_MAG_MIP_POINT;
                else if (!minLinear && magLinear && !mipLinear)
                    return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
                else if (minLinear && !magLinear && !mipLinear)
                    return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
                else if (!minLinear && !magLinear && mipLinear)
                    return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
                else if (minLinear && magLinear && mipLinear)
                    return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
                else
                    return D3D12_FILTER_MIN_MAG_MIP_LINEAR; // fallback for unsupported combinations
            }
        }
        SamplerId D3DDevice::CreateSampler(const SamplerInfo& info) {
            auto [handle, data] = mResourcePool->mSamplerHeap.AcquireSlot();

            D3D12_SAMPLER_DESC samplerDesc = {};
            samplerDesc.Filter = ToD3D12Filter(info);
            samplerDesc.AddressU = ToD3D12SamplerAddressMode(info.addressModeU);
            samplerDesc.AddressV = ToD3D12SamplerAddressMode(info.addressModeV);
            samplerDesc.AddressW = ToD3D12SamplerAddressMode(info.addressModeW);
            samplerDesc.MinLOD = info.minLod;
            samplerDesc.MaxLOD = info.maxLod;
            samplerDesc.MipLODBias = info.mipLodBias;
            samplerDesc.MaxAnisotropy = info.maxAnisotropy;
            samplerDesc.ComparisonFunc = ToD3D12CompareOp(info.compareOp);
            data = info;

            mDevice->CreateSampler(&samplerDesc, mResourcePool->mSamplerHeap.Resolve(handle));
            return SamplerId(handle);
        }
        RenderTarget D3DDevice::CreateRenderTarget(const RenderTargetInfo& info) {
            RenderTargetInfo i = info;
            const auto& imgInfo = mResourcePool->Get(info.image);
            bool bDSV = RHIUtil::FormatIsDepthStencil(imgInfo.info.format);

            auto& heap = bDSV ? mResourcePool->mDSVHeap : mResourcePool->mRTVHeap;
            auto [handle, data] = heap.AcquireSlot();
            return RenderTarget(new D3DRenderTarget(this, bDSV, heap.Resolve(handle), eastl::move(i)));
        }
        RasterPipeline D3DDevice::CreateRasterPipeline(const RasterPipelineInfo& info, const RasterPipelineShaderStages& rasterShaderStages) {
            return RasterPipeline(new D3DRasterPipeline(this, info, mRootSignature.Get(), rasterShaderStages));
        }
        ComputePipeline D3DDevice::CreateComputePipeline(const ComputePipelineInfo& info, const ShaderInfo& computeShaderInfo) {
            return ComputePipeline(new D3DComputePipeline(this, info, mRootSignature.Get(), computeShaderInfo));
        }
        ISwapChain* D3DDevice::CreateSwapChain(const SwapChainInfo& info) {
            SwapChainInfo i = info;
            return new D3DSwapChain(this, eastl::move(i));
        }
        Semaphore D3DDevice::CreateSemaphore(const SemaphoreInfo& info) {
            SemaphoreInfo i = info;
            return Semaphore(new D3DSemaphore(this, eastl::move(i)));
        }
        IFence* D3DDevice::CreateFence(const FenceInfo& info) {
            FenceInfo i = info;
            return new D3DFence(this, eastl::move(i));
        }
        ITimestampQueryPool* D3DDevice::CreateTimestampQueryPool(const TimestampQueryPoolInfo& info) {
            return new D3DTimestampQueryPool(this, info);
        }
        ICommandBuffer* D3DDevice::GetCommandBuffer(const CommandBufferInfo& info) {
            D3DCommandBuffer* commands = nullptr;
            if (mPooledCommandBuffers.empty()) {
                ComPtr<ID3D12CommandAllocator> mCommandAllocator = {};
                ComPtr<ID3D12GraphicsCommandList> commandList = {};

                CheckD3DResult(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCommandAllocator)));
                D3DSetDebugName(mCommandAllocator, (info.name + " Allocator").c_str());
                CheckD3DResult(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));
                commands = new D3DCommandBuffer(this, eastl::move(commandList), eastl::move(mCommandAllocator));
            } else {

                commands = mPooledCommandBuffers.back();
                mPooledCommandBuffers.pop_back();
            }
            D3DSetDebugName(commands->GetCommands(), info.name.c_str());

            if (commands->bUsedBefore) {
                commands->Reset();
            } else {
                commands->bUsedBefore = true;
            }
            commands->GetCommands()->SetGraphicsRootSignature(mRootSignature.Get());
            commands->GetCommands()->SetComputeRootSignature(mRootSignature.Get());

            commands->GetCommands()->SetGraphicsRoot32BitConstant(17, 0, 0);

            eastl::array<ID3D12DescriptorHeap* const, 2u> descriptorHeaps{
                mDefaultUAVDescriptorTable.mHeap.Get(),
                mResourcePool->mSamplerHeap.InternalHeap()
            };

            commands->GetCommands()->SetDescriptorHeaps(static_cast<UINT>(descriptorHeaps.size()), descriptorHeaps.data());
            commands->GetCommands()->SetGraphicsRootDescriptorTable(14, mDefaultUAVDescriptorTable.gpuDescriptor);
            commands->GetCommands()->SetGraphicsRootDescriptorTable(15, mResourcePool->mSamplerHeap.DeviceHandle());
            commands->GetCommands()->SetComputeRootDescriptorTable(14, mDefaultUAVDescriptorTable.gpuDescriptor);
            commands->GetCommands()->SetComputeRootDescriptorTable(15, mResourcePool->mSamplerHeap.DeviceHandle());

            return commands;
        }
        void D3DDevice::DestroyMemoryBlock(MemoryBlock& memory) {
            auto& data = mResourcePool->Get(memory);
            ASSERT(data.debugRefs == 0, "All resources using this memory block must be freed beforehand!");
            mResourcePool->ReleaseMemoryBlock(memory);
            memory = PYRO_NULL_MEMORY_BLOCK;
        }
        void D3DDevice::DestroyBuffer(Buffer& buffer) {
            auto& data = mResourcePool->Get(buffer);
            if (data.mappedMemory) {
                bool bReadback = false;
                if (data.info.memoryBlock) {
                    bReadback = mResourcePool->Get(data.info.memoryBlock).info.domain == MemoryAllocationDomain::HostReadback;
                } else {
                    bReadback = data.info.allocationDomain == MemoryAllocationDomain::HostReadback;
                }
                data.resource->Unmap(0, bReadback ? nullptr : &data.range);
            }
            if (data.virtualAlloc.AllocHandle != 0) {
                auto& block = mResourcePool->Get(data.info.memoryBlock);
                block.block->FreeAllocation(data.virtualAlloc);
                --block.debugRefs;
            }
            mResourcePool->ReleaseBuffer(buffer);
            buffer = PYRO_NULL_BUFFER;
        }
        void D3DDevice::DestroyImage(Image& image) {
            auto& imgSlot = mResourcePool->Get(image);
            for (GPUResourceId id : imgSlot.blitImageRTVs) {
                mResourcePool->mRTVHeap.ReleaseSlot(id);
            }
            if (imgSlot.virtualAlloc.AllocHandle != 0) {
                auto& block = mResourcePool->Get(imgSlot.info.memoryBlock);
                block.block->FreeAllocation(imgSlot.virtualAlloc);
                --block.debugRefs;
            }
            mResourcePool->ReleaseImage(image);
            image = PYRO_NULL_IMAGE;
        }
        void D3DDevice::DestroyShaderResource(ShaderResourceId& srv) {
            mResourcePool->mSRVHeap.ReleaseSlot(srv);
            srv = PYRO_NULL_SRV;
        }
        void D3DDevice::DestroyUnorderedAccess(UnorderedAccessId& uav) {
            // HACK: invalidate the cache! the handle might be reused
            // This is because the handles currently don't have versioning! (EW!)
            // So once that is fixed, it should naturally get garbage collected by the CollectGarbage()
            DestroyAllUAVDescriptorHeapCopies(uav);
            mResourcePool->mUAVHeap.ReleaseSlot(uav);
            uav = PYRO_NULL_UAV;
        }
        void D3DDevice::DestroySampler(SamplerId& sampler) {
            mResourcePool->mSamplerHeap.ReleaseSlot(sampler);
            sampler = PYRO_NULL_SAMPLER;
        }
        void D3DDevice::DestroyRenderTarget(RenderTarget& renderTarget) {
            auto* rt = eastl::bit_cast<D3DRenderTarget*>(renderTarget);
            auto& heap = rt->IsDSV() ? mResourcePool->mDSVHeap : mResourcePool->mRTVHeap;
            heap.ReleaseSlot(rt->GetDescriptor());
            delete rt;
            renderTarget = nullptr;
        }
        void D3DDevice::DestroyRasterPipeline(RasterPipeline& pipeline) {
            delete eastl::bit_cast<D3DRasterPipeline*>(pipeline);
            pipeline = nullptr;
        }
        void D3DDevice::DestroyComputePipeline(ComputePipeline& pipeline) {
            delete eastl::bit_cast<D3DComputePipeline*>(pipeline);
            pipeline = nullptr;
        }
        void D3DDevice::DestroySwapChain(ISwapChain*& swapChain) {
            delete static_cast<D3DSwapChain*>(swapChain);
            swapChain = nullptr;
        }
        void D3DDevice::DestroySemaphore(Semaphore& semaphore) {
            delete eastl::bit_cast<D3DSemaphore*>(semaphore);
            semaphore = nullptr;
        }
        void D3DDevice::DestroyFence(IFence*& fence) {
            delete static_cast<D3DFence*>(fence);
            fence = nullptr;
        }
        void D3DDevice::DestroyTimestampQueryPool(ITimestampQueryPool*& queryPool) {
            delete static_cast<D3DTimestampQueryPool*>(queryPool);
            queryPool = nullptr;
        }
        eastl::optional<Format> D3DDevice::PickSupportedFormat(const eastl::span<Format>& candidates, FormatFeatureFlags features) {
            return eastl::optional<Format>();
        }
        eastl::span<ICommandQueue*> D3DDevice::GetCommandQueues() {
            return mCommandQueueList;
        }
        ICommandQueue* D3DDevice::GetPresentQueue() {
            return mCommandQueue;
        }
        void D3DDevice::WaitIdle() {
            mCommandQueue->WaitIdle();
        }

        void D3DDevice::SubmitQueue(const CommandQueueSubmitInfo& info) {
            CollectGarbage();
            auto* q = static_cast<D3DCommandQueue*>(info.queue);
            q->InternalQueue()->ExecuteCommandLists(static_cast<UINT>(q->mPendingCommandListExecutes.size()), q->mPendingCommandListExecutes.data());
            q->mPendingCommandListExecutes.clear();

            // Fence for deferred resource destruction from incoming command buffers
            UINT64 fenceForThisFrame = mNextDeferredDeleterValue++;
            // FIXME: multiple queues?
            CheckD3DResult(q->InternalQueue()->Signal(mDeferredDeleterFence.Get(), fenceForThisFrame));

            // signal the given fences
            for (auto [fence, value] : info.signalFences) {
                auto* s = static_cast<D3DFence*>(fence);
                CheckD3DResult(q->InternalQueue()->Signal(s->InternalFence(), value));
            }

            // pool back the submitted command buffers for later reuse
            // and also add the zombies to the destroy queue
            for (D3DCommandBuffer* cmb : q->mSubmittedCommands) {
                mPooledCommandBuffers.emplace_back(cmb);
                for (i32 i = 0; i < cmb->mDeferredDeleteOps.size(); ++i) {
                    mDeferredDeletes.emplace_back(fenceForThisFrame, eastl::move(cmb->mDeferredDeleteOps[i]));
                }
                cmb->mDeferredDeleteOps.clear();
                for (i32 i = 0; i < cmb->mPendingReturnLinearUploadBuffers.size(); ++i) {
                    mOccupiedLinearUploadBuffers.emplace_back(fenceForThisFrame, eastl::move(cmb->mPendingReturnLinearUploadBuffers[i]));
                }
                cmb->mPendingReturnLinearUploadBuffers.clear();
            }
            q->mSubmittedCommands.clear();
        }
        void D3DDevice::PresentQueue(const CommandQueuePresentInfo& info) {
            auto* q = static_cast<D3DCommandQueue*>(info.queue);
            for (auto [swapChain, syncInterval] : q->mPendingSwapPresents) {
                // TODO: how to get rid of the stupid dxgi delay?
                swapChain->Present(syncInterval, DXGI_PRESENT_DO_NOT_WAIT);
            }
            q->mPendingSwapPresents.clear();
        }
        const DeviceInfo& D3DDevice::GetInfo() {
            static DeviceInfo x{};
            return x;
        }
        const DevicePropertiesInfo& D3DDevice::GetProperties() {
            return mProperties;
        }

        // This creates the necessary SRVs/RTVs for image blits
        void D3DDevice::ImageAddIfNecessaryBlitSupport(D3DImageResourceData& data) {
            if (data.info.usage & ImageUsageFlagBits::BLIT_SRC) {
                data.blitImageSRVHeaps.resize(data.info.arrayLayerCount * data.info.mipLevelCount);

                D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                srvDesc.Format = data.desc.Format;
                srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

                for (i32 i = 0; i < data.info.mipLevelCount; ++i) {
                    for (i32 j = 0; j < data.info.arrayLayerCount; ++j) {
                        UINT subresource = D3D12CalcSubresource(i, j, 0, data.info.mipLevelCount, data.info.arrayLayerCount);
                        DescriptorTableInfo& table = data.blitImageSRVHeaps[subresource];

                        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
                        heapDesc.NumDescriptors = 1;
                        heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                        heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

                        CheckD3DResult(mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&table.mHeap)));
                        eastl::string name = "Blit Src Descriptor Heap ";
                        name += "Mip = " + eastl::to_string(i) + ", Layer=" + eastl::to_string(j);
                        D3DSetDebugName(table.mHeap, name.c_str());
                        table.cpuDescriptor = table.mHeap->GetCPUDescriptorHandleForHeapStart();
                        table.gpuDescriptor = table.mHeap->GetGPUDescriptorHandleForHeapStart();

                        if (data.info.arrayLayerCount > 1) {
                            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                            srvDesc.Texture2DArray.MostDetailedMip = i;
                            srvDesc.Texture2DArray.MipLevels = 1;
                            srvDesc.Texture2DArray.FirstArraySlice = j;
                            srvDesc.Texture2DArray.ArraySize = 1;
                            srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
                        } else {
                            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                            srvDesc.Texture2D.MostDetailedMip = i;
                            srvDesc.Texture2D.MipLevels = 1;
                            srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
                        }
                        mDevice->CreateShaderResourceView(data.resource.Get(), &srvDesc, table.cpuDescriptor);
                    }
                }
            }
            if (data.info.usage & ImageUsageFlagBits::BLIT_DST) {
                data.blitImageRTVs.resize(data.info.arrayLayerCount * data.info.mipLevelCount);

                for (i32 i = 0; i < data.info.mipLevelCount; ++i) {
                    for (i32 j = 0; j < data.info.arrayLayerCount; ++j) {
                        UINT subresource = D3D12CalcSubresource(i, j, 0, data.info.mipLevelCount, data.info.arrayLayerCount);
                        GPUResourceId& descriptor = data.blitImageRTVs[subresource];
                        descriptor = mResourcePool->mRTVHeap.AcquireSlot().first;
                        D3D12_RENDER_TARGET_VIEW_DESC view = {};
                        view.Format = data.desc.Format;
                        if (data.info.arrayLayerCount > 1) {
                            view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                            view.Texture2DArray.MipSlice = i;
                            view.Texture2DArray.FirstArraySlice = j;
                            view.Texture2DArray.ArraySize = 1;
                        } else {
                            view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                            view.Texture2D.MipSlice = i;
                        }
                        mResourcePool->mRTVHeap.AcquireSlot();
                        mDevice->CreateRenderTargetView(data.resource.Get(), &view, mResourcePool->mRTVHeap.Resolve(descriptor));
                    }
                }
            }
        }

        const DescriptorTableInfo& D3DDevice::GetUnorderedAccessViewDescriptorTable(const UAVDescriptorTableCache& desc) {
            if (desc.EffectivelyEmpty()) {
                return mDefaultUAVDescriptorTable;
            }
            auto it = mUAVDescriptorTableCache.find(desc);
            if (it != mUAVDescriptorTableCache.end()) {
                it->second.first = 0;
                return it->second.second;
            }
            Logger::Trace(gDX12Sink, "[D3D12] Copying descriptor tables");

            DescriptorTableInfo tableInfo{};
            D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
            heapDesc.NumDescriptors = NUM_CRV_SRV_UAV + Limits::MAX_UNORDERED_ACCESS_VIEW_SLOTS;
            heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

            CheckD3DResult(mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&tableInfo.mHeap)));
            D3DSetDebugName(tableInfo.mHeap, "SRV-UAV Descriptor Copy Heap");
            tableInfo.cpuDescriptor = tableInfo.mHeap->GetCPUDescriptorHandleForHeapStart();
            tableInfo.gpuDescriptor = tableInfo.mHeap->GetGPUDescriptorHandleForHeapStart();
            UINT incsz = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

            UINT cpySz = NUM_CRV_SRV_UAV;
            D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = mResourcePool->mSRVHeap.HostHandle();
            mDevice->CopyDescriptors(1, &tableInfo.cpuDescriptor, &cpySz,
                1, &srcHandle, &cpySz, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

            INT index = 0;
            for (UnorderedAccessId id : desc.boundUavs) {
                if (id != PYRO_NULL_UAV) {
                    CD3DX12_CPU_DESCRIPTOR_HANDLE currOffsetHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(tableInfo.cpuDescriptor);
                    D3D12_CPU_DESCRIPTOR_HANDLE handle = mResourcePool->mUAVHeap.Resolve(id);
                    mDevice->CopyDescriptorsSimple(1U, currOffsetHandle.Offset((NUM_CRV_SRV_UAV + index) * incsz), handle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                }
                ++index;
            }

            return mUAVDescriptorTableCache.emplace(desc, eastl::pair{ 0, eastl::move(tableInfo) }).first->second.second;
        }

        ID3D12PipelineState* D3DDevice::GetBlitImagePipeline(DXGI_FORMAT format, bool bArray) {
            UINT64 hash = static_cast<UINT>(format) | (bArray ? 0x100000000ULL : 0x0000000ULL);

            auto it = mBlitImagePipelineStates.find(hash);
            if (it != mBlitImagePipelineStates.end())
                return it->second.Get();


            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
            psoDesc.pRootSignature = mBlitImageRootSignature.Get();
            psoDesc.VS = { static_cast<const void*>(gBlitImageVS), PYRO_ARRAY_SIZE(gBlitImageVS) };
            psoDesc.PS = { static_cast<const void*>(gBlitImagePS), PYRO_ARRAY_SIZE(gBlitImagePS) };
            psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
            // --- Rasterizer State ---
            D3D12_RASTERIZER_DESC rastDesc = {};
            rastDesc.FillMode = D3D12_FILL_MODE_SOLID;
            rastDesc.CullMode = D3D12_CULL_MODE_NONE;
            rastDesc.FrontCounterClockwise = TRUE;
            rastDesc.DepthBias = 0;
            rastDesc.DepthBiasClamp = 0;
            rastDesc.SlopeScaledDepthBias = 0;
            rastDesc.DepthClipEnable = FALSE;
            rastDesc.MultisampleEnable = FALSE;
            rastDesc.AntialiasedLineEnable = FALSE;
            rastDesc.ForcedSampleCount = 0;
            rastDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
            psoDesc.RasterizerState = rastDesc;
            psoDesc.SampleMask = 0xFFFFFFFFU;
            // --- Blend State ---
            D3D12_BLEND_DESC blendDesc = {};
            blendDesc.AlphaToCoverageEnable = false;
            blendDesc.IndependentBlendEnable = false;
            blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
            blendDesc.RenderTarget[0].BlendEnable = FALSE;
            psoDesc.BlendState = blendDesc;

            D3D12_DEPTH_STENCIL_DESC dsDesc = {};
            dsDesc.DepthEnable = FALSE;
            dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
            dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            psoDesc.DepthStencilState = dsDesc;
            psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            // --- Render Targets ---
            psoDesc.NumRenderTargets = 1U;
            psoDesc.RTVFormats[0] = format;
            psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;

            // --- Multisampling ---
            psoDesc.SampleDesc.Count = 1;
            psoDesc.SampleDesc.Quality = 0;


            ComPtr<ID3D12PipelineState>& entry = mBlitImagePipelineStates[hash];
            // --- Create PSO ---
            CheckD3DResult(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&entry)));
            eastl::string name = eastl::string("Blit image pipeline state DXGI format=") + eastl::to_string((UINT)format) + ", Array=" +
                                 (bArray ? "true" : "false");
            D3DSetDebugName(entry, name.c_str());

            Logger::Trace(gDX12Sink, "[D3D12] Creating image blit pipeline state object");

            return entry.Get();
        }

        LinearUploadBuffer* D3DDevice::GetLinearBufferAllocation(UINT64 minSize) {
            if (mAvailableLinearUploadBuffers.empty()) {
                return new LinearUploadBuffer(mDevice.Get(), minSize);
            } else {
                LinearUploadBuffer* buff = mAvailableLinearUploadBuffers.back().second;
                mAvailableLinearUploadBuffers.pop_back();
                buff->Reset();
                return buff;
            }
        }

        void D3DDevice::WriteAllSRVDescriptorHeapCopies(ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc, GPUResourceId handle) {
            UINT incSz = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            CD3DX12_CPU_DESCRIPTOR_HANDLE h = CD3DX12_CPU_DESCRIPTOR_HANDLE(mDefaultUAVDescriptorTable.cpuDescriptor);
            mDevice->CreateShaderResourceView(pResource, pDesc, h.Offset(handle.index * incSz));
            for (auto& [cache, data] : mUAVDescriptorTableCache) {
                h = CD3DX12_CPU_DESCRIPTOR_HANDLE(data.second.cpuDescriptor);
                mDevice->CreateShaderResourceView(pResource, pDesc, h.Offset(handle.index * incSz));
            }
        }
        void D3DDevice::DestroyAllUAVDescriptorHeapCopies(UnorderedAccessId handle) {
            eastl::vector<UAVDescriptorTableCache> destroyCaches = {};
            for (auto& [cache, data] : mUAVDescriptorTableCache) {
                for (i32 i = 0; i < cache.boundUavs.size(); ++i) {
                    if (cache.boundUavs[i] == handle) {
                        destroyCaches.emplace_back(cache);
                    }
                }
            }
            for (const auto& cache : destroyCaches) {
                mUAVDescriptorTableCache.erase(cache);
            }
        }

        void D3DDevice::CollectGarbage() {
            UINT64 completedFence = mDeferredDeleterFence->GetCompletedValue();

            // Delete objects scheduled for later destruction
            for (usize i = 0; i < mDeferredDeletes.size(); ++i) {
                auto& [fenceVal, zombie] = mDeferredDeletes[i];
                if (fenceVal > completedFence)
                    continue;
                zombie.deleter(this, zombie.resource);
                mDeferredDeletes.erase(mDeferredDeletes.begin() + i);
                --i;
            }

            // Put back used linear upload buffers
            for (usize i = 0; i < mOccupiedLinearUploadBuffers.size(); ++i) {
                auto& [fenceVal, zombie] = mOccupiedLinearUploadBuffers[i];
                if (fenceVal > completedFence)
                    continue;
                mAvailableLinearUploadBuffers.emplace_back(eastl::pair{ 0ULL, zombie });
                mOccupiedLinearUploadBuffers.erase(mOccupiedLinearUploadBuffers.begin() + i);
                --i;
            }
            // Delete linear upload buffers that haven't been used in a long time
            for (usize i = 0; i < mAvailableLinearUploadBuffers.size(); ++i) {
                auto& [unusedFrames, zombie] = mAvailableLinearUploadBuffers[i];
                if (unusedFrames++ > MAX_FRAMES_LINEAR_UPLOAD_BUFFER_UNSUED_LIFETIME) {
                    delete zombie;
                    mAvailableLinearUploadBuffers.erase(mAvailableLinearUploadBuffers.begin() + i);
                    --i;
                }
            }

            // cleanup UAV tables that haven't been used in a long time
            eastl::vector<UAVDescriptorTableCache> deleteUAVTableCacheHandles = {};
            for (auto& [cacheHandle, h] : mUAVDescriptorTableCache) {
                auto& [unusedFrames, signature] = h;
                if (unusedFrames++ > MAX_FRAMES_UAV_TABLE_CACHE_UNUSED_LIFETIME) {
                    deleteUAVTableCacheHandles.emplace_back(cacheHandle);
                }
            }
            for (auto handle : deleteUAVTableCacheHandles) {
                mUAVDescriptorTableCache.erase(handle);
            }
        }

    } // namespace RHIDX12
} // namespace PyroshockStudios
