#include "gtest/gtest.h"
#include <PyroRHI/Api/Types.hpp>

using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;

TEST(ImageMipArraySlice, Contains_SelfContains) {
    ImageMipArraySlice slice = { 0, 3, 0, 6 };
    EXPECT_TRUE(slice.Contains(slice));
}

TEST(ImageMipArraySlice, Contains_SubSlice) {
    ImageMipArraySlice outer = { 0, 5, 0, 10 };
    ImageMipArraySlice inner = { 1, 3, 2, 5 };
    EXPECT_TRUE(outer.Contains(inner));
}

TEST(ImageMipArraySlice, Contains_ExactMatch) {
    ImageMipArraySlice a = { 2, 3, 4, 5 };
    EXPECT_TRUE(a.Contains(a));
}

TEST(ImageMipArraySlice, Contains_NotContainedMipExceeds) {
    ImageMipArraySlice outer = { 0, 3, 0, 6 };
    ImageMipArraySlice inner = { 2, 2, 0, 6 };
    EXPECT_FALSE(outer.Contains(inner));
}

TEST(ImageMipArraySlice, Contains_NotContainedLayerExceeds) {
    ImageMipArraySlice outer = { 0, 3, 0, 6 };
    ImageMipArraySlice inner = { 0, 3, 4, 4 };
    EXPECT_FALSE(outer.Contains(inner));
}

TEST(ImageMipArraySlice, Contains_CompletelyDisjoint) {
    ImageMipArraySlice a = { 0, 3, 0, 6 };
    ImageMipArraySlice b = { 5, 3, 10, 6 };
    EXPECT_FALSE(a.Contains(b));
}

TEST(ImageMipArraySlice, Intersects_Overlapping) {
    ImageMipArraySlice a = { 0, 5, 0, 10 };
    ImageMipArraySlice b = { 3, 5, 5, 10 };
    EXPECT_TRUE(a.Intersects(b));
    EXPECT_TRUE(b.Intersects(a));
}

TEST(ImageMipArraySlice, Intersects_AdjacentMipsNotOverlapping) {
    ImageMipArraySlice a = { 0, 3, 0, 6 };
    ImageMipArraySlice b = { 3, 3, 0, 6 };
    EXPECT_FALSE(a.Intersects(b));
    EXPECT_FALSE(b.Intersects(a));
}

TEST(ImageMipArraySlice, Intersects_AdjacentLayersNotOverlapping) {
    ImageMipArraySlice a = { 0, 3, 0, 6 };
    ImageMipArraySlice b = { 0, 3, 6, 6 };
    EXPECT_FALSE(a.Intersects(b));
}

TEST(ImageMipArraySlice, Intersects_Subset) {
    ImageMipArraySlice outer = { 0, 5, 0, 10 };
    ImageMipArraySlice inner = { 1, 2, 3, 4 };
    EXPECT_TRUE(outer.Intersects(inner));
    EXPECT_TRUE(inner.Intersects(outer));
}

TEST(ImageMipArraySlice, Intersects_CompletelyDisjoint) {
    ImageMipArraySlice a = { 0, 3, 0, 6 };
    ImageMipArraySlice b = { 5, 3, 10, 6 };
    EXPECT_FALSE(a.Intersects(b));
    EXPECT_FALSE(b.Intersects(a));
}

TEST(ImageMipArraySlice, Intersect_Overlapping) {
    ImageMipArraySlice a = { 0, 5, 0, 10 };
    ImageMipArraySlice b = { 3, 5, 5, 10 };
    ImageMipArraySlice result = a.Intersect(b);
    EXPECT_EQ(result.baseMipLevel, 3u);
    EXPECT_EQ(result.levelCount, 2u);
    EXPECT_EQ(result.baseArrayLayer, 5u);
    EXPECT_EQ(result.layerCount, 5u);
}

TEST(ImageMipArraySlice, Intersect_NoIntersection) {
    ImageMipArraySlice a = { 0, 3, 0, 6 };
    ImageMipArraySlice b = { 5, 3, 10, 6 };
    ImageMipArraySlice result = a.Intersect(b);
    EXPECT_EQ(result.baseMipLevel, 0u);
    EXPECT_EQ(result.levelCount, 0u);
    EXPECT_EQ(result.baseArrayLayer, 0u);
    EXPECT_EQ(result.layerCount, 0u);
}

