#include "gtest/gtest.h"
#include <PyroRHI/Api/Types.hpp>
#include <PyroRHI/Api/ICommandBuffer.hpp>

using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;

TEST(DrawArgumentBuffer, SizeAndLayout) {
    EXPECT_EQ(sizeof(DrawArgumentBuffer), 16u);
    DrawArgumentBuffer buf = { 100, 1, 0, 0 };
    EXPECT_EQ(buf.vertexCount, 100u);
    EXPECT_EQ(buf.instanceCount, 1u);
    EXPECT_EQ(buf.firstVertex, 0u);
    EXPECT_EQ(buf.firstInstance, 0u);
}

TEST(DrawArgumentBuffer, OffsetsMatchVulkanSpec) {
    EXPECT_EQ(offsetof(DrawArgumentBuffer, vertexCount), 0u);
    EXPECT_EQ(offsetof(DrawArgumentBuffer, instanceCount), 4u);
    EXPECT_EQ(offsetof(DrawArgumentBuffer, firstVertex), 8u);
    EXPECT_EQ(offsetof(DrawArgumentBuffer, firstInstance), 12u);
}

TEST(DrawIndexedArgumentBuffer, SizeAndLayout) {
    EXPECT_EQ(sizeof(DrawIndexedArgumentBuffer), 20u);
    DrawIndexedArgumentBuffer buf = { 1000, 1, 0, 0, 0 };
    EXPECT_EQ(buf.indexCount, 1000u);
    EXPECT_EQ(buf.instanceCount, 1u);
    EXPECT_EQ(buf.firstIndex, 0u);
    EXPECT_EQ(buf.vertexOffset, 0);
    EXPECT_EQ(buf.firstInstance, 0u);
}

TEST(DrawIndexedArgumentBuffer, OffsetsMatchVulkanSpec) {
    EXPECT_EQ(offsetof(DrawIndexedArgumentBuffer, indexCount), 0u);
    EXPECT_EQ(offsetof(DrawIndexedArgumentBuffer, instanceCount), 4u);
    EXPECT_EQ(offsetof(DrawIndexedArgumentBuffer, firstIndex), 8u);
    EXPECT_EQ(offsetof(DrawIndexedArgumentBuffer, vertexOffset), 12u);
    EXPECT_EQ(offsetof(DrawIndexedArgumentBuffer, firstInstance), 16u);
}

TEST(DispatchArgumentBuffer, SizeAndLayout) {
    EXPECT_EQ(sizeof(DispatchArgumentBuffer), 12u);
    DispatchArgumentBuffer buf = { 16, 8, 1 };
    EXPECT_EQ(buf.x, 16u);
    EXPECT_EQ(buf.y, 8u);
    EXPECT_EQ(buf.z, 1u);
}

TEST(DispatchArgumentBuffer, Offsets) {
    EXPECT_EQ(offsetof(DispatchArgumentBuffer, x), 0u);
    EXPECT_EQ(offsetof(DispatchArgumentBuffer, y), 4u);
    EXPECT_EQ(offsetof(DispatchArgumentBuffer, z), 8u);
}

TEST(DrawInfo, DefaultValues) {
    DrawInfo info = {};
    EXPECT_EQ(info.vertexCount, 0u);
    EXPECT_EQ(info.instanceCount, 1u);
    EXPECT_EQ(info.firstVertex, 0u);
    EXPECT_EQ(info.firstInstance, 0u);
}

TEST(DrawIndexedInfo, DefaultValues) {
    DrawIndexedInfo info = {};
    EXPECT_EQ(info.indexCount, 0u);
    EXPECT_EQ(info.instanceCount, 1u);
    EXPECT_EQ(info.firstIndex, 0u);
    EXPECT_EQ(info.vertexOffset, 0);
    EXPECT_EQ(info.firstInstance, 0u);
}

TEST(DispatchInfo, DefaultValues) {
    DispatchInfo info = {};
    EXPECT_EQ(info.x, 1u);
    EXPECT_EQ(info.y, 1u);
    EXPECT_EQ(info.z, 1u);
}

TEST(DrawIndirectInfo, DefaultStride) {
    DrawIndirectInfo info = {};
    EXPECT_EQ(info.drawCommandStride, sizeof(DrawArgumentBuffer));
}

TEST(DrawIndexedIndirectInfo, DefaultStride) {
    DrawIndexedIndirectInfo info = {};
    EXPECT_EQ(info.drawCommandStride, sizeof(DrawIndexedArgumentBuffer));
}
