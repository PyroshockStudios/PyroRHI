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

#include "QueryPool.hpp"
#include "Device.hpp"
#include <RHIVulkan/VkContext.hpp>

#include <libassert/assert.hpp>

namespace PyroshockStudios {
    namespace RHIVulkan {
        VulkanTimestampQueryPool::VulkanTimestampQueryPool(VulkanDevice* device, const TimestampQueryPoolInfo& info)
            : mInfo(info), mDevice(device) {
            VkQueryPoolCreateInfo createInfo{
                .sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
                .queryType = VK_QUERY_TYPE_TIMESTAMP,
                .queryCount = info.queryCount,
            };

           CheckVkResult(vkCreateQueryPool(device->GetVkDevice(),
                &createInfo, device->Context()->GetVkAllocator(), &mQueryPool), "Failed to create timestamp query pool!");
            if (vkSetDebugUtilsObjectNameEXT) {
                const VkDebugUtilsObjectNameInfoEXT nameinfo = {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .pNext = nullptr,
                    .objectType = VK_OBJECT_TYPE_QUERY_POOL,
                    .objectHandle = eastl::bit_cast<uint64_t>(mQueryPool),
                    .pObjectName = info.name.c_str(),
                };
                vkSetDebugUtilsObjectNameEXT(mDevice->GetVkDevice(), &nameinfo);
            }
            mResultBuffer.resize(info.queryCount);
        }
        VulkanTimestampQueryPool::~VulkanTimestampQueryPool() {
            vkDestroyQueryPool(mDevice->GetVkDevice(), mQueryPool, mDevice->Context()->GetVkAllocator());
        }

        eastl::span<const u64> VulkanTimestampQueryPool::GetTimestamps(u32 startIndex, u32 count) const {
            ASSERT((startIndex + count) <= mInfo.queryCount, "Timestamp query out of bounds!");
            VkResult result = vkGetQueryPoolResults(mDevice->GetVkDevice(), mQueryPool, startIndex, count,
                mResultBuffer.size() * sizeof(u64), mResultBuffer.data() + startIndex, sizeof(u64), VK_QUERY_RESULT_64_BIT);
            if (result == VK_NOT_READY) {
                return {};
            } else {
                CheckVkResult(result, "Failed to get timestamp query results!");
            } 
            return eastl::span<const u64>(mResultBuffer.cbegin() + startIndex, mResultBuffer.cbegin() + startIndex + count);
        }

    } // namespace RHIVulkan
} // namespace PyroshockStudios