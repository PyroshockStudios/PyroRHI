#pragma once
#include <RHIVulkan/Core.hpp>

#include <PyroRHI/Api/Pipeline.hpp>

namespace PyroshockStudios {
    namespace RHIVulkan {
        class VulkanDevice;

        class VulkanRasterPipeline : DeleteCopy, DeleteMove {
        public:
            VulkanRasterPipeline(VulkanDevice* device, const RasterPipelineInfo& info, const RasterPipelineShaderStages& rasterShaderStages);
            ~VulkanRasterPipeline();

            VkPipeline GetVkPipeline() {
                return mPipeline;
            }

            const RasterPipelineInfo& Info() {
                return mInfo;
            }

        private:
            VkPipeline mPipeline;
            VulkanDevice* mDevice;
            RasterPipelineInfo mInfo;
        };

        class VulkanComputePipeline : DeleteCopy, DeleteMove {
        public:
            VulkanComputePipeline(VulkanDevice* device, const ComputePipelineInfo& info, const ShaderInfo& shader);
            ~VulkanComputePipeline();

            VkPipeline GetVkPipeline() {
                return mPipeline;
            }

            const ComputePipelineInfo& Info() {
                return mInfo;
            }

        private:
            VkPipeline mPipeline;
            VulkanDevice* mDevice;
            ComputePipelineInfo mInfo;
        };
    } // namespace RHIVulkan
} // namespace PyroshockStudios