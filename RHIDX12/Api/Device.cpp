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
#include <RHIDX12/D3DContext.hpp>
#include <RHIDX12/InternalShaders.hpp>
#include <RHIDX12/Variables.hpp>

#include <libassert/assert.hpp>

#include <comdef.h>

namespace PyroshockStudios {
    namespace RHIDX12 {
        constexpr UINT NUM_SAMPLERS = 2048;
        constexpr UINT NUM_CRV_SRV_UAV = 32768;
        constexpr UINT64 MAX_FRAMES_UAV_TABLE_CACHE_UNUSED_LIFETIME = 60;
        constexpr UINT64 MAX_FRAMES_LINEAR_UPLOAD_BUFFER_UNSUED_LIFETIME = 100;

        D3DDevice::D3DDevice(ComPtr<ID3D12Device>&& device, ComPtr<IDXGIFactory4>&& factory, ComPtr<IDXGIAdapter1>&& adapter)
            : mDevice(eastl::move(device)), mFactory(eastl::move(factory)), mAdapter(eastl::move(adapter)) {

            mDevice->QueryInterface(IID_PPV_ARGS(&mDevice5));

            CheckFeatureSupport();
            CreateCommandQueues();
            InitializeResourcePool();
            CreateDescriptorHeaps();
            CreateGlobalRootSignature();
            CreateBlitImageRootSignature();
            CreateDefaultSamplerHeaps();
            CreateCommandSignatures();
            CreateMemoryAllocator();
            PopulateDeviceInfo();
            PopulateDeviceFeatures();
            PopulateDeviceProperties();
        }
        D3DDevice::~D3DDevice() {
            Logger::Debug(gDX12Sink, "Destroying Device...");
            WaitIdle();
            gDx12Context->FlushDebugMessages();
            CollectGarbage();
            gDx12Context->FlushDebugMessages();
            ASSERT(mDeferredDeletes.Empty(), "Command buffers must finish execution before device destruction! Deferred destruction was leaked!");
            DestroyCommandQueues();
            DestroyUploadBuffers();
            ReportDeviceRemovalReason();
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
        bool D3DDevice::IsBlasValid(BlasId id) const {
            return id.index != 0;
        }
        bool D3DDevice::IsTlasValid(TlasId id) const {
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
        const GpuResourceInfo& D3DDevice::GetShaderResourceInfo(ShaderResourceId id) const {
            ASSERT(IsValid(id));
            return eastl::get<GpuResourceInfo>(mResourcePool->mSRVHeap.GetInfo(id));
        }
        const GpuResourceInfo& D3DDevice::GetUnorderedAccessInfo(UnorderedAccessId id) const {
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

        const BlasInfo& D3DDevice::GetBlasInfo(BlasId blas) const {
            ASSERT(IsValid(blas));
            return mResourcePool->Get(blas).info;
        }

        const TlasInfo& D3DDevice::GetTlasInfo(TlasId tlas) const {
            ASSERT(IsValid(tlas));
            return eastl::get<D3DTlasData>(mResourcePool->mSRVHeap.GetInfo(tlas)).info;
        }

        DeviceAddress D3DDevice::BufferDeviceAddress(Buffer buffer) const {
            ASSERT(IsValid(buffer));
            return DeviceAddress();
        }

        u8* D3DDevice::BufferHostAddress(Buffer buffer) const {
            ASSERT(IsValid(buffer));
            return mResourcePool->Get(buffer).mappedMemory;
        }
        BlasAddress D3DDevice::BlasInstanceAddress(BlasId blas) const {
            return static_cast<BlasAddress>(mResourcePool->Get(blas).address);
        }
        DeviceSize D3DDevice::ImageSizeRequirements(Image image) const {
            ASSERT(IsValid(image));

            D3D12_RESOURCE_ALLOCATION_INFO resourceAllocInfo = mDevice->GetResourceAllocationInfo(0, 1, &mResourcePool->Get(image).desc);
            gDx12Context->FlushDebugMessages();
            return resourceAllocInfo.SizeInBytes;
        }
        u32 D3DDevice::ImageSubresourceRowPitch(Image image, u32 rowWidth, ImageSlice slice) const {
            auto& img = mResourcePool->Get(image);
            UINT dstSubresource = D3D12CalcSubresource(slice.mipLevel, slice.arrayLayer, 0, img.info.mipLevelCount, img.info.arrayLayerCount);
            D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
            UINT numRows = {};
            UINT64 rowSizesInBytes = {};
            UINT64 requiredSize = {};
            mDevice->GetCopyableFootprints(&img.desc, dstSubresource, 1, 0,
                &footprint, &numRows, &rowSizesInBytes, &requiredSize);
            gDx12Context->FlushDebugMessages();
            return footprint.Footprint.RowPitch;
        }
        AccelerationStructureBuildSizesInfo D3DDevice::BlasSizeRequirements(const BlasBuildInfo& info) const {
            if (!mDevice5) {
                Logger::Error(gDX12Sink, "Cannot get BLAS size requirements: ID3D12Device5 is not available.");
                return {};
            }

            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
            eastl::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geomDescs; // Scratch space
            FillBlasInputs(inputs, info, geomDescs);

            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo = {};
            mDevice5->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuildInfo);

            return {
                .accelerationStructureSize = prebuildInfo.ResultDataMaxSizeInBytes,
                .updateScratchSize = prebuildInfo.UpdateScratchDataSizeInBytes,
                .buildScratchSize = prebuildInfo.ScratchDataSizeInBytes
            };
        }
        AccelerationStructureBuildSizesInfo D3DDevice::TlasSizeRequirements(const TlasBuildInfo& info) const {
            if (!mDevice5) {
                Logger::Error(gDX12Sink, "Cannot get TLAS size requirements: ID3D12Device5 is not available.");
                return {};
            }

            D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
            inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
            inputs.Flags = ToD3D12ASBuildFlags(info.flags);
            if (info.update) {
                inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
            }

            inputs.NumDescs = info.instances.count;
            inputs.InstanceDescs = mResourcePool->Get(info.instances.data).resource->GetGPUVirtualAddress();

            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo = {};
            mDevice5->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuildInfo);

            return {
                .accelerationStructureSize = prebuildInfo.ResultDataMaxSizeInBytes,
                .updateScratchSize = prebuildInfo.UpdateScratchDataSizeInBytes,
                .buildScratchSize = prebuildInfo.ScratchDataSizeInBytes
            };
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
            CheckD3DResult(mDevice->CreateHeap(&heapDec, IID_PPV_ARGS(&data.heap)), "Failed to create memory block ID3D12Heap!");
            D3DSetDebugName(data.heap, info.name.c_str());
            D3D12MA::VIRTUAL_BLOCK_DESC vblockDesc = {};
            vblockDesc.Size = info.size;
            if (info.strategy == VirtualSuballocationStrategy::AggressiveRing) {
                D3D12MA::VIRTUAL_BLOCK_DESC vblockDesc = {};
                (u32&)vblockDesc.Flags |= D3D12MA::VIRTUAL_BLOCK_FLAG_ALGORITHM_LINEAR;
            }
            CheckD3DResult(D3D12MA::CreateVirtualBlock(&vblockDesc, data.block.GetAddressOf()), "Failed to create memory block allocator!");
            gDx12Context->FlushDebugMessages();
            return memory;
        }
        Buffer D3DDevice::CreateBuffer(const BufferInfo& info) {
            auto [buffer, data] = mResourcePool->AllocBuffer();
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

            if ((info.usage & (BufferUsageFlagBits::UNORDERED_ACCESS |
                                  BufferUsageFlagBits::ACCELERATION_STRUCTURE_SCRATCH_BUFFER)) != BufferUsageFlagBits::NONE)
                bufferDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

            if ((info.usage & (BufferUsageFlagBits::SHADER_RESOURCE |
                                  BufferUsageFlagBits::BLAS_GEOMETRY_BUFFER |
                                  BufferUsageFlagBits::BLAS_INSTANCE_BUFFER)) == BufferUsageFlagBits::NONE)
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
                ASSERT(info.initialLayout == BufferLayout::TransferDst);
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
                CheckD3DResult(hr, "Failed to create buffer resource without memory block");

            } else {
                auto& block = mResourcePool->Get(info.memoryBlock);
                D3D12MA::VIRTUAL_ALLOCATION_DESC allocDesc = {};
                D3D12_RESOURCE_ALLOCATION_INFO resourceAllocInfo = mDevice->GetResourceAllocationInfo(0, 1, &bufferDesc);
                allocDesc.Size = resourceAllocInfo.SizeInBytes;
                allocDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
                allocDesc.Size = info.size;
                switch (block.info.strategy) {
                case VirtualSuballocationStrategy::AggressiveRing:
                case VirtualSuballocationStrategy::TimeEfficient:
                    (u32&)allocDesc.Flags |= D3D12MA::VIRTUAL_ALLOCATION_FLAG_STRATEGY_MIN_TIME;
                    break;
                case VirtualSuballocationStrategy::SpaceEfficient:
                    (u32&)allocDesc.Flags |= D3D12MA::VIRTUAL_ALLOCATION_FLAG_STRATEGY_MIN_MEMORY;
                    break;
                default:
                    break;
                }
                UINT64 offset;
                HRESULT hr = block.block->Allocate(&allocDesc, &data.virtualAlloc, &offset);
                if (hr == E_OUTOFMEMORY) {
                    Logger::Debug(gDX12Sink, "Memory block {} of size {} ran out of space to allocate buffer {} with size {}. This is not an error",
                        block.info.name, block.info.size, info.name, info.size);
                    mResourcePool->ReleaseBuffer(buffer);
                    return PYRO_NULL_BUFFER;
                }
                hr = mDevice->CreatePlacedResource(
                    block.heap.Get(), // heap from above
                    offset,           // offset of this virtual slice
                    &bufferDesc,
                    state,
                    nullptr,
                    IID_PPV_ARGS(&data.resource));
                CheckD3DResult(hr, "Failed to create buffer resource with memory block");
                ++block.debugRefs;
            }
            D3DSetDebugName(data.resource, info.name.c_str());

            if (bMap) {
                D3D12_RANGE range = { 0, info.size };
                CheckD3DResult(data.resource->Map(0, &range, reinterpret_cast<void**>(&data.mappedMemory)), "Failed to persistently map buffer memory!");
            }
            gDx12Context->FlushDebugMessages();
            return buffer;
        }
        Image D3DDevice::CreateImage(const ImageInfo& info) {
            auto [image, data] = mResourcePool->AllocImage();
            data.info = info;

            // Describe the texture
            D3D12_RESOURCE_DESC textureDesc = {};
            textureDesc.MipLevels = info.mipLevelCount;
            textureDesc.Width = static_cast<UINT64>(info.size.width);
            textureDesc.Height = info.size.height;
            textureDesc.DepthOrArraySize = static_cast<UINT16>(eastl::max(info.size.depth, info.arrayLayerCount));
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
            if (info.usage & ImageUsageFlagBits::SHADER_RESOURCE && dsv || info.flags & ImageCreateFlagBits::MUTABLE_FORMAT) {
                textureDesc.Format = ToDXGIFormatTypeless(info.format);
            } else {
                textureDesc.Format = ToDXGIFormat(info.format);
            }

            textureDesc.SampleDesc.Count = static_cast<UINT>(info.sampleCount);
            textureDesc.SampleDesc.Quality = 0;

            switch (info.dimensions) {
            case ImageDimensions::e1D:
                textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
                break;
            case ImageDimensions::e2D:
                textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                break;
            case ImageDimensions::e3D:
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
                CheckD3DResult(hr, "Failed to create image resource without memory block");
            } else {
                auto& block = mResourcePool->Get(info.memoryBlock);
                D3D12MA::VIRTUAL_ALLOCATION_DESC allocDesc = {};
                D3D12_RESOURCE_ALLOCATION_INFO resourceAllocInfo = mDevice->GetResourceAllocationInfo(0, 1, &textureDesc);
                allocDesc.Size = resourceAllocInfo.SizeInBytes;
                allocDesc.Alignment = resourceAllocInfo.Alignment;
                switch (block.info.strategy) {
                case VirtualSuballocationStrategy::AggressiveRing:
                case VirtualSuballocationStrategy::TimeEfficient:
                    (u32&)allocDesc.Flags |= D3D12MA::VIRTUAL_ALLOCATION_FLAG_STRATEGY_MIN_TIME;
                    break;
                case VirtualSuballocationStrategy::SpaceEfficient:
                    (u32&)allocDesc.Flags |= D3D12MA::VIRTUAL_ALLOCATION_FLAG_STRATEGY_MIN_MEMORY;
                    break;
                default:
                    break;
                }
                UINT64 offset;
                HRESULT hr = block.block->Allocate(&allocDesc, &data.virtualAlloc, &offset);
                if (hr == E_OUTOFMEMORY) {
                    Logger::Debug(gDX12Sink, "Memory block {} of size {} ran out of space to allocate image {} with {} bytes. This is not an error",
                        block.info.name, block.info.size, info.name, allocDesc.Size);
                    mResourcePool->ReleaseImage(image);
                    return PYRO_NULL_IMAGE;
                }
                hr = mDevice->CreatePlacedResource(
                    block.heap.Get(), // heap from above
                    offset,           // offset of this virtual slice
                    &textureDesc,
                    D3D12_RESOURCE_STATE_COMMON,
                    nullptr,
                    IID_PPV_ARGS(&data.resource));
                CheckD3DResult(hr, "Failed to create image resource with memory block");
                ++block.debugRefs;
            }

            data.desc = textureDesc;
            D3DSetDebugName(data.resource, info.name.c_str());
            gDx12Context->FlushDebugMessages();

            ImageAddIfNecessaryBlitSupport(data);

            return image;
        }
        ShaderResourceId D3DDevice::CreateShaderResource(const GpuResourceInfo& info) {
            auto [handle, data] = mResourcePool->mSRVHeap.AcquireSlot();
            data = info;

            ID3D12Resource* resource;
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            if (eastl::holds_alternative<BufferResourceInfo>(info)) {
                auto& bufInfo = eastl::get<BufferResourceInfo>(info);
                auto& srcBuf = mResourcePool->Get(bufInfo.buffer);
                resource = srcBuf.resource.Get();

                srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
                srvDesc.Buffer.StructureByteStride = 0;
                srvDesc.Buffer.FirstElement = static_cast<UINT>(bufInfo.region.offset);
                srvDesc.Buffer.NumElements = eastl::min(static_cast<UINT>(srcBuf.info.size), static_cast<UINT>(bufInfo.region.size)) / 4;
                srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
                srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

            } else if (eastl::holds_alternative<ImageResourceInfo>(info)) {
                auto& imgInfo = eastl::get<ImageResourceInfo>(info);
                auto& srcImg = mResourcePool->Get(imgInfo.image);
                resource = srcImg.resource.Get();
                bool bMS = srcImg.info.sampleCount > RasterizationSamples::e1;

                Format fmt = imgInfo.format == Format::Inherit ? srcImg.info.format : imgInfo.format;
                if (RHIUtil::FormatIsDepthStencil(fmt)) {
                    // FIXME how to read stencil only data?
                    switch (fmt) {
                    case Format::D16Unorm:
                        srvDesc.Format = DXGI_FORMAT_R16_UNORM;
                        break;
                    case Format::D32Sfloat:
                        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
                        break;
                    default:
                        ASSERT(false, "TODO implement the rest of the formats");
                        break;
                    }
                } else {
                    srvDesc.Format = ToDXGIFormat(fmt);
                }

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
            gDx12Context->FlushDebugMessages();
            WriteAllSRVDescriptorHeapCopies(resource, &srvDesc, handle);
            gDx12Context->FlushDebugMessages();
            return ShaderResourceId{ handle };
        }
        UnorderedAccessId D3DDevice::CreateUnorderedAccess(const GpuResourceInfo& info) {
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
                bool bMS = srcImg.info.sampleCount > RasterizationSamples::e1;

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
                    uavDesc.Texture3D.WSize = srcImg.info.size.depth;
                    break;
                }

            } else {
                ASSERT(false, "BAD VARIANT!");
            }

