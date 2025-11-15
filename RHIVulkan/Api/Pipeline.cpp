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

#include "Pipeline.hpp"
#include "Device.hpp"
#include <RHIVulkan/VkContext.hpp>

namespace PyroshockStudios {
    namespace RHIVulkan {
        struct TempSpecializationInfo {
            eastl::vector<u32> data;
            eastl::vector<VkSpecializationMapEntry> entries;
            VkSpecializationInfo info;
        };
        static void createShaderStage(const ShaderInfo& shaderInfo, VkShaderStageFlagBits stage,
            eastl::vector<VkPipelineShaderStageCreateInfo>& stages, eastl::vector<eastl::unique_ptr<TempSpecializationInfo>>& specializations, VulkanDevice* device) {
            VkShaderModuleCreateInfo moduleCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .codeSize = static_cast<u32>(shaderInfo.program.size()),
                .pCode = reinterpret_cast<const u32*>(shaderInfo.program.data()),
            };
            VkShaderModule module = {};
            VkResult result = vkCreateShaderModule(device->GetVkDevice(), &moduleCreateInfo, device->Context()->GetVkAllocator(), &module);
            CheckVkResult(result, "Failed to compile shader module!");

            VkPipelineShaderStageCreateInfo stageCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = stage,
                .module = module,
                .pName = "main",
            };


            if (!shaderInfo.specializationConstants.empty()) {
                eastl::unique_ptr specInfo = eastl::make_unique<TempSpecializationInfo>();
                specInfo->data.reserve(shaderInfo.specializationConstants.size());
                specInfo->entries.reserve(shaderInfo.specializationConstants.size());
                for (const auto [location, data] : shaderInfo.specializationConstants) {
                    specInfo->entries.emplace_back(location, static_cast<u32>(specInfo->data.size() * sizeof(u32)), static_cast<u32>(sizeof(u32)));
                    specInfo->data.emplace_back(data.Get<u32>());
                }
                specInfo->info = {
                    .mapEntryCount = static_cast<u32>(specInfo->entries.size()),
                    .pMapEntries = specInfo->entries.data(),
                    .dataSize = specInfo->data.size() * sizeof(u32),
                    .pData = specInfo->data.data(),
                };
                specializations.emplace_back(eastl::move(specInfo));
                stageCreateInfo.pSpecializationInfo = &specializations.back()->info;
            }