TEST(ImageMipArraySlice, Intersect_Subset) {
    ImageMipArraySlice outer = { 0, 10, 0, 20 };
    ImageMipArraySlice inner = { 3, 4, 7, 5 };
    ImageMipArraySlice result = outer.Intersect(inner);
    EXPECT_EQ(result.baseMipLevel, 3u);
    EXPECT_EQ(result.levelCount, 4u);
    EXPECT_EQ(result.baseArrayLayer, 7u);
    EXPECT_EQ(result.layerCount, 5u);
}

TEST(ImageMipArraySlice, Subtract_NoIntersection) {
    ImageMipArraySlice region = { 0, 5, 0, 10 };
    ImageMipArraySlice slice = { 10, 5, 20, 10 };
    auto [result, count] = region.Subtract(slice);
    ASSERT_EQ(count, 1u);
    EXPECT_EQ(result[0].baseMipLevel, 0u);
    EXPECT_EQ(result[0].levelCount, 5u);
    EXPECT_EQ(result[0].baseArrayLayer, 0u);
    EXPECT_EQ(result[0].layerCount, 10u);
}

TEST(ImageMipArraySlice, Subtract_CompleteOverlap) {
    ImageMipArraySlice region = { 2, 3, 4, 5 };
    ImageMipArraySlice slice = { 0, 10, 0, 20 };
    auto [result, count] = region.Subtract(slice);
    EXPECT_EQ(count, 0u);
}

TEST(ImageMipArraySlice, Subtract_SliceMipsBeforeAndAfter) {
    ImageMipArraySlice region = { 0, 5, 0, 10 };
    ImageMipArraySlice slice = { 2, 1, 0, 10 };
    auto [result, count] = region.Subtract(slice);
    ASSERT_EQ(count, 2u);
    EXPECT_EQ(result[0].baseMipLevel, 0u);
    EXPECT_EQ(result[0].levelCount, 2u);
    EXPECT_EQ(result[0].baseArrayLayer, 0u);
    EXPECT_EQ(result[0].layerCount, 10u);
    EXPECT_EQ(result[1].baseMipLevel, 3u);
    EXPECT_EQ(result[1].levelCount, 2u);
    EXPECT_EQ(result[1].baseArrayLayer, 0u);
    EXPECT_EQ(result[1].layerCount, 10u);
}

TEST(ImageMipArraySlice, Subtract_SliceLayersBeforeAndAfter) {
    ImageMipArraySlice region = { 0, 5, 0, 10 };
    ImageMipArraySlice slice = { 0, 5, 3, 4 };
    auto [result, count] = region.Subtract(slice);
    ASSERT_EQ(count, 2u);
    EXPECT_EQ(result[0].baseMipLevel, 0u);
    EXPECT_EQ(result[0].levelCount, 5u);
    EXPECT_EQ(result[0].baseArrayLayer, 0u);
    EXPECT_EQ(result[0].layerCount, 3u);
    EXPECT_EQ(result[1].baseMipLevel, 0u);
    EXPECT_EQ(result[1].levelCount, 5u);
    EXPECT_EQ(result[1].baseArrayLayer, 7u);
    EXPECT_EQ(result[1].layerCount, 3u);
}

TEST(ImageMipArraySlice, Subtract_FullCarve) {
    ImageMipArraySlice region = { 0, 6, 0, 8 };
    ImageMipArraySlice slice = { 2, 2, 3, 2 };
    auto [result, count] = region.Subtract(slice);
    ASSERT_EQ(count, 4u);
}

TEST(ImageMipArraySlice, Slice_ToImageArraySlice) {
    ImageMipArraySlice mipSlice = { 2, 5, 3, 10 };
    ImageArraySlice arraySlice = mipSlice.Slice(1);
    EXPECT_EQ(arraySlice.mipLevel, 3u);
    EXPECT_EQ(arraySlice.baseArrayLayer, 3u);
    EXPECT_EQ(arraySlice.layerCount, 10u);

    ImageArraySlice arraySlice2 = mipSlice.Slice(0);
    EXPECT_EQ(arraySlice2.mipLevel, 2u);
}

TEST(ImageMipArraySlice, Slice_ToImageSlice) {
    ImageMipArraySlice mipSlice = { 2, 5, 3, 10 };
    ImageSlice imgSlice = mipSlice.Slice(1, 2);
    EXPECT_EQ(imgSlice.mipLevel, 3u);
    EXPECT_EQ(imgSlice.arrayLayer, 5u);

    ImageSlice imgSlice2 = mipSlice.Slice(0, 0);
    EXPECT_EQ(imgSlice2.mipLevel, 2u);
    EXPECT_EQ(imgSlice2.arrayLayer, 3u);
}

