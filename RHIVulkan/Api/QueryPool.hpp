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
#include <PyroRHI/Api/IQueryPool.hpp>
#include <RHIVulkan/Core.hpp>

namespace PyroshockStudios {
    namespace RHIVulkan {
        class VulkanDevice;

        class VulkanTimestampQueryPool : public ITimestampQueryPool, DeleteCopy, DeleteMove {
        public:
            VulkanTimestampQueryPool(VulkanDevice* device, const TimestampQueryPoolInfo& info);
            ~VulkanTimestampQueryPool();
            const TimestampQueryPoolInfo& Info() const override {
                return mInfo;
            }

            eastl::span<const u64> GetTimestamps(u32 startIndex, u32 count) const override;

            VkQueryPool GetVkQueryPool() {
                return mQueryPool;
            }

        private:
            VkQueryPool mQueryPool;
            VulkanDevice* mDevice;
            TimestampQueryPoolInfo mInfo;
            mutable eastl::vector<u64> mResultBuffer = {};
        };
    } // namespace RHIVulkan
} // namespace PyroshockStudios