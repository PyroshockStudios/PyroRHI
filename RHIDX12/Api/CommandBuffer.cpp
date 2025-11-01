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

#include "CommandBuffer.hpp"
#include "Device.hpp"
#include "GPUResource.hpp"
#include "Pipeline.hpp"
#include "QueryPool.hpp"
#include "RenderTarget.hpp"
#include <RHIDX12/D3DContext.hpp>

#include <DirectXMath.h>
#include <comdef.h>
#include <libassert/assert.hpp>

namespace PyroshockStudios {
    namespace RHIDX12 {

        D3DCommandBuffer::D3DCommandBuffer(D3DDevice* device, ComPtr<ID3D12GraphicsCommandList>&& commandList, ComPtr<ID3D12CommandAllocator>&& allocator)
            : mDevice(device), mCommandList(eastl::move(commandList)), mAllocator(eastl::move(allocator)) {
            mCommandList->QueryInterface(IID_PPV_ARGS(&mCommandList4));
        }
        D3DCommandBuffer::~D3DCommandBuffer() {
        }

        void D3DCommandBuffer::CopyBufferToBuffer(const CopyBufferToBufferInfo& info) {
            const auto& src = mDevice->ResourcePool().Get(info.srcBuffer);
            const auto& dst = mDevice->ResourcePool().Get(info.dstBuffer);
            mCommandList->CopyBufferRegion(dst.resource.Get(), info.dstOffset, src.resource.Get(), info.srcOffset, info.size);
            gDx12Context->FlushDebugMessages();
        }

        void D3DCommandBuffer::CopyBufferToImage(const CopyBufferToImageInfo& info) {
            const auto& src = mDevice->ResourcePool().Get(info.buffer);
            const auto& dst = mDevice->ResourcePool().Get(info.image);

            for (UINT j = 0; j < PYRO_IMAGE_SLICE_RESOLVE_LAYERS(info.imageSlice, dst.info.arrayLayerCount); ++j) {
                UINT dstSubresource = D3D12CalcSubresource(info.imageSlice.mipLevel, info.imageSlice.baseArrayLayer + j, 0, dst.info.mipLevelCount, dst.info.arrayLayerCount);
                D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
                UINT numRows = {};
                UINT64 rowSizesInBytes = {};
                UINT64 requiredSize = {};
                mDevice->InternalDevice()->GetCopyableFootprints(&dst.desc, dstSubresource, 1, info.bufferOffset,
                    &footprint, &numRows, &rowSizesInBytes, &requiredSize);
                ASSERT(PYRO_VERIFY_ALIGNMENT(info.rowPitch, rowSizesInBytes), "Row Pitch MUST be aligned to device requirements!");
                footprint.Footprint.RowPitch = info.rowPitch;
                footprint.Footprint.Width = info.imageExtent.width;
                footprint.Footprint.Height = info.imageExtent.height;
                footprint.Footprint.Depth = info.imageExtent.depth;
                CD3DX12_TEXTURE_COPY_LOCATION Dst(dst.resource.Get(), dstSubresource);
                CD3DX12_TEXTURE_COPY_LOCATION Src(src.resource.Get(), footprint);
                mCommandList->CopyTextureRegion(&Dst, info.imageOffset.x, info.imageOffset.y, info.imageOffset.z, &Src, nullptr);
            }
            gDx12Context->FlushDebugMessages();
        }

        void D3DCommandBuffer::CopyImageToBuffer(const CopyImageToBufferInfo& info) {
            const auto& src = mDevice->ResourcePool().Get(info.image);
            const auto& dst = mDevice->ResourcePool().Get(info.buffer);

            for (UINT j = 0; j < PYRO_IMAGE_SLICE_RESOLVE_LAYERS(info.imageSlice, src.info.arrayLayerCount); ++j) {
                UINT srcSubresource = D3D12CalcSubresource(
                    info.imageSlice.mipLevel,
                    info.imageSlice.baseArrayLayer + j,
                    0,
                    src.info.mipLevelCount,
                    src.info.arrayLayerCount);

                D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
                UINT numRows = 0;
                UINT64 rowSizeInBytes = 0;
                UINT64 requiredSize = 0;

                // Use bufferOffset as base so footprint.Offset includes it
                mDevice->InternalDevice()->GetCopyableFootprints(
                    &src.desc,
                    srcSubresource,
                    1,
                    info.bufferOffset, // base offset into destination buffer
                    &footprint,
                    &numRows,
                    &rowSizeInBytes,
                    &requiredSize);

                ASSERT(PYRO_VERIFY_ALIGNMENT(info.rowPitch, rowSizeInBytes),
                    "Row Pitch MUST be aligned to device requirements!");

                // Update the footprint to reflect the desired copy region
                footprint.Offset = info.bufferOffset;
                footprint.Footprint.RowPitch = info.rowPitch;
                footprint.Footprint.Width = info.imageExtent.width;
                footprint.Footprint.Height = info.imageExtent.height;
                footprint.Footprint.Depth = info.imageExtent.depth;

                // Destination is the buffer footprint, source is the image
                CD3DX12_TEXTURE_COPY_LOCATION Dst(dst.resource.Get(), footprint);
                CD3DX12_TEXTURE_COPY_LOCATION Src(src.resource.Get(), srcSubresource);

                auto srcBox = CD3DX12_BOX(
                    info.imageOffset.x,
                    info.imageOffset.y,
                    info.imageOffset.z,
                    info.imageOffset.x + info.imageExtent.width,
                    info.imageOffset.y + info.imageExtent.height,
                    info.imageOffset.z + info.imageExtent.depth);
                // Copy from the specified offset within the image
                mCommandList->CopyTextureRegion(
                    &Dst,
                    0, 0, 0,
                    &Src,
                    reinterpret_cast<const D3D12_BOX*>(&srcBox));
            }

            gDx12Context->FlushDebugMessages();
        }


        void D3DCommandBuffer::CopyImageToImage(const CopyImageToImageInfo& info) {
            const auto& src = mDevice->ResourcePool().Get(info.srcImage);
            const auto& dst = mDevice->ResourcePool().Get(info.dstImage);
            for (UINT j = 0; j < PYRO_IMAGE_SLICE_RESOLVE_LAYERS(info.srcImageSlice, src.info.arrayLayerCount); ++j) {
                UINT srcSubresource = D3D12CalcSubresource(info.srcImageSlice.mipLevel, info.srcImageSlice.baseArrayLayer + j, 0, src.info.mipLevelCount, src.info.arrayLayerCount);
                UINT dstSubresource = D3D12CalcSubresource(info.dstImageSlice.mipLevel, info.dstImageSlice.baseArrayLayer + j, 0, dst.info.mipLevelCount, dst.info.arrayLayerCount);
                auto srcCpy = CD3DX12_TEXTURE_COPY_LOCATION(src.resource.Get(), srcSubresource);
                auto dstCpy = CD3DX12_TEXTURE_COPY_LOCATION(dst.resource.Get(), dstSubresource);
                D3D12_BOX srcBox = ToD3D12Box(Box3D::Cut(info.extent, info.srcOffset));
                mCommandList->CopyTextureRegion(&dstCpy, (UINT)info.dstOffset.x, (UINT)info.dstOffset.y, (UINT)info.dstOffset.z, &srcCpy, &srcBox);
            }
            gDx12Context->FlushDebugMessages();
        }

