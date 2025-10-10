#include "Helpers/ValidationFixture.hpp"

using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;

TEST_F(RHI_CONTEXT_FIXTURE_NAME, CreateAndDestroyImage) {
    ImageInfo info = {};
    info.size = { 256, 256, 1 };
    info.format = Format::RGBA8Unorm;
    info.usage = ImageUsageFlagBits::RENDER_TARGET;
    info.name = "TestImage";

    Image image = mDevice->CreateImage(info);
    ASSERT_TRUE(mDevice->IsImageValid(image));

    const ImageInfo& queried = mDevice->GetImageInfo(image);
    EXPECT_EQ(queried, info);


    mDevice->DestroyImage(image);
    EXPECT_FALSE(mDevice->IsImageValid(image));
}