            stages.push_back(stageCreateInfo);
        };

        VulkanRasterPipeline::VulkanRasterPipeline(VulkanDevice* device, const RasterPipelineInfo& info, const RasterPipelineShaderStages& rasterShaderStages)
            : mDevice(device), mInfo(info) {
            // === Shader Stages ===
            eastl::vector<VkPipelineShaderStageCreateInfo> shaderStages = {};
            eastl::vector<eastl::unique_ptr<TempSpecializationInfo>> shaderSpecializations = {};
            if (rasterShaderStages.vertexShaderInfo)
                createShaderStage(*rasterShaderStages.vertexShaderInfo, VK_SHADER_STAGE_VERTEX_BIT, shaderStages, shaderSpecializations, device);
            if (rasterShaderStages.hullShaderInfo)
                createShaderStage(*rasterShaderStages.hullShaderInfo, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, shaderStages, shaderSpecializations, device);
            if (rasterShaderStages.domainShaderInfo)
                createShaderStage(*rasterShaderStages.domainShaderInfo, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, shaderStages, shaderSpecializations, device);
            if (rasterShaderStages.geometryShaderInfo)
                createShaderStage(*rasterShaderStages.geometryShaderInfo, VK_SHADER_STAGE_GEOMETRY_BIT, shaderStages, shaderSpecializations, device);
            if (rasterShaderStages.fragmentShaderInfo)
                createShaderStage(*rasterShaderStages.fragmentShaderInfo, VK_SHADER_STAGE_FRAGMENT_BIT, shaderStages, shaderSpecializations, device);

            // === Vertex Input

            eastl::vector<VkVertexInputAttributeDescription> vertexInputAttributes(info.inputAssemblyState.vertexAttributes.size());
            eastl::vector<VkVertexInputBindingDescription> vertexInputBindings(info.inputAssemblyState.vertexBindings.size());

            for (i32 i = 0; i < vertexInputAttributes.size(); ++i) {
                vertexInputAttributes[i].binding = info.inputAssemblyState.vertexAttributes[i].binding;
                vertexInputAttributes[i].format = ToVkFormat(info.inputAssemblyState.vertexAttributes[i].format);
                vertexInputAttributes[i].location = info.inputAssemblyState.vertexAttributes[i].location;
                vertexInputAttributes[i].offset = info.inputAssemblyState.vertexAttributes[i].offset;
            }
            for (i32 i = 0; i < vertexInputBindings.size(); ++i) {
                vertexInputBindings[i].binding = info.inputAssemblyState.vertexBindings[i].binding;
                vertexInputBindings[i].inputRate = info.inputAssemblyState.vertexBindings[i].bPerInstance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
                vertexInputBindings[i].stride = info.inputAssemblyState.vertexBindings[i].stride;
            }

            VkPipelineVertexInputStateCreateInfo vertexInput = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                .vertexBindingDescriptionCount = static_cast<u32>(vertexInputBindings.size()),
                .pVertexBindingDescriptions = vertexInputBindings.data(),
                .vertexAttributeDescriptionCount = static_cast<u32>(vertexInputAttributes.size()),
                .pVertexAttributeDescriptions = vertexInputAttributes.data(),
            };

            // === Input Assembly ===
            VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                .topology = ToVkPrimitiveTopology(info.inputAssemblyState.primitiveTopology),
                .primitiveRestartEnable = info.inputAssemblyState.bPrimitiveRestart
            };

            // === Tesselation State ===
            VkPipelineTessellationStateCreateInfo tessellationState{};
            if (info.tesselationState) {
                tessellationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
                tessellationState.patchControlPoints = info.tesselationState->controlPoints;
            }

            // === Viewport/Scissor (dynamic) ===
            VkPipelineViewportStateCreateInfo viewportState = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .viewportCount = 1,
                .scissorCount = 1
            };

            // === Rasterizer ===
            VkPipelineRasterizationStateCreateInfo rasterizer = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                .depthClampEnable = info.rasterizerState.bDepthClamp,
                .rasterizerDiscardEnable = info.rasterizerState.bRasterizerDiscard,
                .polygonMode = ToVkPolygonMode(info.rasterizerState.polygonMode),
                .cullMode = (VkCullModeFlags)ToVkFaceCull(info.rasterizerState.faceCulling),
                .frontFace = ToVkFrontFaceWinding(info.rasterizerState.frontFaceWinding),
                .depthBiasEnable = info.rasterizerState.bDepthBias,
                .depthBiasConstantFactor = info.rasterizerState.depthBiasConstantFactor,
                .depthBiasClamp = info.rasterizerState.depthBiasClamp,
                .depthBiasSlopeFactor = info.rasterizerState.depthBiasSlopeFactor,
                .lineWidth = info.rasterizerState.lineWidth
            };

            VkPipelineRasterizationLineStateCreateInfoEXT lineRasterModeExt{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO_EXT };
            if (mDevice->mVulkanCaps.bVK_EXT_line_rasterization && info.rasterizerState.lineMode == LineMode::Smooth) {
                lineRasterModeExt.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT;
                rasterizer.pNext = &lineRasterModeExt;
            }

            // === Multisampling ===
            VkPipelineMultisampleStateCreateInfo multisampling = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                .rasterizationSamples = static_cast<VkSampleCountFlagBits>(info.multiSampleState.sampleCount),
                .sampleShadingEnable = info.multiSampleState.bSampleShading,
                .minSampleShading = info.multiSampleState.minSampleShading,
                .alphaToCoverageEnable = info.multiSampleState.bAlphaToCoverage
            };

            // === Depth Stencil ===
            VkPipelineDepthStencilStateCreateInfo depthStencil{};
            if (info.depthStencilState) {
                depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
                switch (info.depthStencilState->depthTestState) {
                case DepthStencilTestState::Ignore:
                    depthStencil.depthTestEnable = VK_FALSE;
                    depthStencil.depthWriteEnable = VK_FALSE;
                    break;
                case DepthStencilTestState::Read:
                    depthStencil.depthTestEnable = VK_TRUE;
                    depthStencil.depthWriteEnable = VK_FALSE;
                    break;
                case DepthStencilTestState::ReadWrite:
                    depthStencil.depthTestEnable = VK_TRUE;
                    depthStencil.depthWriteEnable = VK_TRUE;
                    break;
                }
                depthStencil.depthCompareOp = ToVkCompareOp(info.depthStencilState->depthTest.compareOp);
                depthStencil.minDepthBounds = info.depthStencilState->depthTest.minDepthBounds;
                depthStencil.maxDepthBounds = info.depthStencilState->depthTest.maxDepthBounds;
                depthStencil.stencilTestEnable = info.depthStencilState->stencilTestState != DepthStencilTestState::Ignore;
                if (depthStencil.stencilTestEnable) {
                    depthStencil.front.compareMask = info.depthStencilState->frontStencilTest.compareMask;
                    depthStencil.front.compareOp = ToVkCompareOp(info.depthStencilState->frontStencilTest.compareOp);
                    depthStencil.front.depthFailOp = ToVkStencilOp(info.depthStencilState->frontStencilTest.depthFailOp);
                    depthStencil.front.failOp = ToVkStencilOp(info.depthStencilState->frontStencilTest.failOp);
                    depthStencil.front.passOp = ToVkStencilOp(info.depthStencilState->frontStencilTest.passOp);
                    depthStencil.front.reference = info.depthStencilState->frontStencilTest.reference;
                    depthStencil.front.writeMask = info.depthStencilState->frontStencilTest.writeMask;

                    depthStencil.back.compareMask = info.depthStencilState->backStencilTest.compareMask;
                    depthStencil.back.compareOp = ToVkCompareOp(info.depthStencilState->backStencilTest.compareOp);
                    depthStencil.back.depthFailOp = ToVkStencilOp(info.depthStencilState->backStencilTest.depthFailOp);
                    depthStencil.back.failOp = ToVkStencilOp(info.depthStencilState->backStencilTest.failOp);
                    depthStencil.back.passOp = ToVkStencilOp(info.depthStencilState->backStencilTest.passOp);
                    depthStencil.back.reference = info.depthStencilState->backStencilTest.reference;
                    depthStencil.back.writeMask = info.depthStencilState->backStencilTest.writeMask;
                }
            }

            // === Color Blending ===
            eastl::fixed_vector<VkPipelineColorBlendAttachmentState, 8> blendAttachments;
            eastl::fixed_vector<VkFormat, 8> colorAttachmentFormats;
            for (const auto& attachment : info.colorTargetStates) {
                VkPipelineColorBlendAttachmentState blendState = {
                    .blendEnable = attachment.blend.has_value(),
                    .colorWriteMask = ToVkColorComponentFlags(attachment.writeMask)
                };
                if (attachment.blend) {
                    blendState.alphaBlendOp = ToVkBlendOp(attachment.blend->alphaBlendOp);
                    blendState.colorBlendOp = ToVkBlendOp(attachment.blend->colorBlendOp);
                    blendState.dstAlphaBlendFactor = ToVkBlendFactor(attachment.blend->dstAlphaBlendFactor);
                    blendState.srcAlphaBlendFactor = ToVkBlendFactor(attachment.blend->srcAlphaBlendFactor);
                    blendState.dstColorBlendFactor = ToVkBlendFactor(attachment.blend->dstColorBlendFactor);
                    blendState.srcColorBlendFactor = ToVkBlendFactor(attachment.blend->srcColorBlendFactor);
                }
                blendAttachments.push_back(blendState);
                colorAttachmentFormats.push_back(ToVkFormat(attachment.format));
            }

            VkPipelineColorBlendStateCreateInfo colorBlending = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                .attachmentCount = static_cast<u32>(blendAttachments.size()),
                .pAttachments = blendAttachments.data()
            };

            // === Dynamic State ===
            VkDynamicState dynamicStates[] = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
            };

            VkPipelineDynamicStateCreateInfo dynamicState = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                .dynamicStateCount = 2,
                .pDynamicStates = dynamicStates
            };

            Format depthStencilFormat = Format::Undefined;
            if (info.depthStencilState) {
                depthStencilFormat = info.depthStencilState->depthStencilFormat;
            }

            // === dynamic rendering stuff ===
            VkPipelineRenderingCreateInfo dynamicRenderingInfo = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                .colorAttachmentCount = static_cast<u32>(colorAttachmentFormats.size()),
                .pColorAttachmentFormats = colorAttachmentFormats.data(),
                .depthAttachmentFormat = RHIUtil::FormatHasDepth(depthStencilFormat) ? ToVkFormat(depthStencilFormat) : VK_FORMAT_UNDEFINED,
                .stencilAttachmentFormat = RHIUtil::FormatHasStencil(depthStencilFormat) ? ToVkFormat(depthStencilFormat) : VK_FORMAT_UNDEFINED,
            };


            // === Pipeline Info ===
            VkGraphicsPipelineCreateInfo pipelineInfo = {
                .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                .pNext = &dynamicRenderingInfo,
                .stageCount = static_cast<u32>(shaderStages.size()),
                .pStages = shaderStages.data(),
                .pVertexInputState = &vertexInput,
                .pInputAssemblyState = &inputAssembly,
                .pTessellationState = info.tesselationState ? &tessellationState : nullptr,
                .pViewportState = &viewportState,
                .pRasterizationState = &rasterizer,
                .pMultisampleState = &multisampling,
                .pDepthStencilState = info.depthStencilState ? &depthStencil : nullptr,
                .pColorBlendState = &colorBlending,
                .pDynamicState = &dynamicState,
                .layout = mDevice->GetResourceTable().mPipelineLayout,
                .renderPass = VK_NULL_HANDLE,
                .subpass = 0,
            };

            VkResult result = vkCreateGraphicsPipelines(mDevice->GetVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, mDevice->Context()->GetVkAllocator(), &mPipeline);
            CheckVkResult(result, "Failed to create raster pipeline!");

            if (vkSetDebugUtilsObjectNameEXT) {
                const VkDebugUtilsObjectNameInfoEXT nameinfo = {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .pNext = nullptr,
                    .objectType = VK_OBJECT_TYPE_PIPELINE,
                    .objectHandle = eastl::bit_cast<uint64_t>(mPipeline),
                    .pObjectName = info.name.c_str(),
                };
                vkSetDebugUtilsObjectNameEXT(mDevice->GetVkDevice(), &nameinfo);
            }
            for (const auto& stage : shaderStages) {
                vkDestroyShaderModule(mDevice->GetVkDevice(), stage.module, mDevice->Context()->GetVkAllocator());
            }
        }
        VulkanRasterPipeline::~VulkanRasterPipeline() {
            vkDestroyPipeline(mDevice->GetVkDevice(), mPipeline, mDevice->Context()->GetVkAllocator());
        }
        VulkanComputePipeline::VulkanComputePipeline(VulkanDevice* device, const ComputePipelineInfo& info, const ShaderInfo& shader)
            : mDevice(device), mInfo(info) {
            eastl::vector<VkPipelineShaderStageCreateInfo> shaderStages = {};
            eastl::vector<eastl::unique_ptr<TempSpecializationInfo>> shaderSpecializations = {};
            createShaderStage(shader, VK_SHADER_STAGE_COMPUTE_BIT, shaderStages, shaderSpecializations, device);
            VkComputePipelineCreateInfo createInfo{
                .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
                .stage = shaderStages[0],
                .layout = mDevice->GetResourceTable().mPipelineLayout,
            };

            VkResult result = vkCreateComputePipelines(mDevice->GetVkDevice(), VK_NULL_HANDLE, 1, &createInfo, mDevice->Context()->GetVkAllocator(), &mPipeline);
            CheckVkResult(result, "Failed to create compute pipeline!");

            if (vkSetDebugUtilsObjectNameEXT) {
                const VkDebugUtilsObjectNameInfoEXT nameinfo = {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .pNext = nullptr,
                    .objectType = VK_OBJECT_TYPE_PIPELINE,
                    .objectHandle = eastl::bit_cast<uint64_t>(mPipeline),
                    .pObjectName = info.name.c_str(),
                };
                vkSetDebugUtilsObjectNameEXT(mDevice->GetVkDevice(), &nameinfo);
            }
            for (const auto& stage : shaderStages) {
                vkDestroyShaderModule(mDevice->GetVkDevice(), stage.module, mDevice->Context()->GetVkAllocator());
            }
        }
        VulkanComputePipeline::~VulkanComputePipeline() {
            vkDestroyPipeline(mDevice->GetVkDevice(), mPipeline, mDevice->Context()->GetVkAllocator());
        }
    } // namespace RHIVulkan
} // namespace PyroshockStudios