        void D3DCommandBuffer::BlitImageToImage(const BlitImageToImageInfo& info) {
            auto& srcImage = mDevice->ResourcePool().Get(info.srcImage);
            auto& dstImage = mDevice->ResourcePool().Get(info.dstImage);

            ID3D12PipelineState* pipeline = mDevice->GetBlitImagePipeline(ToDXGIFormat(dstImage.info.format), dstImage.info.arrayLayerCount > 1);
            mCommandList->SetPipelineState(pipeline);
            mCommandList->SetGraphicsRootSignature(mDevice->mBlitImageRootSignature.Get());

            D3D12_VIEWPORT viewport{};
            viewport.TopLeftX = 0.0f;
            viewport.TopLeftY = 0.0f;
            viewport.Width = static_cast<float>(dstImage.info.size.width);
            viewport.Height = static_cast<float>(dstImage.info.size.height);
            viewport.MinDepth = 0.0f;
            viewport.MaxDepth = 1.0f;
            mCommandList->RSSetViewports(1, &viewport);

            D3D12_RECT scissorRect{};
            scissorRect.left = info.dstImageBox.x;
            // origin is top left in the rect... go figure lol
            scissorRect.top = static_cast<i32>(dstImage.info.size.height) - info.dstImageBox.height - info.dstImageBox.y;
            scissorRect.right = info.dstImageBox.x + info.dstImageBox.width;
            scissorRect.bottom = static_cast<i32>(dstImage.info.size.height) - info.dstImageBox.y;
            if (scissorRect.left > scissorRect.right) {
                eastl::swap(scissorRect.left, scissorRect.right);
            }
            if (scissorRect.bottom < scissorRect.top) {
                eastl::swap(scissorRect.bottom, scissorRect.top);
            }
            mCommandList->RSSetScissorRects(1, &scissorRect);

            // Build push constants (src UVs and dst NDC). Handle negative src rects (flips).
            struct PushConstants {
                DirectX::XMFLOAT2 srcLower;
                DirectX::XMFLOAT2 srcUpper;
                DirectX::XMFLOAT2 dstLower;
                DirectX::XMFLOAT2 dstUpper;
            } pushConstants;

            // Compute src UVs from pixel rect -> [0,1] space
            float srcW = static_cast<float>(srcImage.info.size.width);
            float srcH = static_cast<float>(srcImage.info.size.height);

            float srcX0 = static_cast<float>(info.srcImageBox.x);
            // NOTE: UV sampling is vulkan coordinate style, so origin is TOP LEFT!
            float srcY0 = static_cast<float>(info.srcImageBox.y + info.srcImageBox.height);
            float srcX1 = static_cast<float>(info.srcImageBox.x + info.srcImageBox.width);
            float srcY1 = static_cast<float>(info.srcImageBox.y);

            float u0 = srcX0 / srcW;
            float v0 = srcY0 / srcH;
            float u1 = srcX1 / srcW;
            float v1 = srcY1 / srcH;

            pushConstants.srcLower = DirectX::XMFLOAT2(u0, v0);
            pushConstants.srcUpper = DirectX::XMFLOAT2(u1, v1);

            // Convert dst pixel rect to NDC [-1,1].
            float fbWidth = static_cast<float>(dstImage.info.size.width);
            float fbHeight = static_cast<float>(dstImage.info.size.height);

            float dstX0 = static_cast<float>(scissorRect.left);
            float dstY0 = fbHeight - static_cast<float>(scissorRect.bottom);
            float dstX1 = static_cast<float>(scissorRect.right);
            float dstY1 = fbHeight - static_cast<float>(scissorRect.top);

            float ndcX0 = 2.0f * dstX0 / fbWidth - 1.0f;
            float ndcY0 = 2.0f * dstY0 / fbHeight - 1.0f;
            float ndcX1 = 2.0f * dstX1 / fbWidth - 1.0f;
            float ndcY1 = 2.0f * dstY1 / fbHeight - 1.0f;

            // Push to shader: dstLower should be (minX, minY), dstUpper (maxX, maxY) in NDC
            pushConstants.dstLower = DirectX::XMFLOAT2(ndcX0, ndcY0); // minX, minY
            // FIXME: WHY DOES THE PUSH CONSTANT IN THE GPU HAVE THIS RANDOM SLIGHT OFFSET???
            pushConstants.dstUpper = DirectX::XMFLOAT2(ndcX1, ndcY1); // maxX, maxY

            mCommandList->SetGraphicsRoot32BitConstants(0, sizeof(PushConstants) / 4, &pushConstants, 0);

            // 7. Draw quad (4 vertices, no VB needed)
            mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
            bBlitImageState = true;

            for (UINT j = 0; j < PYRO_IMAGE_SLICE_RESOLVE_LAYERS(info.srcImageSlice, srcImage.info.arrayLayerCount); ++j) {
                UINT srcSubresource = D3D12CalcSubresource(info.srcImageSlice.mipLevel, info.srcImageSlice.baseArrayLayer + j, 0,
                    srcImage.info.mipLevelCount, srcImage.info.arrayLayerCount);
                UINT dstSubresource = D3D12CalcSubresource(info.dstImageSlice.mipLevel, info.dstImageSlice.baseArrayLayer + j, 0,
                    dstImage.info.mipLevelCount, dstImage.info.arrayLayerCount);

                D3D12_DISCARD_REGION region = {};
                region.FirstSubresource = dstSubresource;
                region.NumRects = 1;
                region.NumSubresources = 1;
                region.pRects = &scissorRect; /* dst rect */
                mCommandList->DiscardResource(dstImage.resource.Get(), &region);

                ID3D12DescriptorHeap* heaps[] = {
                    srcImage.blitImageSRVHeaps[srcSubresource].mHeap.Get(),
                    info.filter == Filter::Linear
                        ? mDevice->mLinearSamplerDescriptorTable.mHeap.Get()
                        : mDevice->mNearestSamplerDescriptorTable.mHeap.Get(),
                };

                mCommandList->SetDescriptorHeaps(PYRO_ARRAY_SIZE(heaps), heaps);
                mCommandList->SetGraphicsRootDescriptorTable(2, info.filter == Filter::Linear
                                                                    ? mDevice->mLinearSamplerDescriptorTable.gpuDescriptor
                                                                    : mDevice->mNearestSamplerDescriptorTable.gpuDescriptor);
                mCommandList->SetGraphicsRootDescriptorTable(1, srcImage.blitImageSRVHeaps[srcSubresource].gpuDescriptor);

                ASSERT(dstImage.blitImageRTVs.size() > dstSubresource, "Image must have been created with BLIT DST capability, AND the image slice must not exceed the image!");
                D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = mDevice->ResourcePool().mRTVHeap.Resolve(dstImage.blitImageRTVs[dstSubresource]);
                mCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
                mCommandList->DrawInstanced(4, 1, 0, 0);
            }
            gDx12Context->FlushDebugMessages();
        }

