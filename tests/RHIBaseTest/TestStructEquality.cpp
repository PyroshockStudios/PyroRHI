#include "gtest/gtest.h"
#include <PyroRHI/Api/Types.hpp>

using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;

TEST(ColorClearValueEquality, EqualValues) {
    ColorClearValue a = {};
    ColorClearValue b = {};
    a.int32[0] = 10;
    a.int32[1] = 20;
    a.int32[2] = 30;
    a.int32[3] = 40;
    b.int32[0] = 10;
    b.int32[1] = 20;
    b.int32[2] = 30;
    b.int32[3] = 40;
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

TEST(ColorClearValueEquality, UnequalValues) {
    ColorClearValue a = {};
    ColorClearValue b = {};
    a.int32[0] = 10;
    a.int32[1] = 20;
    a.int32[2] = 30;
    a.int32[3] = 40;
    b.int32[0] = 99;
    b.int32[1] = 20;
    b.int32[2] = 30;
    b.int32[3] = 40;
    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a != b);
}

TEST(ColorClearValueEquality, FloatAndIntSameBits) {
    ColorClearValue a = {};
    ColorClearValue b = {};
    a.float32[0] = 1.0f;
    a.float32[1] = 0.0f;
    a.float32[2] = 0.0f;
    a.float32[3] = 1.0f;
    b.int32[0] = a.int32[0];
    b.int32[1] = a.int32[1];
    b.int32[2] = a.int32[2];
    b.int32[3] = a.int32[3];
    EXPECT_TRUE(a == b);
}

TEST(DepthStencilClearValueEquality, EqualValues) {
    DepthStencilClearValue a = { 1.0f, 255 };
    DepthStencilClearValue b = { 1.0f, 255 };
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

TEST(DepthStencilClearValueEquality, UnequalDepth) {
    DepthStencilClearValue a = { 1.0f, 0 };
    DepthStencilClearValue b = { 0.5f, 0 };
    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a != b);
}

TEST(DepthStencilClearValueEquality, UnequalStencil) {
    DepthStencilClearValue a = { 1.0f, 0 };
    DepthStencilClearValue b = { 1.0f, 255 };
    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a != b);
}

TEST(Offset2DComparison, Equal) {
    Offset2D a = { 10, 20 };
    Offset2D b = { 10, 20 };
    EXPECT_EQ(a <=> b, std::strong_ordering::equal);
}

TEST(Offset2DComparison, Less) {
    Offset2D a = { 5, 20 };
    Offset2D b = { 10, 20 };
    EXPECT_EQ(a <=> b, std::strong_ordering::less);
}

TEST(Offset2DComparison, Greater) {
    Offset2D a = { 15, 20 };
    Offset2D b = { 10, 20 };
    EXPECT_EQ(a <=> b, std::strong_ordering::greater);
}

TEST(Offset3DComparison, Equal) {
    Offset3D a = { 1, 2, 3 };
    Offset3D b = { 1, 2, 3 };
    EXPECT_EQ(a <=> b, std::strong_ordering::equal);
}

TEST(Offset3DComparison, Less) {
    Offset3D a = { 0, 0, 0 };
    Offset3D b = { 1, 2, 3 };
    EXPECT_EQ(a <=> b, std::strong_ordering::less);
}

TEST(Extent2DComparison, Equal) {
    Extent2D a = { 1920, 1080 };
    Extent2D b = { 1920, 1080 };
    EXPECT_EQ(a <=> b, std::strong_ordering::equal);
}

TEST(Extent2DComparison, Less) {
    Extent2D a = { 800, 600 };
    Extent2D b = { 1920, 1080 };
    EXPECT_EQ(a <=> b, std::strong_ordering::less);
}

TEST(Extent3DComparison, Equal) {
    Extent3D a = { 100, 200, 3 };
    Extent3D b = { 100, 200, 3 };
    EXPECT_EQ(a <=> b, std::strong_ordering::equal);
}

TEST(Extent3DComparison, Less) {
    Extent3D a = { 10, 20, 1 };
    Extent3D b = { 100, 200, 3 };
    EXPECT_EQ(a <=> b, std::strong_ordering::less);
}

TEST(ViewportInfoComparison, Equal) {
    ViewportInfo a = { 0.0f, 0.0f, 800.0f, 600.0f, 0.0f, 1.0f };
    ViewportInfo b = { 0.0f, 0.0f, 800.0f, 600.0f, 0.0f, 1.0f };
    EXPECT_EQ(a <=> b, std::strong_ordering::equal);
}

TEST(ViewportInfoComparison, Different) {
    ViewportInfo a = { 0.0f, 0.0f, 800.0f, 600.0f, 0.0f, 1.0f };
    ViewportInfo b = { 0.0f, 0.0f, 1024.0f, 768.0f, 0.0f, 1.0f };
    EXPECT_NE(a <=> b, std::strong_ordering::equal);
}

TEST(Rect2DComparison, Equal) {
    Rect2D a = { 0, 0, 100, 100 };
    Rect2D b = { 0, 0, 100, 100 };
    EXPECT_EQ(a <=> b, std::strong_ordering::equal);
}

TEST(Rect2DComparison, Less) {
    Rect2D a = { 0, 0, 50, 50 };
    Rect2D b = { 0, 0, 100, 100 };
    EXPECT_EQ(a <=> b, std::strong_ordering::less);
}

TEST(Box3DComparison, Equal) {
    Box3D a = { 0, 0, 0, 10, 20, 30 };
    Box3D b = { 0, 0, 0, 10, 20, 30 };
    EXPECT_EQ(a <=> b, std::strong_ordering::equal);
}

TEST(Box3DComparison, Different) {
    Box3D a = { 0, 0, 0, 10, 20, 30 };
    Box3D b = { 5, 10, 15, 100, 50, 25 };
    EXPECT_NE(a <=> b, std::strong_ordering::equal);
}

TEST(BufferRegionComparison, Equal) {
    BufferRegion a = { 1024, 512 };
    BufferRegion b = { 1024, 512 };
    EXPECT_EQ(a <=> b, std::strong_ordering::equal);
}

TEST(BufferRegionComparison, Less) {
    BufferRegion a = { 512, 256 };
    BufferRegion b = { 1024, 512 };
    EXPECT_EQ(a <=> b, std::strong_ordering::less);
}

TEST(ImageMipArraySliceComparison, Equal) {
    ImageMipArraySlice a = { 0, 1, 0, 1 };
    ImageMipArraySlice b = { 0, 1, 0, 1 };
    EXPECT_EQ(a <=> b, std::strong_ordering::equal);
}

TEST(ImageMipArraySliceComparison, Different) {
    ImageMipArraySlice a = { 0, 1, 0, 1 };
    ImageMipArraySlice b = { 2, 3, 5, 6 };
    EXPECT_NE(a <=> b, std::strong_ordering::equal);
}

TEST(ImageArraySliceComparison, Equal) {
    ImageArraySlice a = { 0, 0, 1 };
    ImageArraySlice b = { 0, 0, 1 };
    EXPECT_EQ(a <=> b, std::strong_ordering::equal);
}

TEST(ImageSliceComparison, Equal) {
    ImageSlice a = { 0, 0 };
    ImageSlice b = { 0, 0 };
    EXPECT_EQ(a <=> b, std::strong_ordering::equal);
}

TEST(ImageSliceComparison, Less) {
    ImageSlice a = { 0, 0 };
    ImageSlice b = { 1, 1 };
    EXPECT_EQ(a <=> b, std::strong_ordering::less);
}