            mDevice->CreateUnorderedAccessView(resource, nullptr, &uavDesc, mResourcePool->mUAVHeap.Resolve(handle));
            gDx12Context->FlushDebugMessages();
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
            gDx12Context->FlushDebugMessages();
            return SamplerId(handle);
        }
        RenderTarget D3DDevice::CreateRenderTarget(const RenderTargetInfo& info) {
            RenderTargetInfo i = info;
            const auto& imgInfo = mResourcePool->Get(info.image);
            bool bDSV = RHIUtil::FormatIsDepthStencil(imgInfo.info.format);

            auto& heap = bDSV ? mResourcePool->mDSVHeap : mResourcePool->mRTVHeap;
            auto [handle, data] = heap.AcquireSlot();
            auto rt = RenderTarget(new D3DRenderTarget(this, bDSV, heap.Resolve(handle), eastl::move(i)));
            gDx12Context->FlushDebugMessages();
            return rt;
        }
        RasterPipeline D3DDevice::CreateRasterPipeline(const RasterPipelineInfo& info, const RasterPipelineShaderStages& rasterShaderStages) {
            auto rp = RasterPipeline(new D3DRasterPipeline(this, info, mRootSignature.Get(), rasterShaderStages));
            gDx12Context->FlushDebugMessages();
            return rp;
        }
        ComputePipeline D3DDevice::CreateComputePipeline(const ComputePipelineInfo& info, const ShaderInfo& computeShaderInfo) {
            auto cp = ComputePipeline(new D3DComputePipeline(this, info, mRootSignature.Get(), computeShaderInfo));
            gDx12Context->FlushDebugMessages();
            return cp;
        }
        ISwapChain* D3DDevice::CreateSwapChain(const SwapChainInfo& info) {
            SwapChainInfo i = info;
            auto swap = new D3DSwapChain(this, eastl::move(i));
            gDx12Context->FlushDebugMessages();
            return swap;
        }
        Semaphore D3DDevice::CreateSemaphore(const SemaphoreInfo& info) {
            SemaphoreInfo i = info;
            auto sem = Semaphore(new D3DSemaphore(this, eastl::move(i)));
            gDx12Context->FlushDebugMessages();
            return sem;
        }
        IFence* D3DDevice::CreateFence(const FenceInfo& info) {
            FenceInfo i = info;
            auto fen = new D3DFence(this, eastl::move(i));
            gDx12Context->FlushDebugMessages();
            return fen;
        }
        ITimestampQueryPool* D3DDevice::CreateTimestampQueryPool(const TimestampQueryPoolInfo& info) {
            auto qp = new D3DTimestampQueryPool(this, info);
            gDx12Context->FlushDebugMessages();
            return qp;
        }
        BlasId D3DDevice::CreateBlas(const BlasInfo& info) {
            auto [id, data] = mResourcePool->AllocBlas();
            data.info = info;

            D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(
                info.size, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

            D3D12MA::ALLOCATION_DESC allocDesc = {};
            allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
            CheckD3DResult(mAllocator->CreateResource(
                               &allocDesc,
                               &bufferDesc,
                               D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
                               nullptr,
                               &data.allocation,
                               IID_PPV_ARGS(&data.resource)),
                "Failed to create Acceleration Structure (BLAS)");

            D3DSetDebugName(data.resource, info.name.c_str());
            data.address = data.resource->GetGPUVirtualAddress();
            gDx12Context->FlushDebugMessages();

            return id;
        }
        TlasId D3DDevice::CreateTlas(const TlasInfo& info) {
            auto [id, data_] = mResourcePool->mSRVHeap.AcquireSlot();
            data_ = D3DTlasData{};
            auto& data = eastl::get<D3DTlasData>(data_);
            data.info = info;
            D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(
                info.size, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

            D3D12MA::ALLOCATION_DESC allocDesc = {};
            allocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

            CheckD3DResult(mAllocator->CreateResource(
                               &allocDesc,
                               &bufferDesc,
                               D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
                               nullptr,
                               &data.allocation,
                               IID_PPV_ARGS(&data.resource)),
                "Failed to create Acceleration Structure (TLAS)");

            D3DSetDebugName(data.resource, info.name.c_str());

            // This is a "variant" just for tracking in the RHI,
            // we don't need to fill it for this internal-only SRV.

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = DXGI_FORMAT_UNKNOWN;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.RaytracingAccelerationStructure.Location = data.resource->GetGPUVirtualAddress();

            D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = mResourcePool->mSRVHeap.Resolve(id);
            mDevice->CreateShaderResourceView(nullptr, &srvDesc, cpuHandle); // Resource is nullptr for AS SRV

            WriteAllSRVDescriptorHeapCopies(nullptr, &srvDesc, id);
            gDx12Context->FlushDebugMessages();

            return static_cast<TlasId>(id);
        }
        void D3DDevice::DestroyMemoryBlock(MemoryBlock& memory, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = reinterpret_cast<void*>(memory),
                    .deleter = [](D3DDevice* dev, void* res) { dev->DestroyImmediately(reinterpret_cast<MemoryBlock>(res)); }
                };
                mDeferredDeletes.EmplaceBack(eastl::move(SnapshotQueueFenceValues()), zombie);
                return;
            }
            auto& data = mResourcePool->Get(memory);
            ASSERT(data.debugRefs == 0, "All resources using this memory block must be freed beforehand!");
            mResourcePool->ReleaseMemoryBlock(memory);
            gDx12Context->FlushDebugMessages();
            memory = PYRO_NULL_MEMORY_BLOCK;
        }
        void D3DDevice::DestroyBuffer(Buffer& buffer, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = reinterpret_cast<void*>(buffer),
                    .deleter = [](D3DDevice* dev, void* res) { dev->DestroyImmediately(reinterpret_cast<Buffer>(res)); }
                };
                mDeferredDeletes.EmplaceBack(eastl::move(SnapshotQueueFenceValues()), zombie);
                return;
            }
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
            gDx12Context->FlushDebugMessages();
            buffer = PYRO_NULL_BUFFER;
        }
        void D3DDevice::DestroyImage(Image& image, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = reinterpret_cast<void*>(image),
                    .deleter = [](D3DDevice* dev, void* res) { dev->DestroyImmediately(reinterpret_cast<Image>(res)); }
                };
                mDeferredDeletes.EmplaceBack(eastl::move(SnapshotQueueFenceValues()), zombie);
                return;
            }
            auto& imgSlot = mResourcePool->Get(image);
            for (GpuResourceId id : imgSlot.blitImageRTVs) {
                mResourcePool->mRTVHeap.ReleaseSlot(id);
            }
            if (imgSlot.virtualAlloc.AllocHandle != 0) {
                auto& block = mResourcePool->Get(imgSlot.info.memoryBlock);
                block.block->FreeAllocation(imgSlot.virtualAlloc);
                --block.debugRefs;
            }
            mResourcePool->ReleaseImage(image);
            gDx12Context->FlushDebugMessages();
            image = PYRO_NULL_IMAGE;
        }
        void D3DDevice::DestroyShaderResource(ShaderResourceId& srv, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = eastl::bit_cast<void*>(srv),
                    .deleter = [](D3DDevice* dev, void* res) { dev->DestroyImmediately(eastl::bit_cast<ShaderResourceId>(res)); }
                };
                mDeferredDeletes.EmplaceBack(eastl::move(SnapshotQueueFenceValues()), zombie);
                return;
            }
            mResourcePool->mSRVHeap.ReleaseSlot(srv);
            gDx12Context->FlushDebugMessages();
            srv = PYRO_NULL_SRV;
        }
        void D3DDevice::DestroyUnorderedAccess(UnorderedAccessId& uav, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = eastl::bit_cast<void*>(uav),
                    .deleter = [](D3DDevice* dev, void* res) { dev->DestroyImmediately(eastl::bit_cast<UnorderedAccessId>(res)); }
                };
                mDeferredDeletes.EmplaceBack(eastl::move(SnapshotQueueFenceValues()), zombie);
                return;
            }
            // HACK: invalidate the cache! the handle might be reused
            // This is because the handles currently don't have versioning! (EW!)
            // So once that is fixed, it should naturally get garbage collected by the CollectGarbage()
            DestroyAllUAVDescriptorHeapCopies(uav);
            gDx12Context->FlushDebugMessages();
            mResourcePool->mUAVHeap.ReleaseSlot(uav);
            gDx12Context->FlushDebugMessages();
            uav = PYRO_NULL_UAV;
        }
        void D3DDevice::DestroySampler(SamplerId& sampler, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = eastl::bit_cast<void*>(sampler),
                    .deleter = [](D3DDevice* dev, void* res) { dev->DestroyImmediately(eastl::bit_cast<SamplerId>(res)); }
                };
                mDeferredDeletes.EmplaceBack(eastl::move(SnapshotQueueFenceValues()), zombie);
                return;
            }
            mResourcePool->mSamplerHeap.ReleaseSlot(sampler);
            gDx12Context->FlushDebugMessages();
            sampler = PYRO_NULL_SAMPLER;
        }
        void D3DDevice::DestroyRenderTarget(RenderTarget& renderTarget, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = eastl::bit_cast<void*>(renderTarget),
                    .deleter = [](D3DDevice* dev, void* res) { dev->DestroyImmediately(eastl::bit_cast<RenderTarget>(res)); }
                };
                mDeferredDeletes.EmplaceBack(eastl::move(SnapshotQueueFenceValues()), zombie);
                return;
            }
            auto* rt = eastl::bit_cast<D3DRenderTarget*>(renderTarget);
            auto& heap = rt->IsDSV() ? mResourcePool->mDSVHeap : mResourcePool->mRTVHeap;
            heap.ReleaseSlot(rt->GetDescriptor());
            delete rt;
            gDx12Context->FlushDebugMessages();
            renderTarget = nullptr;
        }
        void D3DDevice::DestroyRasterPipeline(RasterPipeline& pipeline, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = eastl::bit_cast<void*>(pipeline),
                    .deleter = [](D3DDevice* dev, void* res) { dev->DestroyImmediately(eastl::bit_cast<RasterPipeline>(res)); }
                };
                mDeferredDeletes.EmplaceBack(eastl::move(SnapshotQueueFenceValues()), zombie);
                return;
            }
            delete eastl::bit_cast<D3DRasterPipeline*>(pipeline);
            gDx12Context->FlushDebugMessages();
            pipeline = nullptr;
        }
        void D3DDevice::DestroyComputePipeline(ComputePipeline& pipeline, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = eastl::bit_cast<void*>(pipeline),
                    .deleter = [](D3DDevice* dev, void* res) { dev->DestroyImmediately(eastl::bit_cast<ComputePipeline>(res)); }
                };
                mDeferredDeletes.EmplaceBack(eastl::move(SnapshotQueueFenceValues()), zombie);
                return;
            }
            delete eastl::bit_cast<D3DComputePipeline*>(pipeline);
            gDx12Context->FlushDebugMessages();
            pipeline = nullptr;
        }
        void D3DDevice::DestroySwapChain(ISwapChain*& swapChain, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = reinterpret_cast<void*>(swapChain),
                    .deleter = [](D3DDevice* dev, void* res) { dev->DestroyImmediately(reinterpret_cast<ISwapChain*>(res)); }
                };
                mDeferredDeletes.EmplaceBack(eastl::move(SnapshotQueueFenceValues()), zombie);
                return;
            }
            delete static_cast<D3DSwapChain*>(swapChain);
            gDx12Context->FlushDebugMessages();
            swapChain = nullptr;
        }
        void D3DDevice::DestroySemaphore(Semaphore& semaphore, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = eastl::bit_cast<void*>(semaphore),
                    .deleter = [](D3DDevice* dev, void* res) { dev->DestroyImmediately(eastl::bit_cast<Semaphore>(res)); }
                };
                mDeferredDeletes.EmplaceBack(eastl::move(SnapshotQueueFenceValues()), zombie);
                return;
            }
            delete eastl::bit_cast<D3DSemaphore*>(semaphore);
            gDx12Context->FlushDebugMessages();
            semaphore = nullptr;
        }
        void D3DDevice::DestroyFence(IFence*& fence, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = reinterpret_cast<void*>(fence),
                    .deleter = [](D3DDevice* dev, void* res) { dev->DestroyImmediately(reinterpret_cast<IFence*>(res)); }
                };
                mDeferredDeletes.EmplaceBack(eastl::move(SnapshotQueueFenceValues()), zombie);
                return;
            }
            delete static_cast<D3DFence*>(fence);
            gDx12Context->FlushDebugMessages();
            fence = nullptr;
        }
        void D3DDevice::DestroyTimestampQueryPool(ITimestampQueryPool*& queryPool, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = reinterpret_cast<void*>(queryPool),
                    .deleter = [](D3DDevice* dev, void* res) { dev->DestroyImmediately(reinterpret_cast<ITimestampQueryPool*>(res)); }
                };
                mDeferredDeletes.EmplaceBack(eastl::move(SnapshotQueueFenceValues()), zombie);
                return;
            }
            delete static_cast<D3DTimestampQueryPool*>(queryPool);
            gDx12Context->FlushDebugMessages();
            queryPool = nullptr;
        }
        void D3DDevice::DestroyBlas(BlasId& blas, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = eastl::bit_cast<void*>(blas),
                    .deleter = [](D3DDevice* dev, void* res) { dev->DestroyImmediately(eastl::bit_cast<BlasId>(res)); }
                };
                mDeferredDeletes.EmplaceBack(eastl::move(SnapshotQueueFenceValues()), zombie);
                return;
            }
            mResourcePool->ReleaseBlas(blas);
            gDx12Context->FlushDebugMessages();
            blas = PYRO_NULL_BLAS;
        }
        void D3DDevice::DestroyTlas(TlasId& tlas, bool bDefer) {
            if (bDefer) {
                ZombieDeleter zombie = {
                    .resource = eastl::bit_cast<void*>(tlas),
                    .deleter = [](D3DDevice* dev, void* res) { dev->DestroyImmediately(eastl::bit_cast<TlasId>(res)); }
                };
                mDeferredDeletes.EmplaceBack(eastl::move(SnapshotQueueFenceValues()), zombie);
                return;
            }

            mResourcePool->mSRVHeap.ReleaseSlot(tlas);
            gDx12Context->FlushDebugMessages();
            tlas = PYRO_NULL_TLAS;
        }
        eastl::optional<Format> D3DDevice::PickSupportedFormat(const eastl::span<Format>& candidates, FormatFeatureFlags features) const {
            return eastl::optional<Format>();
        }
        eastl::span<ICommandQueue*> D3DDevice::GetCommandQueues() {
            return mCommandQueueList;
        }
        ICommandQueue* D3DDevice::GetPresentQueue() {
            return mPrimaryCommandQueue;
        }
        void D3DDevice::WaitIdle() {
            for (ICommandQueue* q : GetCommandQueues()) {
                q->WaitIdle();
            }
            gDx12Context->FlushDebugMessages();
        }

        void D3DDevice::SubmitQueue(const CommandQueueSubmitInfo& info) {
            auto* q = static_cast<D3DCommandQueue*>(info.queue);
            for (auto [waitSemaphore, stage] : info.waitSemaphores) {
                D3DSemaphore* semaphore = eastl::bit_cast<D3DSemaphore*>(waitSemaphore);
                semaphore->Wait(q->InternalQueue());
                gDx12Context->FlushDebugMessages();
            }
            const UINT64 cpuFenceValue = q->IncGetCpuValue();
            // avoid race condition by adding this to the queue immediately before any execution!
            mQueuePendingSubmits.EmplaceBack(cpuFenceValue, q);

            eastl::vector<ID3D12CommandList*> commands;
            commands.reserve(info.commands.size());
            for (ICommandBuffer* c : info.commands) {
                commands.push_back(static_cast<D3DCommandBuffer*>(c)->GetCommands());
            }
            q->InternalQueue()->ExecuteCommandLists(static_cast<UINT>(commands.size()), commands.data());
            gDx12Context->FlushDebugMessages();

            // Fences for deferred resource destruction from incoming command buffers
            QueueFenceSnapshot fencesForThisFrame = SnapshotQueueFenceValues();

            gDx12Context->FlushDebugMessages();

            for (auto [signalSemaphore, stage] : info.signalSemaphores) {
                D3DSemaphore* semaphore = eastl::bit_cast<D3DSemaphore*>(signalSemaphore);
                semaphore->Signal(q->InternalQueue());
                gDx12Context->FlushDebugMessages();
            }

            // signal the given fences
            for (auto [fence, value] : info.signalFences) {
                auto* s = static_cast<D3DFence*>(fence);
                CheckD3DResult(q->InternalQueue()->Signal(s->InternalFence(), value),
                    "Failed to create signal ID3D12Fence");
                gDx12Context->FlushDebugMessages();
            }
            // Signal fence for the command buffer deleters
            q->SignalQueueFence(cpuFenceValue);

            // pool back the submitted command buffers for later reuse
            // and also add the zombies to the destroy queue
            for (ICommandBuffer* c : info.commands) {
                D3DCommandBuffer* cmb = static_cast<D3DCommandBuffer*>(c);
                q->RestoreCommandBuffer(cmb);

                for (i32 i = 0; i < cmb->mDeferredDeleteOps.size(); ++i) {
                    mDeferredDeletes.EmplaceBack(fencesForThisFrame, eastl::move(cmb->mDeferredDeleteOps[i]));
                }
                cmb->mDeferredDeleteOps.clear();
                for (i32 i = 0; i < cmb->mPendingReturnLinearUploadBuffers.size(); ++i) {
                    mDeferredDeletes.EmplaceBack(
                        fencesForThisFrame,
                        ZombieDeleter{
                            .resource = reinterpret_cast<void*>(cmb->mPendingReturnLinearUploadBuffers[i]),
                            .deleter = [](D3DDevice* device, void* resource) {
                                device->mAvailableLinearUploadBuffers.EmplaceBack(0ULL, reinterpret_cast<LinearUploadBuffer*>(resource));
                            },
                        });
                }
                cmb->mPendingReturnLinearUploadBuffers.clear();
            }
            gDx12Context->FlushDebugMessages();
        }
        void D3DDevice::PresentQueue(const CommandQueuePresentInfo& info) {
            for (ISwapChain* s : info.swapChains) {
                D3DSwapChain* swapChain = static_cast<D3DSwapChain*>(s);
                // TODO: how to get rid of the stupid dxgi delay?
                HRESULT hrTest = swapChain->InternalSwapChain()->Present(0, DXGI_PRESENT_TEST);
                if (hrTest == DXGI_STATUS_OCCLUDED) {
                    continue;
                }
                CheckD3DResult(hrTest, "Swapchain present failed!");
                HRESULT hr = swapChain->InternalSwapChain()->Present(swapChain->mSyncInterval, 0);
                CheckD3DResult(hr, "Swapchain present failed!");
                gDx12Context->FlushDebugMessages();
            }
        }
        const DeviceInfo& D3DDevice::Info() const {
            return mInfo;
        }
        const DevicePropertiesInfo& D3DDevice::Properties() const {
            return mProperties;
        }
        const DeviceFeaturesInfo& D3DDevice::Features() const {
            return mFeatures;
        }
        DeviceStatusInfo D3DDevice::Status() const {
            return DeviceStatusInfo{};
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

                        CheckD3DResult(mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&table.mHeap)),
                            "Failed to create image blit descriptor heap!");
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
                        GpuResourceId& descriptor = data.blitImageRTVs[subresource];
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
                        mDevice->CreateRenderTargetView(data.resource.Get(), &view, mResourcePool->mRTVHeap.Resolve(descriptor));
                    }
                }
            }
            gDx12Context->FlushDebugMessages();
        }

        DescriptorTableInfo D3DDevice::GetUnorderedAccessViewDescriptorTable(const UAVDescriptorTableCache& desc) {
            if (desc.EffectivelyEmpty()) {
                return mDefaultUAVDescriptorTable;
            }
            auto table = mUAVDescriptorTableCache.GetOpt(desc);
            if (table.has_value()) {
                mUAVDescriptorTableCache.Ref(desc, [](auto& table) {
                    table.first = 0;
                });
                return table->second;
            }
            Logger::Trace(gDX12Sink, "[D3D12] Copying descriptor tables");

            DescriptorTableInfo tableInfo{};
            D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
            heapDesc.NumDescriptors = NUM_CRV_SRV_UAV + Limits::MAX_UNORDERED_ACCESS_VIEW_SLOTS;
            heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

            CheckD3DResult(mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&tableInfo.mHeap)),
                "Failed to create SRV-UAV Descriptor Copy Heap!");
            D3DSetDebugName(tableInfo.mHeap, "SRV-UAV Descriptor Copy Heap");
            tableInfo.cpuDescriptor = tableInfo.mHeap->GetCPUDescriptorHandleForHeapStart();
            tableInfo.gpuDescriptor = tableInfo.mHeap->GetGPUDescriptorHandleForHeapStart();
            UINT incsz = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

            UINT cpySz = NUM_CRV_SRV_UAV;
            D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = mResourcePool->mSRVHeap.HostHandle();
            mDevice->CopyDescriptors(1, &tableInfo.cpuDescriptor, &cpySz,
                1, &srcHandle, &cpySz, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            gDx12Context->FlushDebugMessages();

            INT index = 0;
            for (UnorderedAccessId id : desc.boundUavs) {
                if (id != PYRO_NULL_UAV) {
                    CD3DX12_CPU_DESCRIPTOR_HANDLE currOffsetHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(tableInfo.cpuDescriptor);
                    D3D12_CPU_DESCRIPTOR_HANDLE handle = mResourcePool->mUAVHeap.Resolve(id);
                    mDevice->CopyDescriptorsSimple(1U, currOffsetHandle.Offset((NUM_CRV_SRV_UAV + index) * incsz), handle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                    gDx12Context->FlushDebugMessages();
                }
                ++index;
            }

            mUAVDescriptorTableCache.Emplace(desc, eastl::pair{ 0, tableInfo });
            return tableInfo;
        }

        ID3D12PipelineState* D3DDevice::GetBlitImagePipeline(DXGI_FORMAT format, bool bArray) {
            UINT64 hash = static_cast<UINT>(format) | (bArray ? 0x100000000ULL : 0x0000000ULL);

            auto pipelineOpt = mBlitImagePipelineStates.GetOpt(hash);
            if (pipelineOpt.has_value())
                return pipelineOpt->Get();


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


            Logger::Trace(gDX12Sink, "[D3D12] Creating image blit pipeline state object");
            ComPtr<ID3D12PipelineState> entry;
            // --- Create PSO ---
            CheckD3DResult(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&entry)), "Failed to create image blit pipeline state object!");
            mBlitImagePipelineStates.Emplace(hash, entry);
            eastl::string name = eastl::string("Blit image pipeline state DXGI format=") + eastl::to_string((UINT)format) + ", Array=" +
                                 (bArray ? "true" : "false");
            D3DSetDebugName(entry, name.c_str());
            gDx12Context->FlushDebugMessages();

            return entry.Get();
        }

        LinearUploadBuffer* D3DDevice::GetLinearBufferAllocation(UINT64 minSize) {
            if (mAvailableLinearUploadBuffers.Empty()) {
                auto lub = new LinearUploadBuffer(mDevice.Get(), minSize);
                gDx12Context->FlushDebugMessages();
                return lub;
            } else {
                LinearUploadBuffer* buff = mAvailableLinearUploadBuffers.PopBack().second;
                buff->Reset();
                gDx12Context->FlushDebugMessages();
                return buff;
            }
        }

        QueueFenceSnapshot D3DDevice::SnapshotQueueFenceValues() const {
            eastl::vector<u64> values = {};
            values.reserve(mCommandQueueList.size());
            for (ICommandQueue* q : mCommandQueueList) {
                auto* vkQueue = static_cast<D3DCommandQueue*>(q);
                values.emplace_back(vkQueue->GetCpuValue());
            }
            return values;
        }

        void D3DDevice::WriteAllSRVDescriptorHeapCopies(ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc, GpuResourceId handle) {
            UINT incSz = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            CD3DX12_CPU_DESCRIPTOR_HANDLE h = CD3DX12_CPU_DESCRIPTOR_HANDLE(mDefaultUAVDescriptorTable.cpuDescriptor);
            mDevice->CreateShaderResourceView(pResource, pDesc, h.Offset(handle.index * incSz));
            gDx12Context->FlushDebugMessages();
            mUAVDescriptorTableCache.ForEach([=, &h](const auto& cache, auto& data) {
                h = CD3DX12_CPU_DESCRIPTOR_HANDLE(data.second.cpuDescriptor);
                mDevice->CreateShaderResourceView(pResource, pDesc, h.Offset(handle.index * incSz));
                gDx12Context->FlushDebugMessages();
            });
        }
        void D3DDevice::DestroyAllUAVDescriptorHeapCopies(UnorderedAccessId handle) {
            eastl::vector<UAVDescriptorTableCache> destroyCaches = {};
            mUAVDescriptorTableCache.ForEach([&](const auto& cache, auto& data) {
                for (i32 i = 0; i < cache.boundUavs.size(); ++i) {
                    if (cache.boundUavs[i] == handle) {
                        destroyCaches.emplace_back(cache);
                    }
                }
            });
            for (const auto& cache : destroyCaches) {
                mUAVDescriptorTableCache.Erase(cache);
            }
        }

        void D3DDevice::FillBlasInputs(D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& inputs, const BlasBuildInfo& info, eastl::vector<D3D12_RAYTRACING_GEOMETRY_DESC>& scratchGeomDescs) const {
            inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
            inputs.Flags = ToD3D12ASBuildFlags(info.flags);
            if (info.bUpdate) {
                inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
            }

            if (auto* triangleGeos = eastl::get_if<eastl::span<const BlasTriangleGeometryInfo>>(&info.geometries)) {
                scratchGeomDescs.resize(triangleGeos->size());
                for (usize i = 0; i < triangleGeos->size(); ++i) {
                    const auto& geo = (*triangleGeos)[i];
                    auto& desc = scratchGeomDescs[i];
                    desc = {};
                    desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
                    desc.Flags = ToD3D12ASGeometryFlags(geo.flags);

                    desc.Triangles.VertexBuffer.StartAddress = mResourcePool->Get(geo.vertexBuffer).resource->GetGPUVirtualAddress() + geo.vertexByteOffset;
                    desc.Triangles.VertexBuffer.StrideInBytes = geo.vertexStride;
                    desc.Triangles.VertexFormat = ToDXGIFormat(geo.vertexFormat);
                    desc.Triangles.VertexCount = geo.vertexCount;

                    if (geo.indexType != IndexType::None) {
                        u32 indexSize = (geo.indexType == IndexType::Uint16) ? 2 : 4;
                        desc.Triangles.IndexBuffer = mResourcePool->Get(geo.indexBuffer).resource->GetGPUVirtualAddress() + (geo.indexOffset * indexSize);
                        desc.Triangles.IndexFormat = ToDXGIFormat(geo.indexType);
                        desc.Triangles.IndexCount = geo.indexCount;
                    } else {
                        desc.Triangles.IndexFormat = DXGI_FORMAT_UNKNOWN;
                    }

                    if (geo.transformData != PYRO_NULL_BUFFER) {
                        desc.Triangles.Transform3x4 = mResourcePool->Get(geo.transformData).resource->GetGPUVirtualAddress() + geo.transformDataOffset;
                    }
                }
                inputs.NumDescs = (UINT)scratchGeomDescs.size();
                inputs.pGeometryDescs = scratchGeomDescs.data();
            } else if (auto* aabbGeos = eastl::get_if<eastl::span<const BlasAabbGeometryInfo>>(&info.geometries)) {
                scratchGeomDescs.resize(aabbGeos->size());
                for (usize i = 0; i < aabbGeos->size(); ++i) {
                    const auto& geo = (*aabbGeos)[i];
                    auto& desc = scratchGeomDescs[i];
                    desc = {};
                    desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
                    desc.Flags = ToD3D12ASGeometryFlags(geo.flags);
                    desc.AABBs.AABBs.StartAddress = mResourcePool->Get(geo.data).resource->GetGPUVirtualAddress();
                    desc.AABBs.AABBs.StrideInBytes = geo.stride;
                    desc.AABBs.AABBCount = geo.count;
                }
                inputs.NumDescs = (UINT)scratchGeomDescs.size();
                inputs.pGeometryDescs = scratchGeomDescs.data();
            }
        }

        // See RHIVulkan for explanation, as this is very similar
        bool D3DDevice::CollectGarbage() {
            std::unique_lock glock(mGlobalQueueLock);
            for (ICommandQueue* commandQueue : mCommandQueueList) {
                if (static_cast<D3DCommandQueue*>(commandQueue)->HasOpenCommands()) {
                    return false;
                }
            }

            eastl::vector<eastl::pair<D3DCommandQueue*, u64>> oldestQueueTimelines = {};
            oldestQueueTimelines.reserve(mCommandQueueList.size());
            for (ICommandQueue* commandQueue : mCommandQueueList) {
                oldestQueueTimelines.emplace_back(static_cast<D3DCommandQueue*>(commandQueue), UINT64_MAX);
            }
            {
                std::lock_guard l(mQueuePendingSubmits.GetLock());
                auto& vec = mQueuePendingSubmits.UnderlyingVector();
                for (usize i = 0; i < vec.size(); ++i) {
                    auto [queueTimeline, queue] = vec[i];
                    u64 completedValue = queue->GetFenceValue();
                    if (completedValue >= queueTimeline) {
                        vec.erase(vec.begin() + i);
                        --i;
                    } else {
                        for (auto& [oldestTimelineQueue, oldestVal] : oldestQueueTimelines) {
                            if (queue != oldestTimelineQueue)
                                continue;
                            oldestVal = eastl::min(oldestVal, completedValue);
                        }
                    }
                }
            }
            {
                std::lock_guard l(mDeferredDeletes.GetLock());
                auto& vec = mDeferredDeletes.UnderlyingVector();
                for (usize i = 0; i < vec.size(); ++i) {
                    auto [deleteTimelineSnapshot, zombie] = vec[i];

                    bool bReady = true;
                    // Check across all queues to see what is pending.
                    // Queues that are not participating will just have a value of UINT64_MAX and naturally skip the check
                    for (usize j = 0; j < oldestQueueTimelines.size(); ++j) {
                        auto [queue, completedTimeline] = oldestQueueTimelines[j];
                        // resources should NEVER catch up to the main cpu timeline,
                        // otherwise they were scheduled for destruction after a final queue submission (BAD)
                        // So a "less than equal" condition is necessary to determine if this is not ready.
                        if (completedTimeline <= deleteTimelineSnapshot[j]) {
                            bReady = false;
                            break;
                        }
                    }
                    if (bReady) {
                        zombie.deleter(this, zombie.resource); // destroy
                        gDx12Context->FlushDebugMessages();
                        vec.erase(vec.begin() + i);
                        --i;
                    }
                }
            }

            // === Miscellaneous garbage collection ===

            // Delete linear upload buffers that haven't been used in a long time
            {
                std::lock_guard l(mAvailableLinearUploadBuffers.GetLock());
                auto& vec = mAvailableLinearUploadBuffers.UnderlyingVector();
                for (usize i = 0; i < vec.size(); ++i) {
                    auto& [unusedFrames, zombie] = vec[i];
                    if (unusedFrames++ > MAX_FRAMES_LINEAR_UPLOAD_BUFFER_UNSUED_LIFETIME) {
                        delete zombie;
                        gDx12Context->FlushDebugMessages();
                        vec.erase(vec.begin() + i);
                        --i;
                    }
                }
            }

            // cleanup UAV tables that haven't been used in a long time
            eastl::vector<UAVDescriptorTableCache> deleteUAVTableCacheHandles = {};
            mUAVDescriptorTableCache.ForEach([&](const auto& cache, auto& data) mutable {
                if (data.first++ > MAX_FRAMES_UAV_TABLE_CACHE_UNUSED_LIFETIME) {
                    deleteUAVTableCacheHandles.emplace_back(cache);
                }
            });
            for (auto handle : deleteUAVTableCacheHandles) {
                mUAVDescriptorTableCache.Erase(handle);
            }
            return true;
        }

        u32 D3DDevice::GetActiveShaderModel() const {
            return mActiveShaderModel;
        }

        void D3DDevice::SetShaderModel(u32 shaderModel) {
            ASSERT(shaderModel <= mFeatures.maxSupportedShaderModel, "Shader model used is unsupported!");
            mActiveShaderModel = shaderModel;
        }

        void D3DDevice::CheckFeatureSupport() {
            Logger::Debug(gDX12Sink, "Checking D3D12 Feature support");
            mDx12FeatureSupport.Init(mDevice.Get());
            gDx12Context->FlushDebugMessages();

            if (mDx12FeatureSupport.ResourceBindingTier() < 2) {
                Logger::Fatal(gDX12Sink, "Insufficient resource binding tier! Resource binding tier 2 or higher is required!");
            }
            if (mDx12FeatureSupport.HighestShaderModel() < D3D_SHADER_MODEL_5_1) {
                Logger::Fatal(gDX12Sink, "Insufficient shader model support! Shader model 5.1 or higher is required!");
            }
        }

        void D3DDevice::CreateCommandQueues() {
            Logger::Debug(gDX12Sink, "Querying Command Queues...");
            eastl::array<D3D12_COMMAND_QUEUE_DESC, 3> queueDescs = {
                D3D12_COMMAND_QUEUE_DESC{
                    .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
                    .Priority = 0,
                    .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
                },
                D3D12_COMMAND_QUEUE_DESC{
                    .Type = D3D12_COMMAND_LIST_TYPE_COMPUTE,
                    .Priority = 0,
                    .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
                },
                D3D12_COMMAND_QUEUE_DESC{
                    .Type = D3D12_COMMAND_LIST_TYPE_COPY,
                    .Priority = 0,
                    .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
                },
            };
            for (auto& queueDesc : queueDescs) {
                ComPtr<ID3D12CommandQueue> commandQueue;
                CheckD3DResult(mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)), "Failed to create command queue!");

                CommandQueueInfo queueDescData;
                switch (queueDesc.Type) {
                case D3D12_COMMAND_LIST_TYPE_DIRECT:
                    queueDescData.flags = CommandQueueFlagBits::GRAPHICS |
                                          CommandQueueFlagBits::COMPUTE |
                                          CommandQueueFlagBits::TRANSFER;
                    queueDescData.bPresentable = true;
                    queueDescData.name = "Direct Queue";
                    break;

                case D3D12_COMMAND_LIST_TYPE_COMPUTE:
                    queueDescData.flags = CommandQueueFlagBits::COMPUTE |
                                          CommandQueueFlagBits::TRANSFER;
                    queueDescData.bPresentable = false;
                    queueDescData.name = "Compute Queue";
                    break;

                case D3D12_COMMAND_LIST_TYPE_COPY:
                    queueDescData.flags = CommandQueueFlagBits::TRANSFER;
                    queueDescData.bPresentable = false;
                    queueDescData.name = "Copy Queue";
                    break;
                }
                auto* queue = new D3DCommandQueue(
                    this, eastl::move(queueDescData),
                    eastl::move(commandQueue));
                if (queueDesc.Type == D3D12_COMMAND_LIST_TYPE_DIRECT) {
                    mPrimaryCommandQueue = queue;
                }
                mCommandQueueList.emplace_back(static_cast<ICommandQueue*>(queue));
                gDx12Context->FlushDebugMessages();
            }
            Logger::Trace(gDX12Sink, "Created {} Command Queues", queueDescs.size());
        }

        void D3DDevice::InitializeResourcePool() {
            Logger::Debug(gDX12Sink, "Initialising Resource Pool...");
            mResourcePool = eastl::make_unique<GPUResourcePool>(this, 2048, 2048, NUM_CRV_SRV_UAV, NUM_CRV_SRV_UAV, NUM_SAMPLERS);
            gDx12Context->FlushDebugMessages();
        }

        void D3DDevice::CreateDescriptorHeaps() { // We need this device visible heap for SRVs and UAVs because of the binding model we are using.
            Logger::Debug(gDX12Sink, "Creating Base UAV copy descriptor heap...");
            {
                D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
                heapDesc.NumDescriptors = NUM_CRV_SRV_UAV + Limits::MAX_UNORDERED_ACCESS_VIEW_SLOTS;
                heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

                CheckD3DResult(mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mDefaultUAVDescriptorTable.mHeap)), "Failed to create global descriptor heap!");
                mDefaultUAVDescriptorTable.cpuDescriptor = mDefaultUAVDescriptorTable.mHeap->GetCPUDescriptorHandleForHeapStart();
                mDefaultUAVDescriptorTable.gpuDescriptor = mDefaultUAVDescriptorTable.mHeap->GetGPUDescriptorHandleForHeapStart();
                D3DSetDebugName(mDefaultUAVDescriptorTable.mHeap, "Default SRV-UAV Descriptor Heap");
                gDx12Context->FlushDebugMessages();
            }
        }

        void D3DDevice::CreateGlobalRootSignature() { // create global root signature
            Logger::Debug(gDX12Sink, "Creating Global Root Signature...");
            {
                CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
                CD3DX12_ROOT_PARAMETER rootparams[18]{};
                // push constants
                // 32 * 1 = 32 DWORDS
                rootparams[0].InitAsConstants(32, RHIDX12_ROOT_CONSTANT_CBV_REGISTER);
                // "specialisation constants"
                // because dx12 doesn't support them, we have to emulate them through these views
                // 5 * 2 = 10 DWORDS
                rootparams[1].InitAsConstantBufferView(RHIDX12_EMU_SPECCONST_CBV_VS_REGISTER);
                rootparams[2].InitAsConstantBufferView(RHIDX12_EMU_SPECCONST_CBV_HS_REGISTER);
                rootparams[3].InitAsConstantBufferView(RHIDX12_EMU_SPECCONST_CBV_DS_REGISTER);
                rootparams[4].InitAsConstantBufferView(RHIDX12_EMU_SPECCONST_CBV_GS_REGISTER);
                rootparams[5].InitAsConstantBufferView(RHIDX12_EMU_SPECCONST_CBV_PS_REGISTER);
                // finally, our uniform buffers
                // 8 * 2 = 16 DWORDS
                rootparams[6].InitAsConstantBufferView(RHIDX12_FIXED_CBV_REGISTER_0);
                rootparams[7].InitAsConstantBufferView(RHIDX12_FIXED_CBV_REGISTER_1);
                rootparams[8].InitAsConstantBufferView(RHIDX12_FIXED_CBV_REGISTER_2);
                rootparams[9].InitAsConstantBufferView(RHIDX12_FIXED_CBV_REGISTER_3);
                rootparams[10].InitAsConstantBufferView(RHIDX12_FIXED_CBV_REGISTER_4);
                rootparams[11].InitAsConstantBufferView(RHIDX12_FIXED_CBV_REGISTER_5);
                rootparams[12].InitAsConstantBufferView(RHIDX12_FIXED_CBV_REGISTER_6);
                rootparams[13].InitAsConstantBufferView(RHIDX12_FIXED_CBV_REGISTER_7);

                const UINT NUM_BINDLESS_SRV_SPACES = 9;

                CD3DX12_DESCRIPTOR_RANGE srvDescriptorRanges[NUM_BINDLESS_SRV_SPACES]{};

                for (UINT i = 0; i < NUM_BINDLESS_SRV_SPACES; ++i) {
                    srvDescriptorRanges[i].Init(
                        D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                        NUM_CRV_SRV_UAV,
                        RHIDX12_SRV_REGISTER,                  // t0
                        RHIDX12_BINDLESS_DESCRIPTOR_SPACE + i, // space1, space2, space3...
                        0                                      // Force offset to 0 so they all alias the same heap memory
                    );
                }

                const UINT NUM_BINDLESS_SAMPLER_SPACES = 2;
                CD3DX12_DESCRIPTOR_RANGE samplerDescriptorRanges[NUM_BINDLESS_SAMPLER_SPACES]{};
                samplerDescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, NUM_SAMPLERS, RHIDX12_SAMPLER_STATE_REGISTER, 30, 0); // space30
                samplerDescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, NUM_SAMPLERS, RHIDX12_SAMPLER_STATE_REGISTER, 31, 0); // space31

                CD3DX12_DESCRIPTOR_RANGE uavDescriptorRange{};
                uavDescriptorRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, Limits::MAX_UNORDERED_ACCESS_VIEW_SLOTS, 0, 1, NUM_CRV_SRV_UAV);

                rootparams[14].InitAsDescriptorTable(NUM_BINDLESS_SRV_SPACES, srvDescriptorRanges);
                rootparams[15].InitAsDescriptorTable(NUM_BINDLESS_SAMPLER_SPACES, samplerDescriptorRanges);
                rootparams[16].InitAsDescriptorTable(1, &uavDescriptorRange);

                // DrawID constant
                // 3 * 1 = 3 DWORDS
                // SV_DrawIndex
                rootparams[17].InitAsConstants(1, RHIDX12_EMU_DRAWINDEX_CBV_VS_REGISTER, RHIDX12_EMU_DRAWINDEX_CBV_VS_DESCRIPTOR_SPACE, D3D12_SHADER_VISIBILITY_VERTEX);

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
                CheckD3DResult(hr, "Failed to D3D12SerializeRootSignature");
                CheckD3DResult(mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)),
                    "Failed to create global root signature!");
                D3DSetDebugName(mRootSignature, "Default Root Signature");
                gDx12Context->FlushDebugMessages();
            }
        }

        void D3DDevice::CreateBlitImageRootSignature() {
            // create blit image root signature
            Logger::Debug(gDX12Sink, "Creating Image Blit Root Signature...");
            {
                CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
                CD3DX12_ROOT_PARAMETER rootparams[3]{};
                // Blit coordinates
                rootparams[0].InitAsConstants(4 * sizeof(eastl::array<f32, 2>) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
                // Blit image srv
                CD3DX12_DESCRIPTOR_RANGE srvDescriptorRange{};
                srvDescriptorRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, RHIDX12_EMU_BLITIMAGE_SRV_REGISTER, RHIDX12_EMU_BLITIMAGE_DESCRIPTOR_SPACE);
                rootparams[1].InitAsDescriptorTable(1, &srvDescriptorRange, D3D12_SHADER_VISIBILITY_PIXEL);
                // Blit image sampler
                CD3DX12_DESCRIPTOR_RANGE samplerDescriptorRange{};
                samplerDescriptorRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, RHIDX12_EMU_BLITIMAGE_SAMPLER_STATE_REGISTER, RHIDX12_EMU_BLITIMAGE_DESCRIPTOR_SPACE);
                rootparams[2].InitAsDescriptorTable(1, &samplerDescriptorRange, D3D12_SHADER_VISIBILITY_PIXEL);

                const D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
                rootSignatureDesc.Init(PYRO_ARRAY_SIZE(rootparams), rootparams, 0, nullptr, flags);
                ComPtr<ID3DBlob> signature;
                ComPtr<ID3DBlob> error;
                HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, &error);
                if (error && error->GetBufferPointer()) {
                    OutputDebugStringA((const char*)error->GetBufferPointer());
                }
                CheckD3DResult(hr, "Failed to D3D12SerializeRootSignature for image blit root signature");
                CheckD3DResult(mDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mBlitImageRootSignature)), "Failed to create image blit root signature");
                D3DSetDebugName(mBlitImageRootSignature, "Blit Image Root Signature");
                gDx12Context->FlushDebugMessages();
            }
        }

        void D3DDevice::CreateDefaultSamplerHeaps() { // Finally, create the sampler heap objects
            Logger::Debug(gDX12Sink, "Creating Default Sampler Heap Objects...");
            {
                D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
                heapDesc.NumDescriptors = 1;
                heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
                heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

                CheckD3DResult(mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mNearestSamplerDescriptorTable.mHeap)),
                    "Failed to create default NN sampler heap");
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
                samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NONE;

                mDevice->CreateSampler(&samplerDesc, mNearestSamplerDescriptorTable.cpuDescriptor);

                CheckD3DResult(mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mLinearSamplerDescriptorTable.mHeap)),
                    "Failed to create default linear sampler heap");
                mLinearSamplerDescriptorTable.cpuDescriptor = mLinearSamplerDescriptorTable.mHeap->GetCPUDescriptorHandleForHeapStart();
                mLinearSamplerDescriptorTable.gpuDescriptor = mLinearSamplerDescriptorTable.mHeap->GetGPUDescriptorHandleForHeapStart();
                D3DSetDebugName(mLinearSamplerDescriptorTable.mHeap, "Blit Image Linear Sampler Descriptor Heap");

                samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
                mDevice->CreateSampler(&samplerDesc, mLinearSamplerDescriptorTable.cpuDescriptor);
                gDx12Context->FlushDebugMessages();
            }
        }

        void D3DDevice::CreateCommandSignatures() {
            Logger::Debug(gDX12Sink, "Creating Default Command Signatures...");
            {
                D3D12_INDIRECT_ARGUMENT_DESC argDesc;
                D3D12_COMMAND_SIGNATURE_DESC cmdSigDesc = {};
                cmdSigDesc.NodeMask = 0;
                cmdSigDesc.NumArgumentDescs = 1;
                cmdSigDesc.pArgumentDescs = &argDesc;

                argDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
                cmdSigDesc.ByteStride = sizeof(DrawArgumentBuffer);
                CheckD3DResult(mDevice->CreateCommandSignature(&cmdSigDesc, nullptr, IID_PPV_ARGS(&mIndirectDrawSignature)),
                    "Failed to create indirect draw command signature!");
                argDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
                cmdSigDesc.ByteStride = sizeof(DrawIndexedArgumentBuffer);
                CheckD3DResult(mDevice->CreateCommandSignature(&cmdSigDesc, nullptr, IID_PPV_ARGS(&mIndirectDrawIndexedSignature)),
                    "Failed to create indirect draw indexed command signature!");
                argDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
                cmdSigDesc.ByteStride = sizeof(DispatchArgumentBuffer);
                CheckD3DResult(mDevice->CreateCommandSignature(&cmdSigDesc, nullptr, IID_PPV_ARGS(&mIndirectDispatchSignature)),
                    "Failed to create indirect dispatch command signature!");
                gDx12Context->FlushDebugMessages();
            }
        }

        void D3DDevice::CreateMemoryAllocator() {
            Logger::Debug(gDX12Sink, "Creating D3D12MA Allocator...");
            D3D12MA::ALLOCATOR_DESC allocatorDesc{};
            allocatorDesc.Flags = D3D12MA::ALLOCATOR_FLAG_NONE;
            allocatorDesc.pDevice = mDevice.Get();
            allocatorDesc.pAllocationCallbacks = nullptr;
            allocatorDesc.pAdapter = mAdapter.Get();
            CheckD3DResult(D3D12MA::CreateAllocator(&allocatorDesc, mAllocator.GetAddressOf()),
                "Failed to create D3D12MA allocator!");
            gDx12Context->FlushDebugMessages();
        }
        void D3DDevice::PopulateDeviceInfo() {
            DXGI_ADAPTER_DESC1 desc = {};
            mAdapter->GetDesc1(&desc);

            mInfo.name = WideToUTF8(desc.Description);
            mInfo.vendorID = desc.VendorId;
            mInfo.deviceID = desc.DeviceId;
            mInfo.subsystemID = desc.SubSysId;
            mInfo.revisionID = desc.Revision;

            mInfo.dedicatedVideoMemory = desc.DedicatedVideoMemory;
            mInfo.sharedSystemMemory = desc.SharedSystemMemory;
            if (desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE) {
                mInfo.deviceType = DeviceType::CPU;
            } else {
                mInfo.deviceType = DeviceType::Unknown;
            }
            mInfo.bUnifiedMemory = mDx12FeatureSupport.UMA();
            mInfo.bRemovable = false;
            mInfo.bPrimaryAdapter = !!(desc.Flags & DXGI_ADAPTER_FLAG3_ACG_COMPATIBLE);
            mInfo.adapterLUIDLow = desc.AdapterLuid.LowPart;
            mInfo.adapterLUIDHigh = desc.AdapterLuid.HighPart;

            switch (desc.VendorId) {
            case 0x10DE:
                mInfo.vendor = "NVIDIA";
                break;
            case 0x1002:
            case 0x1022:
                mInfo.vendor = "AMD";
                break;
            case 0x8086:
                mInfo.vendor = "Intel";
                break;
            case 0x13B5:
                mInfo.vendor = "ARM";
                break;
            case 0x5143:
                mInfo.vendor = "Qualcomm";
                break;
            case 0x1414:
                mInfo.vendor = "Microsoft";
                break;
            default:
                mInfo.vendor = "Unknown";
                break;
            }
            // API + driver info
            mInfo.apiVersion = "D3D12 (11.0)";

            LARGE_INTEGER umdVersion = {};
            HRESULT hr = mAdapter->CheckInterfaceSupport(__uuidof(IDXGIDevice), &umdVersion);
            if (SUCCEEDED(hr)) {
                u64 q = static_cast<u64>(umdVersion.QuadPart);
                unsigned major = static_cast<unsigned>((q >> 48) & 0xFFFF);
                unsigned minor = static_cast<unsigned>((q >> 32) & 0xFFFF);
                unsigned sub = static_cast<unsigned>((q >> 16) & 0xFFFF);
                unsigned build = static_cast<unsigned>(q & 0xFFFF);

                mInfo.driverVersion.sprintf("%u.%u.%u.%u", major, minor, sub, build);
            }
        }
        void D3DDevice::PopulateDeviceProperties() {
            CD3DX12FeatureSupport support;
            CheckD3DResult(support.Init(mDevice.Get()),
                "Failed to get DX12 device feature support!");
            mProperties.minStorageBufferOffsetAlignment = D3D12_RAW_UAV_SRV_BYTE_ALIGNMENT;
            mProperties.minUniformBufferOffsetAlignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
            mProperties.bufferImageCopyOffsetAlignment = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT;
            mProperties.bufferImageRowAlignment = D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;

            mProperties.bHasDedicatedComputeQueue = true;
            mProperties.bHasDedicatedTransferQueue = true;

            mProperties.graphicsQueueCount = 1;
            mProperties.computeQueueCount = 1;
            mProperties.transferQueueCount = 1;

            mProperties.maxTextureWidth = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
            mProperties.maxTextureHeight = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
            mProperties.maxTextureDepth = D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
            mProperties.maxTextureArrayLayers = std::min(D3D12_REQ_TEXTURE1D_ARRAY_AXIS_DIMENSION, D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION);
            mProperties.maxSamplerAnisotropy = D3D12_MAX_MAXANISOTROPY;

            mProperties.minLineWidth = 1.0f;
            mProperties.maxLineWidth = 1.0f;

            // FIXME: this is probably not a good way to deal with it, but most GPUs supporting dx12 support this mask
            constexpr RasterizationSamples typicalMSAASupport = static_cast<RasterizationSamples>(15);
            mProperties.msaaSupportColorTarget = typicalMSAASupport;
            mProperties.msaaSupportDepthStencilTarget = typicalMSAASupport;
            mProperties.msaaSupportShaderResourceView = typicalMSAASupport;
            mProperties.msaaSupportUnorderedAccessView = typicalMSAASupport;
        }
        void D3DDevice::PopulateDeviceFeatures() {
            mFeatures.bGeometryShaders = true; // Always supported
            mFeatures.bTesselationShaders = true;
            mFeatures.bUint8IndexBuffer = false;
            mFeatures.bBCnTextureCompression = true;
            mFeatures.bHeadlessSwapChainWindow = true;
            mFeatures.bConservativeRasterization = mDx12FeatureSupport.ConservativeRasterizationTier() != D3D12_CONSERVATIVE_RASTERIZATION_TIER_NOT_SUPPORTED;
            mFeatures.bInt64ShaderOps = mDx12FeatureSupport.Int64ShaderOps();
            mFeatures.bWaveOps = mDx12FeatureSupport.WaveOps();

            mFeatures.bAccelerationStructureBuild = mDx12FeatureSupport.RaytracingTier() != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
            mFeatures.bRayTracingPipelines = mDx12FeatureSupport.RaytracingTier() >= D3D12_RAYTRACING_TIER_1_0;
            mFeatures.bRayQueries = mDx12FeatureSupport.RaytracingTier() >= D3D12_RAYTRACING_TIER_1_1;
            mFeatures.bMeshShaders = mDx12FeatureSupport.MeshShaderTier() != D3D12_MESH_SHADER_TIER_NOT_SUPPORTED;
            mFeatures.bVariableRateShading = mDx12FeatureSupport.VariableShadingRateTier() != D3D12_VARIABLE_SHADING_RATE_TIER_NOT_SUPPORTED;
            mFeatures.maxSupportedShaderModel = static_cast<u32>(mDx12FeatureSupport.HighestShaderModel());
        }


        void D3DDevice::DestroyCommandQueues() {
            for (auto* queue : mCommandQueueList) {
                delete static_cast<D3DCommandQueue*>(queue);
                gDx12Context->FlushDebugMessages();
            }
        }

        void D3DDevice::DestroyUploadBuffers() {
            // ASSERT(mOccupiedLinearUploadBuffers.empty(), "Command buffers must finish execution before device destruction! Linear upload buffers were leaked!");
            mAvailableLinearUploadBuffers.ForEach([](const auto& kv) {
                delete kv.second;
                gDx12Context->FlushDebugMessages();
            });
        }

        void D3DDevice::ReportDeviceRemovalReason() {
            HRESULT reason = mDevice->GetDeviceRemovedReason();
            if (reason != S_OK) {
                _com_error err = _com_error(reason);
                LPCTSTR desc = static_cast<LPCTSTR>(err.Description());
                Logger::Error(gDX12Sink, "DEVICE REMOVED: {} \"{}\"", err.ErrorMessage(), desc ? desc : "");
            }
        }

    } // namespace RHIDX12
} // namespace PyroshockStudios
