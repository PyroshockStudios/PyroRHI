#include "Helpers/ValidationFixture.hpp"
#include <ostream>

using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;
using namespace ::testing;

struct ImageCopyFuzzParams {
    eastl::string testName;
    Format format;
    Extent3D imageSize;
    ImageDimensions dimensions;
    u32 totalMipLevels;
    u32 totalArrayLayers;

    u32 targetMip;
    u32 targetLayer;
    Offset3D copyOffset;
    Extent3D copyExtent;

    // Google Test will automatically look for this operator when printing failures
    friend std::ostream& operator<<(std::ostream& os, const ImageCopyFuzzParams& p) {
        os << "{\n"
           << "  Base Size: [" << p.imageSize.width << ", " << p.imageSize.height << ", " << p.imageSize.depth << "]\n"
           << "  Mip/Layer: [" << p.targetMip << " / " << p.targetLayer << "]\n"
           << "  Copy Offset: [" << p.copyOffset.x << ", " << p.copyOffset.y << ", " << p.copyOffset.z << "]\n"
           << "  Copy Extent: [" << p.copyExtent.width << ", " << p.copyExtent.height << ", " << p.copyExtent.depth << "]\n"
           << "}";
        return os;
    }
};

class RHI_ImageCopyFuzzing : public RHI_CONTEXT_FIXTURE_NAME,
                             public WithParamInterface<ImageCopyFuzzParams> {
};

TEST_P(RHI_ImageCopyFuzzing, UploadStressTest) {
    const ImageCopyFuzzParams& params = GetParam();

    ImageInfo info = {};
    info.size = params.imageSize;
    info.dimensions = params.dimensions;
    info.format = params.format;
    info.mipLevelCount = params.totalMipLevels;
    info.arrayLayerCount = params.totalArrayLayers;
    info.usage = ImageUsageFlagBits::TRANSFER_DST;
    info.name = params.testName;

    TRACK_RHI_PARAMETER(info);
    Image image = mDevice->CreateImage(info);
    ASSERT_TRUE(mDevice->IsImageValid(image));
    TRACK_RHI_HANDLE(image);

    // get upload requirements for the specific mip/layer slice
    ImageUploadSlice uploadSlice = mDevice->ImageUploadRequirements(
        image,
        ImageSlice{ .mipLevel = params.targetMip, .arrayLayer = params.targetLayer });
    TRACK_RHI_PARAMETER(uploadSlice);

    BufferInfo bufferInfo = {};
    bufferInfo.size = uploadSlice.size;
    bufferInfo.usage = BufferUsageFlagBits::TRANSFER_SRC;
    bufferInfo.allocationDomain = MemoryAllocationDomain::HostStaging;
    bufferInfo.initialLayout = BufferLayout::TransferSrc;
    bufferInfo.name = "FuzzStagingBuffer";

    TRACK_RHI_PARAMETER(bufferInfo);
    Buffer stagingBuffer = mDevice->CreateBuffer(bufferInfo);
    ASSERT_TRUE(mDevice->IsBufferValid(stagingBuffer));
    TRACK_RHI_HANDLE(stagingBuffer);

    ICommandQueue* queue = mDevice->GetCommandQueues()[0];
    TRACK_RHI_HANDLE(queue);
    ICommandBuffer* commands = queue->GetCommandBuffer({ .name = "Staging commands" });
    TRACK_RHI_HANDLE(commands);

    ImageMemoryBarrierInfo barrierInfo{
        .image = image,
        .dstAccess = AccessConsts::TRANSFER_WRITE,
        .dstLayout = ImageLayout::TransferDst,
    };
    TRACK_RHI_PARAMETER(barrierInfo);
    commands->ImageBarrier(barrierInfo);

    CopyBufferToImageInfo copyInfo{
        .buffer = stagingBuffer,
        .bufferOffset = 0,
        .image = image,
        .imageSlice = {
            .mipLevel = params.targetMip,
            .baseArrayLayer = params.targetLayer,
            .layerCount = 1,
        },
        .imageOffset = params.copyOffset,
        .imageExtent = params.copyExtent,
        .rowPitch = uploadSlice.uploadPitch,
    };

    TRACK_RHI_PARAMETER(copyInfo);
    commands->CopyBufferToImage(copyInfo);
    commands->Complete();

    mDevice->SubmitQueue({ .queue = queue, .commands = { &commands, 1 } });
    queue->WaitIdle();

    mDevice->Destroy(stagingBuffer);
    mDevice->Destroy(image);
}

