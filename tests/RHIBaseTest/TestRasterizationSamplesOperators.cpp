#include "gtest/gtest.h"
#include <PyroRHI/Api/Types.hpp>

using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;

TEST(RasterizationSamplesOperators, BitwiseOr) {
    RasterizationSamples result = RasterizationSamples::e1 | RasterizationSamples::e2;
    EXPECT_EQ(static_cast<u32>(result), 3u);
}

TEST(RasterizationSamplesOperators, BitwiseAnd) {
    RasterizationSamples combined = RasterizationSamples::e4 | RasterizationSamples::e8;
    RasterizationSamples result = combined & RasterizationSamples::e4;
    EXPECT_EQ(static_cast<u32>(result), 4u);
}

TEST(RasterizationSamplesOperators, BitwiseXor) {
    RasterizationSamples a = RasterizationSamples::e4 | RasterizationSamples::e8;
    RasterizationSamples result = a ^ RasterizationSamples::e4;
    EXPECT_EQ(static_cast<u32>(result), 8u);
}

TEST(RasterizationSamplesOperators, OrAssignment) {
    RasterizationSamples val = RasterizationSamples::e1;
    val |= RasterizationSamples::e4;
    EXPECT_EQ(static_cast<u32>(val), 5u);
}

TEST(RasterizationSamplesOperators, AndAssignment) {
    RasterizationSamples val = RasterizationSamples::e4 | RasterizationSamples::e8;
    val &= RasterizationSamples::e4;
    EXPECT_EQ(static_cast<u32>(val), 4u);
}

TEST(RasterizationSamplesOperators, XorAssignment) {
    RasterizationSamples val = RasterizationSamples::e4 | RasterizationSamples::e8;
    val ^= RasterizationSamples::e4;
    EXPECT_EQ(static_cast<u32>(val), 8u);
}

TEST(RasterizationSamplesOperators, SelfOrSelf) {
    RasterizationSamples val = RasterizationSamples::e4;
    EXPECT_EQ(static_cast<u32>(val | val), 4u);
}

TEST(RasterizationSamplesOperators, SelfAndSelf) {
    RasterizationSamples val = RasterizationSamples::e4;
    EXPECT_EQ(static_cast<u32>(val & val), 4u);
}

TEST(RasterizationSamplesOperators, SelfXorSelf) {
    RasterizationSamples val = RasterizationSamples::e4;
    EXPECT_EQ(static_cast<u32>(val ^ val), 0u);
}

TEST(RasterizationSamplesOperators, DisjointAnd) {
    RasterizationSamples a = RasterizationSamples::e1;
    RasterizationSamples b = RasterizationSamples::e4;
    EXPECT_EQ(static_cast<u32>(a & b), 0u);
}
