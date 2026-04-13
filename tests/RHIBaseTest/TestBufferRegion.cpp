#include "gtest/gtest.h"
#include <PyroRHI/Api/Types.hpp>

using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;

TEST(BufferRegion, Contains_SelfContains) {
    BufferRegion region = { 0, 100 };
    EXPECT_TRUE(region.Contains(region));
}

TEST(BufferRegion, Contains_Subregion) {
    BufferRegion outer = { 0, 100 };
    BufferRegion inner = { 10, 50 };
    EXPECT_TRUE(outer.Contains(inner));
}

TEST(BufferRegion, Contains_ExactMatch) {
    BufferRegion a = { 50, 200 };
    BufferRegion b = { 50, 200 };
    EXPECT_TRUE(a.Contains(b));
}

TEST(BufferRegion, Contains_StartsBefore) {
    BufferRegion outer = { 0, 100 };
    BufferRegion inner = { 0, 50 };
    EXPECT_TRUE(outer.Contains(inner));
}

TEST(BufferRegion, Contains_EndsAtSamePoint) {
    BufferRegion outer = { 0, 100 };
    BufferRegion inner = { 50, 50 };
    EXPECT_TRUE(outer.Contains(inner));
}

TEST(BufferRegion, Contains_NotContainedExtendsBeyond) {
    BufferRegion outer = { 0, 100 };
    BufferRegion inner = { 50, 100 };
    EXPECT_FALSE(outer.Contains(inner));
}

TEST(BufferRegion, Contains_NotContainedStartsBefore) {
    BufferRegion outer = { 50, 100 };
    BufferRegion inner = { 0, 100 };
    EXPECT_FALSE(outer.Contains(inner));
}

TEST(BufferRegion, Contains_CompletelyDisjoint) {
    BufferRegion a = { 0, 100 };
    BufferRegion b = { 200, 50 };
    EXPECT_FALSE(a.Contains(b));
}

TEST(BufferRegion, Intersects_Overlapping) {
    BufferRegion a = { 0, 100 };
    BufferRegion b = { 50, 100 };
    EXPECT_TRUE(a.Intersects(b));
    EXPECT_TRUE(b.Intersects(a));
}

TEST(BufferRegion, Intersects_AdjacentNotOverlapping) {
    BufferRegion a = { 0, 100 };
    BufferRegion b = { 100, 100 };
    EXPECT_FALSE(a.Intersects(b));
    EXPECT_FALSE(b.Intersects(a));
}

TEST(BufferRegion, Intersects_CompletelyDisjoint) {
    BufferRegion a = { 0, 50 };
    BufferRegion b = { 100, 50 };
    EXPECT_FALSE(a.Intersects(b));
    EXPECT_FALSE(b.Intersects(a));
}

TEST(BufferRegion, Intersects_Subset) {
    BufferRegion outer = { 0, 100 };
    BufferRegion inner = { 25, 25 };
    EXPECT_TRUE(outer.Intersects(inner));
    EXPECT_TRUE(inner.Intersects(outer));
}

TEST(BufferRegion, Intersects_SelfIntersects) {
    BufferRegion region = { 0, 100 };
    EXPECT_TRUE(region.Intersects(region));
}

TEST(BufferRegion, Intersect_Overlapping) {
    BufferRegion a = { 0, 100 };
    BufferRegion b = { 50, 100 };
    BufferRegion result = a.Intersect(b);
    EXPECT_EQ(result.offset, 50u);
    EXPECT_EQ(result.size, 50u);
}

TEST(BufferRegion, Intersect_Subset) {
    BufferRegion outer = { 0, 200 };
    BufferRegion inner = { 50, 50 };
    BufferRegion result = outer.Intersect(inner);
    EXPECT_EQ(result.offset, 50u);
    EXPECT_EQ(result.size, 50u);
}

TEST(BufferRegion, Intersect_NoIntersection) {
    BufferRegion a = { 0, 50 };
    BufferRegion b = { 100, 50 };
    BufferRegion result = a.Intersect(b);
    EXPECT_EQ(result.offset, 0u);
    EXPECT_EQ(result.size, 0u);
}

TEST(BufferRegion, Intersect_ExactMatch) {
    BufferRegion a = { 100, 200 };
    BufferRegion result = a.Intersect(a);
    EXPECT_EQ(result.offset, 100u);
    EXPECT_EQ(result.size, 200u);
}

TEST(BufferRegion, Subtract_NoIntersection) {
    BufferRegion region = { 0, 100 };
    BufferRegion slice = { 200, 50 };
    auto [result, count] = region.Subtract(slice);
    ASSERT_EQ(count, 1u);
    EXPECT_EQ(result[0].offset, 0u);
    EXPECT_EQ(result[0].size, 100u);
}

TEST(BufferRegion, Subtract_CompleteOverlap) {
    BufferRegion region = { 50, 50 };
    BufferRegion slice = { 0, 200 };
    auto [result, count] = region.Subtract(slice);
    EXPECT_EQ(count, 0u);
}

TEST(BufferRegion, Subtract_SliceAtEnd) {
    BufferRegion region = { 0, 100 };
    BufferRegion slice = { 50, 50 };
    auto [result, count] = region.Subtract(slice);
    ASSERT_EQ(count, 1u);
    EXPECT_EQ(result[0].offset, 0u);
    EXPECT_EQ(result[0].size, 50u);
}

TEST(BufferRegion, Subtract_SliceAtStart) {
    BufferRegion region = { 0, 100 };
    BufferRegion slice = { 0, 50 };
    auto [result, count] = region.Subtract(slice);
    ASSERT_EQ(count, 1u);
    EXPECT_EQ(result[0].offset, 50u);
    EXPECT_EQ(result[0].size, 50u);
}

TEST(BufferRegion, Subtract_SliceInMiddle) {
    BufferRegion region = { 0, 100 };
    BufferRegion slice = { 25, 50 };
    auto [result, count] = region.Subtract(slice);
    ASSERT_EQ(count, 2u);
    EXPECT_EQ(result[0].offset, 0u);
    EXPECT_EQ(result[0].size, 25u);
    EXPECT_EQ(result[1].offset, 75u);
    EXPECT_EQ(result[1].size, 25u);
}

TEST(BufferRegion, Subtract_PartialOverlapAtEnd) {
    BufferRegion region = { 0, 100 };
    BufferRegion slice = { 75, 100 };
    auto [result, count] = region.Subtract(slice);
    ASSERT_EQ(count, 1u);
    EXPECT_EQ(result[0].offset, 0u);
    EXPECT_EQ(result[0].size, 75u);
}

TEST(BufferRegion, Subtract_PartialOverlapAtStart) {
    BufferRegion region = { 50, 100 };
    BufferRegion slice = { 0, 75 };
    auto [result, count] = region.Subtract(slice);
    ASSERT_EQ(count, 1u);
    EXPECT_EQ(result[0].offset, 75u);
    EXPECT_EQ(result[0].size, 75u);
}