INSTANTIATE_TEST_SUITE_P(
    BufferToImageCopies,
    RHI_ImageCopyFuzzing,
    Values(
        // -------------------------------------------------------------------------
        // 1. NON-POWER OF TWO (NPOT)
        // -------------------------------------------------------------------------
        ImageCopyFuzzParams{
            "NPOT_OddDimensions",
            Format::RGBA8Unorm,
            { 137, 63, 1 },
            ImageDimensions::e2D,
            1,
            1,
            0,
            0,
            { 0, 0, 0 },
            { 137, 63, 1 },
        },
        ImageCopyFuzzParams{
            "NPOT_PrimeNumbers",
            Format::R8Unorm,
            { 257, 127, 1 },
            ImageDimensions::e2D,
            1,
            1,
            0,
            0,
            { 0, 0, 0 },
            { 257, 127, 1 },
        },

        // -------------------------------------------------------------------------
        // 2. BLOCK COMPRESSED FORMATS (BC1/BC3/etc.)
        // Block compressed formats require specific row pitch alignments and
        // dimensions are often padded to 4x4 blocks internally.
        // -------------------------------------------------------------------------
        ImageCopyFuzzParams{
            "Compressed_BC1_Standard",
            Format::BC1RGBUnormBlock,
            { 256, 256, 1 },
            ImageDimensions::e2D,
            1,
            1,
            0,
            0,
            { 0, 0, 0 },
            { 256, 256, 1 },
        },
        ImageCopyFuzzParams{
            "Compressed_BC3_NPOT", Format::BC3UnormBlock,
            { 130, 62, 1 }, ImageDimensions::e2D, 1, 1,
            0, 0, { 0, 0, 0 }, { 130, 62, 1 }, // Driver must handle block padding internally
        },

        // -------------------------------------------------------------------------
        // 3. MIP LEVELS
        // Uploading to a tiny 1x1 or 2x2 mip level deep in the chain.
        // -------------------------------------------------------------------------
        ImageCopyFuzzParams{
            "Mip_DeepLevel_1x1",
            Format::RGBA8Unorm,
            { 1024, 1024, 1 },
            ImageDimensions::e2D,
            11,
            1, // 1024x1024 goes down to 1x1 at mip 10
            10,
            0,
            { 0, 0, 0 },
            { 1, 1, 1 },
        },
        ImageCopyFuzzParams{
            "Mip_MidLevel_NPOT", Format::RGBA8Unorm,
            { 1023, 1024, 1 }, ImageDimensions::e2D, 11, 1,
            3, 0, { 0, 0, 0 }, { 127, 128, 1 }, // Mip 3 is 127x128
        },

        // -------------------------------------------------------------------------
        // 4. OFFSETS AND PARTIAL COPIES
        // Uploading data into a sub-region of the image.
        // -------------------------------------------------------------------------
        ImageCopyFuzzParams{
            "Offset_CenterRegion", Format::RGBA8Unorm,
            { 512, 512, 1 }, ImageDimensions::e2D, 1, 1,
            0, 0,
            { 128, 128, 0 }, // Offset into the image
            { 256, 256, 1 }, // Extent of the copy (filling the center)
        },
        ImageCopyFuzzParams{
            "Offset_UnalignedEdge",
            Format::RGBA8Unorm,
            { 256, 256, 1 },
            ImageDimensions::e2D,
            1,
            1,
            0,
            0,
            { 13, 17, 0 }, // Weird offset
            { 64, 64, 1 },
        },

        // -------------------------------------------------------------------------
        // 5. TEXTURE ARRAYS & 3D TEXTURES
        // -------------------------------------------------------------------------
        ImageCopyFuzzParams{
            "Array_DeepLayer",
            Format::RGBA8Unorm,
            { 256, 256, 1 },
            ImageDimensions::e2D,
            1,
            16, // 16 layers
            0,
            15, // Uploading to the last layer
            { 0, 0, 0 },
            { 256, 256, 1 },
        },
        ImageCopyFuzzParams{
            "Volume_3D_Texture", Format::RGBA8Unorm,
            { 64, 64, 64 }, ImageDimensions::e3D, 1, 1, // 3D texture
            0, 0,
            { 0, 0, 16 }, { 64, 64, 16 }, // Copying a "slab" into the middle of the 3D volume
        },
        ImageCopyFuzzParams{
            "ExtremeAspect_Wide",
            Format::RGBA8Unorm,
            { 8192, 1, 1 },
            ImageDimensions::e2D,
            1,
            1,
            0,
            0,
            { 0, 0, 0 },
            { 8192, 1, 1 },
        },
        ImageCopyFuzzParams{
            "ExtremeAspect_Tall",
            Format::RGBA8Unorm,
            { 1, 8192, 1 },
            ImageDimensions::e2D,
            1,
            1,
            0,
            0,
            { 0, 0, 0 },
            { 1, 8192, 1 },
        },

        // -------------------------------------------------------------------------
        // 7. COMPRESSION + MIP LEVELS + NPOT
        // The ultimate test for block padding logic.
        // Base is 137x63. Mip 1 halves and floors to 68x31.
        // Both dimensions are not perfectly divisible by 4, forcing the driver
        // to handle partial blocks at the edges.
        // -------------------------------------------------------------------------
        ImageCopyFuzzParams{
            "Compressed_BC1_Mip_NPOT",
            Format::BC1RGBUnormBlock,
            { 137, 63, 1 },
            ImageDimensions::e2D,
            3,
            1,
            1,
            0,
            { 0, 0, 0 },
            { 68, 31, 1 },
        },

        // -------------------------------------------------------------------------
        // 8. COMPRESSION + SUB-REGION OFFSETS
        // Vulkan SPEC requires imageOffset to be a multiple of the block size (4x4).
        // Extents must also be a multiple of the block size unless touching the edge.
        // -------------------------------------------------------------------------
        ImageCopyFuzzParams{
            "Compressed_BC3_SubRegion", Format::BC3UnormBlock,
            { 256, 256, 1 }, ImageDimensions::e2D, 1, 1,
            0, 0,
            { 16, 24, 0 }, // Valid offset (Multiple of 4)
            { 64, 32, 1 }, // Valid extent (Multiple of 4)
        },
        ImageCopyFuzzParams{
            "Compressed_BC1_SingleBlock", Format::BC1RGBUnormBlock,
            { 1024, 1024, 1 }, ImageDimensions::e2D, 1, 1,
            0, 0,
            { 512, 512, 0 }, // Deep in the texture
            { 4, 4, 1 },     // Exactly one BC1 block (8 bytes total)
        },

        // -------------------------------------------------------------------------
        // 9. THE KITCHEN SINK (3D + Mip + NPOT + Offset)
        // Base: 257 x 127 x 63. Target: Mip 2.
        // Mip 0: 257x127x63  |  Mip 1: 128x63x31  |  Mip 2: 64x31x15
        // -------------------------------------------------------------------------
        ImageCopyFuzzParams{
            "Volume_3D_Mip_Subregion_NPOT", Format::R8Unorm, // 1-byte pixel highlights alignment issues easily
            { 257, 127, 63 }, ImageDimensions::e3D, 8, 1,
            2, 0,
            { 16, 8, 4 },  // Offset into Mip 2
            { 32, 15, 6 }, // Extent matching a sub-cube inside Mip 2
        },

        // -------------------------------------------------------------------------
        // 10. LARGE PIXEL FORMATS
        // Tests math limits for buffer allocations and row pitch when texels are huge.
        // RGBA32Float is 16 bytes per texel.
        // -------------------------------------------------------------------------
        ImageCopyFuzzParams{
            "Format_Large_RGBA32Float",
            Format::RGBA32Sfloat, 
            { 1023, 1023, 1 },
            ImageDimensions::e2D,
            1,
            1,
            0,
            0,
            { 0, 0, 0 },
            { 1023, 1023, 1 },
        },

        // -------------------------------------------------------------------------
        // 11. TINY OPERATIONS
        // -------------------------------------------------------------------------
        ImageCopyFuzzParams{
            "SinglePixel_Offset", Format::RGBA8Unorm,
            { 1024, 1024, 1 }, ImageDimensions::e2D, 1, 1,
            0, 0,
            { 511, 733, 0 }, // Random weird coordinate
            { 1, 1, 1 },     // Just uploading 4 bytes!
        }),
    [](const TestParamInfo<ImageCopyFuzzParams>& info) {
        return std::string(info.param.testName.c_str());
    });