        void D3DCommandBuffer::ClearUnorderedAccessView(const ClearUnorderedAccessViewInfo& info) {
            // FIXME: optimise this by caching the UAVs, this is probably insanely slow
            ID3D12DescriptorHeap* heapGpu = nullptr;
            ID3D12DescriptorHeap* heapCpu = nullptr;
            D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
            heapDesc.NumDescriptors = 1;
            heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            CheckD3DResult(mDevice->InternalDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heapGpu)));
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            CheckD3DResult(mDevice->InternalDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heapCpu)));
            D3DSetDebugName(heapGpu, "UAV Clear GPU Desriptor Heap");
            D3DSetDebugName(heapCpu, "UAV Clear CPU Desriptor Heap");

            D3D12_CPU_DESCRIPTOR_HANDLE handle = mDevice->ResourcePool().mUAVHeap.Resolve(info.view);
            mDevice->InternalDevice()->CopyDescriptorsSimple(1U, heapGpu->GetCPUDescriptorHandleForHeapStart(), handle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            mDevice->InternalDevice()->CopyDescriptorsSimple(1U, heapCpu->GetCPUDescriptorHandleForHeapStart(), handle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

            ID3D12Resource* resource = {};
            bool bUintClear = false;
            bool bRepeatFirst = false;
            const auto& vinfo = mDevice->ResourcePool().mUAVHeap.GetInfo(info.view);

            if (eastl::holds_alternative<BufferResourceInfo>(vinfo)) {
                auto& bresinfo = eastl::get<BufferResourceInfo>(vinfo);
                resource = mDevice->ResourcePool().Get(bresinfo.buffer).resource.Get();
                bUintClear = true;
                bRepeatFirst = true;
            } else if (eastl::holds_alternative<ImageResourceInfo>(vinfo)) {
                auto& iresinfo = eastl::get<ImageResourceInfo>(vinfo);
                auto& imgInfo = mDevice->ResourcePool().Get(iresinfo.image);
                resource = imgInfo.resource.Get();
                bUintClear = RHIUtil::GetFormatNumericType(eastl::get<ImageResourceInfo>(vinfo).format == Format::Inherit
                                                               ? imgInfo.info.format
                                                               : eastl::get<ImageResourceInfo>(vinfo).format) != RHIUtil::FormatNumericType::Float;
                bRepeatFirst = false;
            } else {
                ASSERT(false, "BAD VARIANT");
            }

            mCommandList->SetDescriptorHeaps(1U, &heapGpu);
            if (bUintClear) {
                UINT clearUint[4];
                if (bRepeatFirst) {
                    clearUint[0] = info.clearValue.uint32[0];
                    clearUint[1] = info.clearValue.uint32[0];
                    clearUint[2] = info.clearValue.uint32[0];
                    clearUint[3] = info.clearValue.uint32[0];
                } else {
                    clearUint[0] = info.clearValue.uint32[0];
                    clearUint[1] = info.clearValue.uint32[1];
                    clearUint[2] = info.clearValue.uint32[2];
                    clearUint[3] = info.clearValue.uint32[3];
                }
                mCommandList->ClearUnorderedAccessViewUint(heapGpu->GetGPUDescriptorHandleForHeapStart(), heapCpu->GetCPUDescriptorHandleForHeapStart(),
                    resource, clearUint, 0, nullptr);
            } else {
                FLOAT clearFlt[4];
                if (bRepeatFirst) {
                    clearFlt[0] = info.clearValue.float32[0];
                    clearFlt[1] = info.clearValue.float32[0];
                    clearFlt[2] = info.clearValue.float32[0];
                    clearFlt[3] = info.clearValue.float32[0];
                } else {
                    clearFlt[0] = info.clearValue.float32[0];
                    clearFlt[1] = info.clearValue.float32[1];
                    clearFlt[2] = info.clearValue.float32[2];
                    clearFlt[3] = info.clearValue.float32[3];
                }
                mCommandList->ClearUnorderedAccessViewFloat(heapGpu->GetGPUDescriptorHandleForHeapStart(), heapCpu->GetCPUDescriptorHandleForHeapStart(),
                    resource, clearFlt, 0, nullptr);
            }
            mDeferredDeleteOps.push_back({
                .resource = reinterpret_cast<void*>(heapGpu),
                .deleter = [](D3DDevice* device, void* resource) {
                    reinterpret_cast<ID3D12DescriptorHeap*>(resource)->Release();
                },
            });
            mDeferredDeleteOps.push_back({
                .resource = reinterpret_cast<void*>(heapCpu),
                .deleter = [](D3DDevice* device, void* resource) {
                    reinterpret_cast<ID3D12DescriptorHeap*>(resource)->Release();
                },
            });
            // invalidate
            mGraphicsLastBoundUAVDescriptorTable = {};
            mComputeLastBoundUAVDescriptorTable = {};
            FlushPendingUnorderedAccessViewBinds();
            gDx12Context->FlushDebugMessages();
        }
        void D3DCommandBuffer::UpdateBuffer(const UpdateBufferInfo& info) {
            if (!mCurrentLinearUploadBuffer) {
                mCurrentLinearUploadBuffer = mDevice->GetLinearBufferAllocation();
            }
            auto& dstBuffer = mDevice->ResourcePool().Get(info.buffer);
            UINT64 sz = eastl::min(dstBuffer.info.size, info.region.size);
            UINT64 offset;
            void* ptr = mCurrentLinearUploadBuffer->Allocate(sz, offset);
            if (ptr == nullptr) {
                mPendingReturnLinearUploadBuffers.emplace_back(mCurrentLinearUploadBuffer);
                mCurrentLinearUploadBuffer = nullptr;
                D3DCommandBuffer::UpdateBuffer(info);
            }
            memcpy(ptr, info.data, sz);
            mCommandList->CopyBufferRegion(dstBuffer.resource.Get(), info.region.offset,
                mCurrentLinearUploadBuffer->GetResource(), offset, sz);
            gDx12Context->FlushDebugMessages();
        }

        void D3DCommandBuffer::BufferBarrier(const BufferMemoryBarrierInfo& info) {
            if (info.srcLayout == BufferLayout::Undefined && info.dstLayout == BufferLayout::Undefined)
                return;
            auto& bufferInfo = mDevice->ResourcePool().Get(info.buffer);
            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Transition.pResource = bufferInfo.resource.Get();
            barrier.Transition.StateBefore = ToD3D12BufferResourceState(info.srcLayout);
            barrier.Transition.StateAfter = ToD3D12BufferResourceState(info.dstLayout);
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            if (barrier.Transition.StateAfter == barrier.Transition.StateBefore) {
                if (barrier.Transition.StateAfter == D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
                    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(barrier.Transition.pResource);
                    mCommandList->ResourceBarrier(1, &uavBarrier);
                }
                return;
            }
            mCommandList->ResourceBarrier(1, &barrier);
            gDx12Context->FlushDebugMessages();
            if (info.srcLayout == BufferLayout::Undefined) {
                if (info.dstLayout == BufferLayout::UnorderedAccess) {
                    mCommandList->DiscardResource(barrier.Transition.pResource, nullptr);
                    gDx12Context->FlushDebugMessages();
                }
            }
        }

        void D3DCommandBuffer::ImageBarrier(const ImageMemoryBarrierInfo& info) {
            if (info.srcLayout == ImageLayout::Undefined && info.dstLayout == ImageLayout::Undefined)
                return;
            auto& imageInfo = mDevice->ResourcePool().Get(info.image);
            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

            D3D12_RESOURCE_STATES validMask =
                D3D12_RESOURCE_STATE_COMMON |
                D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER |
                D3D12_RESOURCE_STATE_INDEX_BUFFER |
                D3D12_RESOURCE_STATE_RENDER_TARGET |
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS |
                D3D12_RESOURCE_STATE_DEPTH_WRITE |
                D3D12_RESOURCE_STATE_DEPTH_READ |
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE |
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
                D3D12_RESOURCE_STATE_STREAM_OUT |
                D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT |
                D3D12_RESOURCE_STATE_COPY_DEST |
                D3D12_RESOURCE_STATE_COPY_SOURCE |
                D3D12_RESOURCE_STATE_RESOLVE_DEST |
                D3D12_RESOURCE_STATE_RESOLVE_SOURCE |
                D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE |
                D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE |
                D3D12_RESOURCE_STATE_RESERVED_INTERNAL_8000 |
                D3D12_RESOURCE_STATE_RESERVED_INTERNAL_4000 |
                D3D12_RESOURCE_STATE_RESERVED_INTERNAL_100000 |
                D3D12_RESOURCE_STATE_RESERVED_INTERNAL_40000000 |
                D3D12_RESOURCE_STATE_RESERVED_INTERNAL_80000000 |
                D3D12_RESOURCE_STATE_GENERIC_READ |
                D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE |
                D3D12_RESOURCE_STATE_PRESENT |
                D3D12_RESOURCE_STATE_PREDICATION |
                D3D12_RESOURCE_STATE_VIDEO_DECODE_READ |
                D3D12_RESOURCE_STATE_VIDEO_DECODE_WRITE |
                D3D12_RESOURCE_STATE_VIDEO_PROCESS_READ |
                D3D12_RESOURCE_STATE_VIDEO_PROCESS_WRITE |
                D3D12_RESOURCE_STATE_VIDEO_ENCODE_READ |
                D3D12_RESOURCE_STATE_VIDEO_ENCODE_WRITE;
            bool bDepthStencil = RHIUtil::FormatIsDepthStencil(imageInfo.info.format);
            if (bDepthStencil) {
                validMask &= ~(D3D12_RESOURCE_STATE_RENDER_TARGET);
            } else {
                validMask &= ~(D3D12_RESOURCE_STATE_DEPTH_WRITE | D3D12_RESOURCE_STATE_DEPTH_READ);
            }

            barrier.Transition.pResource = imageInfo.resource.Get();
            if (bDepthStencil && info.srcLayout == ImageLayout::RenderTargetReadOnly) {
                barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_READ;
            } else {
                barrier.Transition.StateBefore = ToD3D12ImageResourceState(info.srcLayout) & validMask;
            }
            if (bDepthStencil && info.dstLayout == ImageLayout::RenderTargetReadOnly) {
                barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_READ;
            } else {
                barrier.Transition.StateAfter = ToD3D12ImageResourceState(info.dstLayout) & validMask;
            }
            if (barrier.Transition.StateAfter == barrier.Transition.StateBefore) {
                if (barrier.Transition.StateAfter == D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
                    D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(barrier.Transition.pResource);
                    mCommandList->ResourceBarrier(1, &uavBarrier);
                }
                return;
            }
            for (UINT i = 0; i < PYRO_IMAGE_SLICE_RESOLVE_LEVELS(info.imageSlice, imageInfo.info.mipLevelCount); ++i) {
                for (UINT j = 0; j < PYRO_IMAGE_SLICE_RESOLVE_LAYERS(info.imageSlice, imageInfo.info.arrayLayerCount); ++j) {
                    barrier.Transition.Subresource = D3D12CalcSubresource(info.imageSlice.baseMipLevel + i, info.imageSlice.baseArrayLayer + j, 0,
                        imageInfo.info.mipLevelCount, imageInfo.info.arrayLayerCount);
                    mCommandList->ResourceBarrier(1, &barrier);
                }
            }
            gDx12Context->FlushDebugMessages();
            if (info.srcLayout == ImageLayout::Undefined) {
                if (info.dstLayout == ImageLayout::UnorderedAccess || info.dstLayout == ImageLayout::RenderTarget || info.dstLayout == ImageLayout::BlitDst) {
                    mCommandList->DiscardResource(barrier.Transition.pResource, nullptr);
                    gDx12Context->FlushDebugMessages();
                }
            }
        }

        void D3DCommandBuffer::TransferBufferOwnership(Buffer buffer, ICommandQueue* dstQueue) {
            /*NOP*/
        }

        void D3DCommandBuffer::TransferImageOwnership(Image image, ICommandQueue* dstQueue) {
            /*NOP*/
        }

        void D3DCommandBuffer::AcquireBufferOwnership(Buffer buffer, ICommandQueue* srcQueue) {
            /*NOP*/
        }

        void D3DCommandBuffer::AcquireImageOwnership(Image image, ICommandQueue* srcQueue) {
            /*NOP*/
        }

        void D3DCommandBuffer::InvalidateTimestampQuery(const InvalidateTimestampQueryInfo& info) {
            /*NOP*/
        }

        void D3DCommandBuffer::WriteTimestamp(const WriteTimestampInfo& info) {
            auto* pool = static_cast<D3DTimestampQueryPool*>(info.queryPool);

            mCommandList->EndQuery(pool->GetInternalHeap(),
                D3D12_QUERY_TYPE_TIMESTAMP, info.queryIndex);
            auto& pair = mPendingQueryPoolMinMaxResolves[pool];
            pair.first = eastl::min(pair.first, info.queryIndex);
            pair.second = eastl::max(pair.second, info.queryIndex);
            gDx12Context->FlushDebugMessages();
        }


        void D3DCommandBuffer::BeginLabel(const CommandLabelInfo& info) {
            if (!gPixBeginEventOnCommandListFn)
                return;
            UINT64 col = (u64)(info.labelColor.r * 255) << 24 |
                         (u64)(info.labelColor.g * 255) << 16 |
                         (u64)(info.labelColor.b * 255) << 8 |
                         (u64)(info.labelColor.a * 255) << 0;
            gPixBeginEventOnCommandListFn(mCommandList.Get(), col, info.name.data());
            gDx12Context->FlushDebugMessages();
        }

        void D3DCommandBuffer::EndLabel() {
            if (!gPixBeginEventOnCommandListFn)
                return;
            gPixEndEventOnCommandListFn(mCommandList.Get());
            gDx12Context->FlushDebugMessages();
        }

        void D3DCommandBuffer::BeginRenderPass(const RenderPassBeginInfo& info) {
            D3D12_RECT renderArea = ToD3D12Rect(info.renderArea);
            eastl::fixed_vector<D3D12_CPU_DESCRIPTOR_HANDLE, 8> renderTargets{};
            D3D12_CPU_DESCRIPTOR_HANDLE depthStencil = {};

            for (const auto& colTarg : info.colorAttachments) {
                auto rt = eastl::bit_cast<D3DRenderTarget*>(colTarg.target);
                const auto& imageData = mDevice->ResourcePool().Get(rt->Info().image);
                renderTargets.emplace_back(rt->GetDescriptor());
                if (colTarg.loadOp == AttachmentLoadOp::Clear) {
                    FLOAT clearCol[4];
                    Format imageFormat = imageData.info.format;
                    switch (RHIUtil::GetFormatNumericType(imageFormat)) {
                    case RHIUtil::FormatNumericType::Float:
                        memcpy(clearCol, colTarg.clearValue.float32.data(), sizeof(clearCol));
                        break;
                    case RHIUtil::FormatNumericType::SignedInt:
                        clearCol[0] = static_cast<FLOAT>(colTarg.clearValue.int32[0]);
                        clearCol[1] = static_cast<FLOAT>(colTarg.clearValue.int32[1]);
                        clearCol[2] = static_cast<FLOAT>(colTarg.clearValue.int32[2]);
                        clearCol[3] = static_cast<FLOAT>(colTarg.clearValue.int32[3]);
                        break;
                    case RHIUtil::FormatNumericType::UnsignedInt:
                        clearCol[0] = static_cast<FLOAT>(colTarg.clearValue.uint32[0]);
                        clearCol[1] = static_cast<FLOAT>(colTarg.clearValue.uint32[1]);
                        clearCol[2] = static_cast<FLOAT>(colTarg.clearValue.uint32[2]);
                        clearCol[3] = static_cast<FLOAT>(colTarg.clearValue.uint32[3]);
                        break;
                    default:
                        ASSERT(false, "Bad image format!");
                        break;
                    }
                    mCommandList->ClearRenderTargetView(renderTargets.back(), clearCol, 1, &renderArea);
                } else if (colTarg.loadOp == AttachmentLoadOp::DontCare) {
                    D3D12_DISCARD_REGION region;
                    region.FirstSubresource = rt->GetSubresource();
                    region.NumSubresources = 1;
                    region.NumRects = 1;
                    region.pRects = &renderArea;
                    mCommandList->DiscardResource(imageData.resource.Get(), &region);
                }
                if (colTarg.resolve.has_value()) {
                    auto dstRt = eastl::bit_cast<D3DRenderTarget*>(colTarg.resolve->target);
                    const auto& dstImageData = mDevice->ResourcePool().Get(dstRt->Info().image);

                    mRenderPassResolves.push_back({
                        .src = imageData.resource.Get(),
                        .srcSubresource = rt->GetSubresource(),
                        .dst = dstImageData.resource.Get(),
                        .dstSubresource = dstRt->GetSubresource(),
                        .format = ToDXGIFormat(dstImageData.info.format),
                        .extent = { dstImageData.info.size.width, dstImageData.info.size.height },
                    });
                }
            }

            if (info.depthStencilAttachment.has_value()) {
                auto* rt = eastl::bit_cast<D3DRenderTarget*>(info.depthStencilAttachment->target);
                depthStencil = rt->GetDescriptor();
                auto& imageData = mDevice->ResourcePool().Get(rt->Info().image);
                D3D12_CLEAR_FLAGS depthStencilClear = {};
                if (info.depthStencilAttachment->depthLoadOp == AttachmentLoadOp::Clear) {
                    depthStencilClear |= D3D12_CLEAR_FLAG_DEPTH;
                }
                if (info.depthStencilAttachment->stencilLoadOp == AttachmentLoadOp::Clear) {
                    depthStencilClear |= D3D12_CLEAR_FLAG_STENCIL;
                }
                if (info.depthStencilAttachment->stencilLoadOp == AttachmentLoadOp::DontCare && info.depthStencilAttachment->depthLoadOp == AttachmentLoadOp::DontCare) {
                    D3D12_DISCARD_REGION region;
                    region.FirstSubresource = rt->GetSubresource();
                    region.NumSubresources = 1;
                    region.NumRects = 1;
                    region.pRects = &renderArea;
                    mCommandList->DiscardResource(imageData.resource.Get(), &region);
                } else if (depthStencilClear) {
                    mCommandList->ClearDepthStencilView(depthStencil, depthStencilClear,
                        info.depthStencilAttachment->clearValue.depth, (UINT)info.depthStencilAttachment->clearValue.stencil,
                        1, &renderArea);
                }
            }
            mCommandList->OMSetRenderTargets(static_cast<UINT>(renderTargets.size()), renderTargets.data(), FALSE,
                info.depthStencilAttachment.has_value() ? &depthStencil : nullptr);

            D3D12_VIEWPORT viewport{
                .TopLeftX = static_cast<FLOAT>(info.renderArea.x),
                .TopLeftY = static_cast<FLOAT>(info.renderArea.y),
                .Width = static_cast<FLOAT>(info.renderArea.width),
                .Height = static_cast<FLOAT>(info.renderArea.height),
                .MinDepth = 0.0f,
                .MaxDepth = 1.0f,
            };
            mCommandList->RSSetViewports(1, &viewport);
            mCommandList->RSSetScissorRects(1, &renderArea);
            gDx12Context->FlushDebugMessages();
        }

        void D3DCommandBuffer::EndRenderPass() {
            for (const auto& resolveInfo : mRenderPassResolves) {
                // discard first for resource optimisation
                D3D12_RECT resolveArea = ToD3D12Rect(Rect2D::Cut(resolveInfo.extent));
                D3D12_DISCARD_REGION region;
                region.FirstSubresource = resolveInfo.dstSubresource;
                region.NumSubresources = 1;
                region.NumRects = 1;
                region.pRects = &resolveArea;
                mCommandList->DiscardResource(resolveInfo.dst, &region);

                // DX12 uniquely has a resolve state, just opaquely transition them and be done with it
                D3D12_RESOURCE_BARRIER enterBarriers[2] = {
                    CD3DX12_RESOURCE_BARRIER::Transition(
                        resolveInfo.dst,
                        D3D12_RESOURCE_STATE_RENDER_TARGET,
                        D3D12_RESOURCE_STATE_RESOLVE_DEST,
                        resolveInfo.dstSubresource),
                    CD3DX12_RESOURCE_BARRIER::Transition(resolveInfo.src,
                        D3D12_RESOURCE_STATE_RENDER_TARGET,
                        D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
                        resolveInfo.srcSubresource),
                };
                D3D12_RESOURCE_BARRIER exitBarriers[2] = {
                    CD3DX12_RESOURCE_BARRIER::Transition(
                        resolveInfo.dst,
                        D3D12_RESOURCE_STATE_RESOLVE_DEST,
                        D3D12_RESOURCE_STATE_RENDER_TARGET,
                        resolveInfo.dstSubresource),
                    CD3DX12_RESOURCE_BARRIER::Transition(resolveInfo.src,
                        D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
                        D3D12_RESOURCE_STATE_RENDER_TARGET,
                        resolveInfo.srcSubresource),
                };
                mCommandList->ResourceBarrier(2, enterBarriers);
                mCommandList->ResolveSubresource(resolveInfo.dst, resolveInfo.dstSubresource, resolveInfo.src, resolveInfo.srcSubresource, resolveInfo.format);
                mCommandList->ResourceBarrier(2, exitBarriers);
            }
            mRenderPassResolves.clear();
            gDx12Context->FlushDebugMessages();
        }

        void D3DCommandBuffer::PushConstantVPtr(const PushConstantInfo& info) {
            ASSERT(PYRO_VERIFY_ALIGNMENT(info.size, 4), "Push constants must be DWord aligned!");
            ASSERT(PYRO_VERIFY_ALIGNMENT(info.offset, 4), "Push constants must be DWord aligned!");

            if (queueFlags & CommandQueueFlagBits::COMPUTE) {
                mCommandList->SetComputeRoot32BitConstants(0, info.size / 4, info.data, info.offset / 4);
            }
            if (queueFlags & CommandQueueFlagBits::GRAPHICS) {
                mCommandList->SetGraphicsRoot32BitConstants(0, info.size / 4, info.data, info.offset / 4);
            }
            gDx12Context->FlushDebugMessages();
        }

        void D3DCommandBuffer::SetUniformBufferView(const SetUniformBufferViewInfo& info) {
            D3D12_GPU_VIRTUAL_ADDRESS gpuAddress =
                mDevice->ResourcePool().Get(info.buffer).resource->GetGPUVirtualAddress() + info.region.offset;

            if (info.bindPoint == PipelineBindPoint::Graphics) {
                mCommandList->SetGraphicsRootConstantBufferView(info.slot + 6 /*match the root params in the signature*/, gpuAddress);
            } else if (info.bindPoint == PipelineBindPoint::Compute) {
                mCommandList->SetComputeRootConstantBufferView(info.slot + 6, gpuAddress);
            }
            gDx12Context->FlushDebugMessages();
        }

        void D3DCommandBuffer::SetUnorderedAccessView(const SetUnorderedAccessViewInfo& info) {
            if (mPendingUAVBinds.boundUavs.size() < info.slot + 1) {
                mPendingUAVBinds.boundUavs.resize(info.slot + 1);
            }
            mPendingUAVBinds.boundUavs[info.slot] = info.view;
            gDx12Context->FlushDebugMessages();
        }

        void D3DCommandBuffer::SetRasterPipeline(RasterPipeline pipeline) {
            auto* pipe = eastl::bit_cast<D3DRasterPipeline*>(pipeline);
            mCurrentRasterPipeline = pipe;
            mCommandList->SetPipelineState(pipe->mPipelineState.Get());
            mCommandList->IASetPrimitiveTopology(pipe->mTopology);
            mInvalidatedVertexBufferBindings.clear();
            u32 slot = 0;
            for (UINT stride : pipe->mVertexBufferStrides) {
                if (stride == 0)
                    continue;
                mInvalidatedVertexBufferBindings.emplace(slot);
            }
            for (UINT i = 0; i < pipe->mEmulatedSpecialisationConstants.size(); ++i) {
                if (pipe->mEmulatedSpecialisationConstants[i]) {
                    mCommandList->SetGraphicsRootConstantBufferView(i + 1 /*match the root params in the signature*/,
                        pipe->mEmulatedSpecialisationConstants[i]->GetGPUVirtualAddress());
                }
            }
            bIsComputePipeline = false;
            if (bBlitImageState) {
                mCommandList->SetGraphicsRootSignature(mDevice->mRootSignature.Get());
                eastl::array<ID3D12DescriptorHeap* const, 2u> descriptorHeaps{
                    mDevice->mDefaultUAVDescriptorTable.mHeap.Get(),
                    mDevice->ResourcePool().mSamplerHeap.InternalHeap()
                };
                mComputeLastBoundUAVDescriptorTable = mDevice->mDefaultUAVDescriptorTable;
                mCommandList->SetDescriptorHeaps(static_cast<UINT>(descriptorHeaps.size()), descriptorHeaps.data());
                bBlitImageState = false;
            }
            gDx12Context->FlushDebugMessages();
        }

        void D3DCommandBuffer::SetComputePipeline(ComputePipeline pipeline) {
            auto* pipe = eastl::bit_cast<D3DComputePipeline*>(pipeline);
            mCommandList->SetPipelineState(pipe->mPipelineState.Get());
            if (pipe->mEmulatedSpecialisationConstant) {
                mCommandList->SetComputeRootConstantBufferView(1 /*match the root params in the signature*/,
                    pipe->mEmulatedSpecialisationConstant->GetGPUVirtualAddress());
            }
            bIsComputePipeline = true;
            gDx12Context->FlushDebugMessages();
        }
        void D3DCommandBuffer::SetViewport(const ViewportInfo& info) {
            D3D12_VIEWPORT viewport{
                .TopLeftX = info.x,
                .TopLeftY = info.y,
                .Width = info.width,
                .Height = info.height,
                .MinDepth = info.minDepth,
                .MaxDepth = info.maxDepth,
            };
            mCommandList->RSSetViewports(1, &viewport);
            gDx12Context->FlushDebugMessages();
        }

        void D3DCommandBuffer::SetScissor(const Rect2D& info) {
            D3D12_RECT renderArea = ToD3D12Rect(info);
            mCommandList->RSSetScissorRects(1, &renderArea);
            gDx12Context->FlushDebugMessages();
        }

        void D3DCommandBuffer::SetVertexBuffer(const SetVertexBufferInfo& info) {
            // Due to how D3D12 requires the stride to be provided when binding a vertex buffer,
            // and our design only provides this in the pipeline level, we need to defer the binding
            // until we know what state we are in
            if (mPendingVertexBufferBinds.size() < (info.slot + 1)) {
                mPendingVertexBufferBinds.resize(info.slot + 1);
            }
            const auto& bufferInfo = mDevice->ResourcePool().Get(info.buffer);

            mPendingVertexBufferBinds[info.slot].BufferLocation = bufferInfo.resource->GetGPUVirtualAddress() + info.offset;
            mPendingVertexBufferBinds[info.slot].SizeInBytes = static_cast<u32>(eastl::min(bufferInfo.info.size - info.offset, static_cast<u64>(UINT32_MAX)));
            mInvalidatedVertexBufferBindings.emplace(info.slot);
            gDx12Context->FlushDebugMessages();
        }

        void D3DCommandBuffer::SetIndexBuffer(const SetIndexBufferInfo& info) {
            D3D12_INDEX_BUFFER_VIEW view{};
            switch (info.indexType) {
            case IndexType::Uint32:
                view.Format = DXGI_FORMAT_R32_UINT;
                break;
            case IndexType::Uint16:
                view.Format = DXGI_FORMAT_R16_UINT;
                break;
                // TODO: is UINT8 actually supported in d3d12?
            case IndexType::Uint8:
                view.Format = DXGI_FORMAT_R8_UINT;
                break;
            }
            const auto& bufferInfo = mDevice->ResourcePool().Get(info.buffer);
            view.BufferLocation = bufferInfo.resource->GetGPUVirtualAddress() + info.offset;
            view.SizeInBytes = static_cast<u32>(eastl::min(bufferInfo.info.size - info.offset, static_cast<u64>(UINT32_MAX)));
            mCommandList->IASetIndexBuffer(&view);
            gDx12Context->FlushDebugMessages();
        }

        void D3DCommandBuffer::Draw(const DrawInfo& info) {
            FlushPendingUnorderedAccessViewBinds();
            FlushPendingVertexBufferBinds();
            mCommandList->DrawInstanced(info.vertexCount, info.instanceCount, info.firstVertex, info.firstInstance);
            gDx12Context->FlushDebugMessages();
        }

        void D3DCommandBuffer::DrawIndexed(const DrawIndexedInfo& info) {
            FlushPendingUnorderedAccessViewBinds();
            FlushPendingVertexBufferBinds();
            mCommandList->DrawIndexedInstanced(info.indexCount, info.instanceCount, info.firstIndex, info.vertexOffset, info.firstInstance);
            gDx12Context->FlushDebugMessages();
        }

        void D3DCommandBuffer::DrawIndirect(const DrawIndirectInfo& info) {
            FlushPendingUnorderedAccessViewBinds();
            FlushPendingVertexBufferBinds();
            ID3D12CommandSignature* signature = mDevice->GetDrawCommandSignature();
            ID3D12Resource* resource = mDevice->ResourcePool().Get(info.indirectBuffer).resource.Get();
            for (UINT64 i = 0; i < info.drawCount; ++i) {
                mCommandList->SetGraphicsRoot32BitConstant(17, static_cast<UINT>(i), 0);
                mCommandList->ExecuteIndirect(signature,
                    1, resource,
                    info.indirectBufferOffset + i * info.drawCommandStride, nullptr, 0);
            }
            // set back to 0
            if (info.drawCount > 1) {
                mCommandList->SetGraphicsRoot32BitConstant(17, 0, 0);
            }
            gDx12Context->FlushDebugMessages();
        }

        void D3DCommandBuffer::DrawIndexedIndirect(const DrawIndexedIndirectInfo& info) {
            FlushPendingUnorderedAccessViewBinds();
            FlushPendingVertexBufferBinds();
            ID3D12CommandSignature* signature = mDevice->GetDrawIndexedCommandSignature();
            ID3D12Resource* resource = mDevice->ResourcePool().Get(info.indirectBuffer).resource.Get();
            for (UINT64 i = 0; i < info.drawCount; ++i) {
                mCommandList->SetGraphicsRoot32BitConstant(17, static_cast<UINT>(i), 0);
                mCommandList->ExecuteIndirect(signature,
                    1, resource,
                    info.indirectBufferOffset + i * info.drawCommandStride, nullptr, 0);
            }
            // set back to 0
            if (info.drawCount > 1) {
                mCommandList->SetGraphicsRoot32BitConstant(17, 0, 0);
            }
            gDx12Context->FlushDebugMessages();
        }

        void D3DCommandBuffer::Dispatch(const DispatchInfo& info) {
            FlushPendingUnorderedAccessViewBinds();
            mCommandList->Dispatch(info.x, info.y, info.z);
            gDx12Context->FlushDebugMessages();
        }

        void D3DCommandBuffer::DispatchIndirect(const DispatchIndirectInfo& info) {
            FlushPendingUnorderedAccessViewBinds();
            ID3D12CommandSignature* signature = mDevice->GetDispatchCommandSignature();
            mCommandList->ExecuteIndirect(signature,
                1, mDevice->ResourcePool().Get(info.indirectBuffer).resource.Get(), info.indirectBufferOffset, nullptr, 0);
            gDx12Context->FlushDebugMessages();
        }

        void D3DCommandBuffer::BuildAccelerationStructures(const BuildAccelerationStructuresInfo& info) {
            if (!mCommandList4) {
                Logger::Error(gDX12Sink, "BuildAccelerationStructures called, but ID3D12GraphicsCommandList4 is not available!");
                return;
            }

            // --- 1. Build all BLAS ---
            eastl::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geomDescs; // Re-used scratch space
            for (const auto& blasInfo : info.blasBuildInfos) {
                ASSERT(blasInfo.dstBlas != PYRO_NULL_BLAS, "dstBlas must never be null!");
                ASSERT(blasInfo.scratchBuffer != PYRO_NULL_BUFFER, "Scratch buffers must never be null!");
                geomDescs.clear(); // Clear for each new BLAS
                D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};

                // Get destination and scratch buffer addresses
                buildDesc.DestAccelerationStructureData = mDevice->ResourcePool().Get(blasInfo.dstBlas).address;
                buildDesc.ScratchAccelerationStructureData = mDevice->ResourcePool().Get(blasInfo.scratchBuffer).resource->GetGPUVirtualAddress();

                // Fill in the geometry inputs
                mDevice->FillBlasInputs(buildDesc.Inputs, blasInfo, geomDescs);

                // Handle update
                if (blasInfo.bUpdate) {
                    buildDesc.SourceAccelerationStructureData = mDevice->ResourcePool().Get(blasInfo.srcBlas).address;
                }

                mCommandList4->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);
            }

            // --- 2. UAV Barrier for BLAS ---
            // We need to ensure all BLAS builds are finished before the TLAS build starts reading them.
            if (info.blasBuildInfos.size() > 0 && info.tlasBuildInfos.size() > 0) {
                D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(nullptr);
                mCommandList->ResourceBarrier(1, &uavBarrier);
            }

            // --- 3. Build all TLAS ---
            for (const auto& tlasInfo : info.tlasBuildInfos) {
                ASSERT(tlasInfo.dstTlas != PYRO_NULL_TLAS, "dstTlas must never be null!");
                ASSERT(tlasInfo.scratchBuffer != PYRO_NULL_BUFFER, "Scratch buffers must never be null!");
                D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};

                // Get destination and scratch buffer addresses
                buildDesc.DestAccelerationStructureData = eastl::get<D3DTlasData>(mDevice->ResourcePool().mSRVHeap.GetInfo(tlasInfo.dstTlas)).resource->GetGPUVirtualAddress();
                buildDesc.ScratchAccelerationStructureData = mDevice->ResourcePool().Get(tlasInfo.scratchBuffer).resource->GetGPUVirtualAddress();

                // Fill inputs
                buildDesc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
                buildDesc.Inputs.Flags = ToD3D12ASBuildFlags(tlasInfo.flags);

                // TODO: DX12 only supports one of these descriptions
                ASSERT(tlasInfo.instances.size() == 1);
                buildDesc.Inputs.NumDescs = tlasInfo.instances[0].count;
                buildDesc.Inputs.InstanceDescs = mDevice->ResourcePool().Get(tlasInfo.instances[0].data).resource->GetGPUVirtualAddress();

                // Handle update
                if (tlasInfo.update) {
                    buildDesc.Inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
                    buildDesc.SourceAccelerationStructureData = eastl::get<D3DTlasData>(mDevice->ResourcePool().mSRVHeap.GetInfo(tlasInfo.srcTlas)).resource->GetGPUVirtualAddress();
                }

                mCommandList4->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);
            }

            // --- 4. Final UAV Barrier ---
            // Ensure TLAS build is finished before any shader tries to read it.
            if (info.tlasBuildInfos.size() > 0) {
                D3D12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(nullptr);
                mCommandList->ResourceBarrier(1, &uavBarrier);
            }

            gDx12Context->FlushDebugMessages();
        }

        void D3DCommandBuffer::Complete() {
            for (auto& [pool, minmax] : mPendingQueryPoolMinMaxResolves) {
                auto [min, max] = minmax;
                mCommandList->ResolveQueryData(pool->GetInternalHeap(),
                    D3D12_QUERY_TYPE_TIMESTAMP, min, max - min + 1 /*turn index into COUNT*/,
                    pool->GetReadbackBuffer(), min * sizeof(UINT64));
            }

            mPendingQueryPoolMinMaxResolves.clear();
            CheckD3DResult(mCommandList->Close());
            mCurrentRasterPipeline = nullptr;
            mInvalidatedVertexBufferBindings.clear();
            mPendingVertexBufferBinds.clear();
            mPendingUAVBinds = {};
            mComputeLastBoundUAVDescriptorTable = {};
            mGraphicsLastBoundUAVDescriptorTable = {};
            mRenderPassResolves.clear();
            if (mCurrentLinearUploadBuffer) {
                mPendingReturnLinearUploadBuffers.push_back(mCurrentLinearUploadBuffer);
                mCurrentLinearUploadBuffer = nullptr;
            }
            gDx12Context->FlushDebugMessages();
        }

        void D3DCommandBuffer::FlushPendingVertexBufferBinds() {
            if (mInvalidatedVertexBufferBindings.empty())
                return;
            for (u32 binding : mInvalidatedVertexBufferBindings) {
                ASSERT(binding < mPendingVertexBufferBinds.size(), "Currently bound raster pipeline requires slot " + eastl::to_string(binding) + " to be bound, but it's not!");
                mPendingVertexBufferBinds[binding].StrideInBytes = mCurrentRasterPipeline->mVertexBufferStrides[binding];
                if (mPendingVertexBufferBinds[binding].StrideInBytes == 0)
                    continue;
                mCommandList->IASetVertexBuffers(binding, 1, &mPendingVertexBufferBinds[binding]);
            }
            mInvalidatedVertexBufferBindings.clear();
            gDx12Context->FlushDebugMessages();
        }
        void D3DCommandBuffer::FlushPendingUnorderedAccessViewBinds() {
            // HACK:
            // This is the UGLIEST way to do dynamic descriptor table updates
            // Right now we keep track of a list of UAVs to be bound, and then last minute copy the entire bindless array
            // To a new descriptor heap (or reuse if it's been cached already) and then bind it.
            auto descriptorTable = mDevice->GetUnorderedAccessViewDescriptorTable(mPendingUAVBinds);

            eastl::array<ID3D12DescriptorHeap* const, 2u> descriptorHeaps{
                descriptorTable.mHeap.Get(),
                mDevice->ResourcePool().mSamplerHeap.InternalHeap()
            };
            if (queueFlags & CommandQueueFlagBits::COMPUTE) {
                if (descriptorTable != mComputeLastBoundUAVDescriptorTable) {
                    mComputeLastBoundUAVDescriptorTable = descriptorTable;
                    mCommandList->SetDescriptorHeaps(static_cast<UINT>(descriptorHeaps.size()), descriptorHeaps.data());
                    mCommandList->SetComputeRootDescriptorTable(14, descriptorTable.gpuDescriptor);
                    mCommandList->SetComputeRootDescriptorTable(16, descriptorTable.gpuDescriptor);
                }
            }
            if (queueFlags & CommandQueueFlagBits::GRAPHICS) {
                if (descriptorTable != mGraphicsLastBoundUAVDescriptorTable) {
                    mGraphicsLastBoundUAVDescriptorTable = descriptorTable;
                    mCommandList->SetDescriptorHeaps(static_cast<UINT>(descriptorHeaps.size()), descriptorHeaps.data());
                    mCommandList->SetGraphicsRootDescriptorTable(14, descriptorTable.gpuDescriptor);
                    mCommandList->SetGraphicsRootDescriptorTable(16, descriptorTable.gpuDescriptor);
                }
            }
            gDx12Context->FlushDebugMessages();
        }
    } // namespace RHIDX12
} // namespace PyroshockStudios