TEST(ImageArraySlice, ContainedIn_True) {
    ImageMipArraySlice mipSlice = { 2, 5, 3, 10 };
    ImageArraySlice arraySlice = { 3, 4, 6 };
    EXPECT_TRUE(arraySlice.ContainedIn(mipSlice));
}

TEST(ImageArraySlice, ContainedIn_MipOutOfRange) {
    ImageMipArraySlice mipSlice = { 2, 5, 3, 10 };
    ImageArraySlice arraySlice = { 7, 4, 6 };
    EXPECT_FALSE(arraySlice.ContainedIn(mipSlice));
}

TEST(ImageArraySlice, ContainedIn_MipAtBoundary) {
    ImageMipArraySlice mipSlice = { 2, 5, 3, 10 };
    ImageArraySlice arraySlice = { 6, 4, 6 };
    EXPECT_TRUE(arraySlice.ContainedIn(mipSlice));
}

TEST(ImageArraySlice, ContainedIn_LayersExceed) {
    ImageMipArraySlice mipSlice = { 2, 5, 3, 10 };
    ImageArraySlice arraySlice = { 3, 8, 6 };
    EXPECT_FALSE(arraySlice.ContainedIn(mipSlice));
}

TEST(ImageArraySlice, Slice_ToImageSlice) {
    ImageArraySlice arraySlice = { 3, 4, 10 };
    ImageSlice imgSlice = arraySlice.Slice(2);
    EXPECT_EQ(imgSlice.mipLevel, 3u);
    EXPECT_EQ(imgSlice.arrayLayer, 6u);

    ImageSlice imgSlice2 = arraySlice.Slice(0);
    EXPECT_EQ(imgSlice2.arrayLayer, 4u);
}

TEST(ImageSlice, ContainedIn_ImageArraySlice_True) {
    ImageArraySlice arraySlice = { 3, 4, 10 };
    ImageSlice imgSlice = { 3, 7 };
    EXPECT_TRUE(imgSlice.ContainedIn(arraySlice));
}

TEST(ImageSlice, ContainedIn_ImageArraySlice_MipMismatch) {
    ImageArraySlice arraySlice = { 3, 4, 10 };
    ImageSlice imgSlice = { 4, 7 };
    EXPECT_FALSE(imgSlice.ContainedIn(arraySlice));
}

TEST(ImageSlice, ContainedIn_ImageArraySlice_LayerOutOfRange) {
    ImageArraySlice arraySlice = { 3, 4, 10 };
    ImageSlice imgSlice = { 3, 14 };
    EXPECT_FALSE(imgSlice.ContainedIn(arraySlice));
}

TEST(ImageSlice, ContainedIn_ImageArraySlice_LayerAtBoundary) {
    ImageArraySlice arraySlice = { 3, 4, 10 };
    ImageSlice imgSlice = { 3, 14 };
    EXPECT_FALSE(imgSlice.ContainedIn(arraySlice));
}

TEST(ImageSlice, ContainedIn_ImageMipArraySlice_True) {
    ImageMipArraySlice mipSlice = { 0, 5, 2, 10 };
    ImageSlice imgSlice = { 2, 5 };
    EXPECT_TRUE(imgSlice.ContainedIn(mipSlice));
}

TEST(ImageSlice, ContainedIn_ImageMipArraySlice_MipOutOfRange) {
    ImageMipArraySlice mipSlice = { 0, 5, 2, 10 };
    ImageSlice imgSlice = { 5, 5 };
    EXPECT_FALSE(imgSlice.ContainedIn(mipSlice));
}

TEST(ImageSlice, ContainedIn_ImageMipArraySlice_LayerOutOfRange) {
    ImageMipArraySlice mipSlice = { 0, 5, 2, 10 };
    ImageSlice imgSlice = { 2, 12 };
    EXPECT_FALSE(imgSlice.ContainedIn(mipSlice));
}

TEST(ImageSlice, ContainedIn_ImageMipArraySlice_AtBoundary) {
    ImageMipArraySlice mipSlice = { 0, 5, 2, 10 };
    ImageSlice imgSliceMip = { 4, 5 };
    ImageSlice imgSliceLayer = { 2, 12 };
    EXPECT_TRUE(imgSliceMip.ContainedIn(mipSlice));
    EXPECT_FALSE(imgSliceLayer.ContainedIn(mipSlice));
}
