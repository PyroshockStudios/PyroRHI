#include "Helpers/Misc.hpp"
#include "Helpers/ValidationFixture.hpp"
#include "Shaders/Integration.hpp"
#include "Shaders/Stubs.hpp"

using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;


TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateAndDestroyRasterPipelineDepthVertexOnly) {
    ShaderObject vsh = mShaderCompiler->CompileShaderFromSource( // Changed to CompileShaderFromSource
        gTEST_VERTEX_SHADER,
        {
            .stage = ShaderStage::Vertex,
            .entryPoint = "main",
            .name = "Vsh",
        });

    RasterPipelineInfo rasterPipelineInfo = {};
    rasterPipelineInfo.name = "TestPipeline_DepthVertexOnly";
    rasterPipelineInfo.depthStencilState.emplace(DepthStencilStateInfo{
        .depthStencilFormat = Format::D32Sfloat, // Assuming a common depth format
        .depthTestState = DepthStencilTestState::ReadWrite,
        .depthTest = { .compareOp = CompareOp::Less },
    });
    RasterPipelineShaderStages rasterPipelineStages = {
        .vertexShaderInfo = eastl::make_optional(ShaderInfo{
            .program = vsh->bytecode,
        })
    };

    RasterPipeline pipeline = mDevice->Create(rasterPipelineInfo, rasterPipelineStages);
    ASSERT_NE(pipeline, RasterPipeline{}); // Check for valid handle
    ASSERT_EQ(mDevice->GetRasterPipelineInfo(pipeline), rasterPipelineInfo);
    mDevice->Destroy(pipeline);
}


TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateAndDestroyRasterPipelineColorVertexFragment) {
    ShaderObject vsh = mShaderCompiler->CompileShaderFromSource( // Changed to CompileShaderFromSource
        gTEST_VERTEX_SHADER,
        {
            .stage = ShaderStage::Vertex,
            .entryPoint = "main",
            .name = "Vsh",
        });
    ShaderObject fsh = mShaderCompiler->CompileShaderFromSource( // Changed to CompileShaderFromSource
        gTEST_FRAGMENT_SHADER,
        {
            .stage = ShaderStage::Fragment,
            .entryPoint = "main",
            .name = "Fsh",
        });

    RasterPipelineInfo rasterPipelineInfo = {};
    rasterPipelineInfo.name = "TestPipeline_ColorVF";
    rasterPipelineInfo.colorTargetStates = {
        ColorTargetStateInfo{
            .format = Format::RGBA8Unorm,

        },
    };
    RasterPipelineShaderStages rasterPipelineStages = {
        .vertexShaderInfo = eastl::make_optional(ShaderInfo{
            .program = vsh->bytecode,
        }),
        .fragmentShaderInfo = eastl::make_optional(ShaderInfo{
            .program = fsh->bytecode,
        })
    };

    RasterPipeline pipeline = mDevice->Create(rasterPipelineInfo, rasterPipelineStages);
    ASSERT_NE(pipeline, RasterPipeline{}); // Check for valid handle
    ASSERT_EQ(mDevice->GetRasterPipelineInfo(pipeline), rasterPipelineInfo);
    mDevice->Destroy(pipeline);
}

TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateAndDestroyRasterPipelineColorBlendVertexFragment) {
    ShaderObject vsh = mShaderCompiler->CompileShaderFromSource( // Changed to CompileShaderFromSource
        gTEST_VERTEX_SHADER,
        {
            .stage = ShaderStage::Vertex,
            .entryPoint = "main",
            .name = "Vsh",
        });
    ShaderObject fsh = mShaderCompiler->CompileShaderFromSource( // Changed to CompileShaderFromSource
        gTEST_FRAGMENT_SHADER,
        {
            .stage = ShaderStage::Fragment,
            .entryPoint = "main",
            .name = "Fsh",
        });

    RasterPipelineInfo rasterPipelineInfo = {};
    rasterPipelineInfo.name = "TestPipeline_ColorBlendVF";
    rasterPipelineInfo.colorTargetStates = {
        ColorTargetStateInfo{
            .format = Format::RGBA8Unorm,
            .blend = eastl::make_optional(BlendInfo{
                .dstColorBlendFactor = BlendFactor::OneMinusSrcAlpha,
                .dstAlphaBlendFactor = BlendFactor::OneMinusSrcAlpha,
            }),
        },
    };
    RasterPipelineShaderStages rasterPipelineStages = {
        .vertexShaderInfo = eastl::make_optional(ShaderInfo{
            .program = vsh->bytecode,
        }),
        .fragmentShaderInfo = eastl::make_optional(ShaderInfo{
            .program = fsh->bytecode,
        })
    };

    RasterPipeline pipeline = mDevice->Create(rasterPipelineInfo, rasterPipelineStages);
    ASSERT_NE(pipeline, RasterPipeline{}); // Check for valid handle
    ASSERT_EQ(mDevice->GetRasterPipelineInfo(pipeline), rasterPipelineInfo);
    mDevice->Destroy(pipeline);
}

TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateAndDestroyRasterPipelineMSAAState) {
    // Check for device support
    if (mDevice->Properties().msaaSupportColorTarget == RasterizationSamples::e1) {
        GTEST_SKIP() << "Device does not support MSAA. Skipping test.";
        return;
    }

    ShaderObject vsh = mShaderCompiler->CompileShaderFromSource( // Changed to CompileShaderFromSource
        gTEST_VERTEX_SHADER,
        {
            .stage = ShaderStage::Vertex,
            .entryPoint = "main",
            .name = "Vsh",
        });
    ShaderObject fsh = mShaderCompiler->CompileShaderFromSource( // Changed to CompileShaderFromSource
        gTEST_FRAGMENT_SHADER,
        {
            .stage = ShaderStage::Fragment,
            .entryPoint = "main",
            .name = "Fsh",
        });

    RasterPipelineInfo rasterPipelineInfo = {};
    rasterPipelineInfo.name = "TestPipeline_MSAA";
    rasterPipelineInfo.colorTargetStates = {
        ColorTargetStateInfo{
            .format = Format::RGBA8Unorm,
        },
    };

    // Use HIGHEST_BIT as requested and correctly set sample shading
    auto msaaSamples = mDevice->Properties().msaaSupportColorTarget;
    auto chosenSamples = HIGHEST_BIT(msaaSamples);

    rasterPipelineInfo.multiSampleState = {
        .sampleCount = chosenSamples,
        .bSampleShading = true,
    };


    RasterPipelineShaderStages rasterPipelineStages = {
        .vertexShaderInfo = eastl::make_optional(ShaderInfo{
            .program = vsh->bytecode,
        }),
        .fragmentShaderInfo = eastl::make_optional(ShaderInfo{
            .program = fsh->bytecode,
        }),
    };

    RasterPipeline pipeline = mDevice->Create(rasterPipelineInfo, rasterPipelineStages);
    ASSERT_NE(pipeline, RasterPipeline{}); // Check for valid handle
    ASSERT_EQ(mDevice->GetRasterPipelineInfo(pipeline), rasterPipelineInfo);
    mDevice->Destroy(pipeline);
}

// --- UPDATED/NEW TESTS START HERE ---

TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateAndDestroyRasterPipelineInputAssembly) {
    ShaderObject vsh = mShaderCompiler->CompileShaderFromSource( // Changed to CompileShaderFromSource
        gTEST_INPUT_ASSEMBLY_VERTEX_SHADER,                      // Use specific shader
        {
            .stage = ShaderStage::Vertex,
            .entryPoint = "main",
            .name = "Vsh_InputAssembly",
        });
    ShaderObject fsh = mShaderCompiler->CompileShaderFromSource( // Changed to CompileShaderFromSource
        gTEST_FRAGMENT_SHADER,                                   // Standard fragment shader is fine
        {
            .stage = ShaderStage::Fragment,
            .entryPoint = "main",
            .name = "Fsh",
        });

    // Define vertex layout (e.g., Position + Normal)
    const eastl::array<VertexAttributeInfo, 2> vertexAttributes = {
        VertexAttributeInfo{
            .location = 0, // Corresponds to POSITION0
            .binding = 0,
            .format = Format::RGB32Sfloat,
            .offset = 0,
        },
        VertexAttributeInfo{
            .location = 1, // Corresponds to NORMAL0
            .binding = 0,
            .format = Format::RGB32Sfloat,
            .offset = 12, // 3 * sizeof(float)
        },
    };

    const eastl::array<VertexBindingInfo, 1> vertexBindings = {
        VertexBindingInfo{
            .binding = 0,
            .stride = 24, // 2 * 3 * sizeof(float)
            .bPerInstance = false,
        },
    };

    RasterPipelineInfo rasterPipelineInfo = {};
    rasterPipelineInfo.name = "TestPipeline_InputAssembly";
    rasterPipelineInfo.colorTargetStates = {
        ColorTargetStateInfo{
            .format = Format::RGBA8Unorm,
        },
    };
    rasterPipelineInfo.inputAssemblyState = {
        .primitiveTopology = PrimitiveTopology::TriangleList,
        .indexType = IndexType::Uint32,
        .bPrimitiveRestart = false,
        .vertexAttributes = vertexAttributes,
        .vertexBindings = vertexBindings,
    };

    RasterPipelineShaderStages rasterPipelineStages = {
        .vertexShaderInfo = eastl::make_optional(ShaderInfo{
            .program = vsh->bytecode,
        }),
        .fragmentShaderInfo = eastl::make_optional(ShaderInfo{
            .program = fsh->bytecode,
        })
    };

    RasterPipeline pipeline = mDevice->Create(rasterPipelineInfo, rasterPipelineStages);
    ASSERT_NE(pipeline, RasterPipeline{}); // Check for valid handle
    ASSERT_EQ(mDevice->GetRasterPipelineInfo(pipeline), rasterPipelineInfo);
    mDevice->Destroy(pipeline);
}

TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateAndDestroyRasterPipelineRasterizerState) {
    ShaderObject vsh = mShaderCompiler->CompileShaderFromSource( // Changed to CompileShaderFromSource
        gTEST_VERTEX_SHADER,
        {
            .stage = ShaderStage::Vertex,
            .entryPoint = "main",
            .name = "Vsh",
        });
    ShaderObject fsh = mShaderCompiler->CompileShaderFromSource( // Changed to CompileShaderFromSource
        gTEST_FRAGMENT_SHADER,
        {
            .stage = ShaderStage::Fragment,
            .entryPoint = "main",
            .name = "Fsh",
        });

    RasterPipelineInfo rasterPipelineInfo = {};
    rasterPipelineInfo.name = "TestPipeline_RasterState";
    rasterPipelineInfo.colorTargetStates = {
        ColorTargetStateInfo{
            .format = Format::RGBA8Unorm,
        },
    };

    // Change rasterizer state from defaults
    rasterPipelineInfo.rasterizerState = {
        .polygonMode = PolygonMode::Line, // Wireframe
        .lineWidth = 1.0f,
        .lineMode = LineMode::Normal,
        .frontFaceWinding = WindingOrder::Clockwise, // Flipped
        .faceCulling = FaceCull::Front,              // Cull front faces
        .bDepthClamp = true,                         // Enable depth clamp
        .bDepthBias = false,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .bRasterizerDiscard = false,
    };

    RasterPipelineShaderStages rasterPipelineStages = {
        .vertexShaderInfo = eastl::make_optional(ShaderInfo{
            .program = vsh->bytecode,
        }),
        .fragmentShaderInfo = eastl::make_optional(ShaderInfo{
            .program = fsh->bytecode,
        })
    };

    RasterPipeline pipeline = mDevice->Create(rasterPipelineInfo, rasterPipelineStages);
    ASSERT_NE(pipeline, RasterPipeline{}); // Check for valid handle
    ASSERT_EQ(mDevice->GetRasterPipelineInfo(pipeline), rasterPipelineInfo);
    mDevice->Destroy(pipeline);
}

TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateAndDestroyRasterPipelineDepthStencilState) {
    ShaderObject vsh = mShaderCompiler->CompileShaderFromSource( // Changed to CompileShaderFromSource
        gTEST_VERTEX_SHADER,
        {
            .stage = ShaderStage::Vertex,
            .entryPoint = "main",
            .name = "Vsh",
        });
    // No fragment shader needed if only writing depth/stencil

    RasterPipelineInfo rasterPipelineInfo = {};
    rasterPipelineInfo.name = "TestPipeline_DepthStencil";

    // Configure both depth and stencil tests
    rasterPipelineInfo.depthStencilState.emplace(DepthStencilStateInfo{
        .depthStencilFormat = Format::D24UnormS8Uint, // Common depth-stencil format
        .depthTestState = DepthStencilTestState::ReadWrite,
        .stencilTestState = DepthStencilTestState::ReadWrite, // Enable stencil
        .depthTest = {
            .compareOp = CompareOp::Greater, // Non-default op
        },
        .frontStencilTest = {
            .failOp = StencilOp::Keep,
            .passOp = StencilOp::IncrementClamp, // Increment on pass
            .depthFailOp = StencilOp::Keep,
            .compareOp = CompareOp::Always,
            .compareMask = 0xFF,
            .writeMask = 0xFF,
            .reference = 1,
        },
        .backStencilTest = {
            .failOp = StencilOp::Keep,
            .passOp = StencilOp::Keep, // Do nothing for back faces
            .depthFailOp = StencilOp::Keep,
            .compareOp = CompareOp::Never,
            .compareMask = 0xFF,
            .writeMask = 0xFF,
            .reference = 0,
        },
    });

    RasterPipelineShaderStages rasterPipelineStages = {
        .vertexShaderInfo = eastl::make_optional(ShaderInfo{
            .program = vsh->bytecode,
        })
        // No fragment shader
    };

    RasterPipeline pipeline = mDevice->Create(rasterPipelineInfo, rasterPipelineStages);
    ASSERT_NE(pipeline, RasterPipeline{}); // Check for valid handle
    ASSERT_EQ(mDevice->GetRasterPipelineInfo(pipeline), rasterPipelineInfo);
    mDevice->Destroy(pipeline);
}

TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateAndDestroyRasterPipelineTessellation) {
    // Check for device support
    if (!mDevice->Features().bTesselationShaders) {
        GTEST_SKIP() << "Device does not support Tesselation Shaders. Skipping test.";
        return;
    }

    // Compile all stages from the single integration shader string
    ShaderObject vsh = mShaderCompiler->CompileShaderFromSource(
        gEXEC_TESS_SHADER,
        {
            .stage = ShaderStage::Vertex,
            .entryPoint = "vertexMain",
            .name = "Vsh_Tess",
        });
    ShaderObject hsh = mShaderCompiler->CompileShaderFromSource(
        gEXEC_TESS_SHADER,
        {
            .stage = ShaderStage::Hull,
            .entryPoint = "hullMain",
            .name = "Hsh_Tess",
        });
    ShaderObject dsh = mShaderCompiler->CompileShaderFromSource(
        gEXEC_TESS_SHADER,
        {
            .stage = ShaderStage::Domain,
            .entryPoint = "domainMain",
            .name = "Dsh_Tess",
        });
    ShaderObject fsh = mShaderCompiler->CompileShaderFromSource(
        gEXEC_TESS_SHADER,
        {
            .stage = ShaderStage::Fragment,
            .entryPoint = "fragmentMain",
            .name = "Fsh_Tess",
        });

    // Check that all shaders compiled successfully
    ASSERT_TRUE(vsh && hsh && dsh && fsh) << "Failed to compile one or more tessellation shaders";

    RasterPipelineInfo rasterPipelineInfo = {};
    rasterPipelineInfo.name = "TestPipeline_Tessellation";
    rasterPipelineInfo.colorTargetStates = {
        ColorTargetStateInfo{
            .format = Format::RGBA8Unorm,
        },
    };

    // Must use PatchList topology for tessellation
    rasterPipelineInfo.inputAssemblyState.primitiveTopology = PrimitiveTopology::PatchList;

    rasterPipelineInfo.tesselationState.emplace(TesselationStateInfo{
        .controlPoints = 3, // Corresponds to InputPatch<VSOut, 3>
        .origin = TesselationDomainOrigin::LowerLeft,
    });

    RasterPipelineShaderStages rasterPipelineStages = {
        .vertexShaderInfo = eastl::make_optional(ShaderInfo{
            .program = vsh->bytecode,
        }),
        .hullShaderInfo = eastl::make_optional(ShaderInfo{
            .program = hsh->bytecode,
        }),
        .domainShaderInfo = eastl::make_optional(ShaderInfo{
            .program = dsh->bytecode,
        }),
        .fragmentShaderInfo = eastl::make_optional(ShaderInfo{
            .program = fsh->bytecode,
        })
    };

    RasterPipeline pipeline = mDevice->Create(rasterPipelineInfo, rasterPipelineStages);
    ASSERT_NE(pipeline, RasterPipeline{}); // Check for valid handle
    ASSERT_EQ(mDevice->GetRasterPipelineInfo(pipeline), rasterPipelineInfo);
    mDevice->Destroy(pipeline);
}

TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateAndDestroyRasterPipelineGeometry) {
    // Check for device support
    if (!mDevice->Features().bGeometryShaders) {
        GTEST_SKIP() << "Device does not support Geometry Shaders. Skipping test.";
        return;
    }

    // Compile all stages from the single integration shader string
    ShaderObject vsh = mShaderCompiler->CompileShaderFromSource(
        gEXEC_GEOMETRY_SHADER,
        {
            .stage = ShaderStage::Vertex,
            .entryPoint = "vertexMain",
            .name = "Vsh_Geom",
        });
    ShaderObject gsh = mShaderCompiler->CompileShaderFromSource(
        gEXEC_GEOMETRY_SHADER,
        {
            .stage = ShaderStage::Geometry,
            .entryPoint = "geometryMain",
            .name = "Gsh_Geom",
        });
    ShaderObject fsh = mShaderCompiler->CompileShaderFromSource(
        gEXEC_GEOMETRY_SHADER,
        {
            .stage = ShaderStage::Fragment,
            .entryPoint = "fragmentMain",
            .name = "Fsh_Geom",
        });

    // Check that all shaders compiled successfully
    ASSERT_TRUE(vsh && gsh && fsh) << "Failed to compile one or more geometry shaders";

    RasterPipelineInfo rasterPipelineInfo = {};
    rasterPipelineInfo.name = "TestPipeline_Geometry";
    rasterPipelineInfo.colorTargetStates = {
        ColorTargetStateInfo{
            .format = Format::RGBA8Unorm,
        },
    };

    // VS in gEXEC_GEOMETRY_SHADER outputs 3 vertices
    rasterPipelineInfo.inputAssemblyState.primitiveTopology = PrimitiveTopology::TriangleList;

    RasterPipelineShaderStages rasterPipelineStages = {
        .vertexShaderInfo = eastl::make_optional(ShaderInfo{
            .program = vsh->bytecode,
        }),
        .geometryShaderInfo = eastl::make_optional(ShaderInfo{
            .program = gsh->bytecode,
        }),
        .fragmentShaderInfo = eastl::make_optional(ShaderInfo{
            .program = fsh->bytecode,
        })
    };

    RasterPipeline pipeline = mDevice->Create(rasterPipelineInfo, rasterPipelineStages);
    ASSERT_NE(pipeline, RasterPipeline{}); // Check for valid handle
    ASSERT_EQ(mDevice->GetRasterPipelineInfo(pipeline), rasterPipelineInfo);
    mDevice->Destroy(pipeline);
}

TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateAndDestroyComputePipelineRayQuery) {
    // Check for device support for Ray Queries
    if (!mDevice->Features().bRayQueries) {
        GTEST_SKIP() << "Device does not support Ray Queries. Skipping test.";
        return;
    }

    u32 minimumSmTier = mApi.loadedContext->GetMinimumShaderModelFeatureTier(ShaderModelFeatureBits::RAY_QUERY);
    if (mDevice->Features().maxSupportedShaderModel < minimumSmTier) {
        GTEST_SKIP() << "Device does not support Shader Model " << std::hex << minimumSmTier << ". Skipping test.";
        return;
    }
    mDevice->SetShaderModel(minimumSmTier);

    // Compile the compute shader with ray query operations
    ShaderObject csh = mShaderCompiler->CompileShaderFromSource(
        gTEST_RAY_QUERY_COMPUTE_SHADER,
        {
            .stage = ShaderStage::Compute,
            .entryPoint = "main",
            .name = "Csh_RayQuery",
        });

    // Ensure the shader itself compiled successfully
    ASSERT_TRUE(csh) << "Failed to compile ray query compute shader. Check Slang errors.";

    ComputePipelineInfo computePipelineInfo = {};
    computePipelineInfo.name = "TestPipeline_RayQuery";

    ShaderInfo computeShaderInfo = {
        .program = csh->bytecode,
    };

    // Create the compute pipeline
    ComputePipeline pipeline = mDevice->Create(computePipelineInfo, computeShaderInfo);

    ASSERT_NE(pipeline, ComputePipeline{}); // Check for a valid (non-null) handle
    ASSERT_EQ(mDevice->GetComputePipelineInfo(pipeline), computePipelineInfo);

    mDevice->Destroy(pipeline);
}
