#include "gtest/gtest.h"
#include <PyroRHI/Api/Types.hpp>

using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;

TEST(Rect2DCut, BasicExtent) {
    Extent2D extent = { 800, 600 };
    Rect2D rect = Rect2D::Cut(extent);
    EXPECT_EQ(rect.x, 0);
    EXPECT_EQ(rect.y, 0);
    EXPECT_EQ(rect.width, 800);
    EXPECT_EQ(rect.height, 600);
}

TEST(Rect2DCut, WithOffset) {
    Extent2D extent = { 400, 300 };
    Offset2D offset = { 100, 50 };
    Rect2D rect = Rect2D::Cut(extent, offset);
    EXPECT_EQ(rect.x, 100);
    EXPECT_EQ(rect.y, 50);
    EXPECT_EQ(rect.width, 400);
    EXPECT_EQ(rect.height, 300);
}

TEST(Rect2DCut, ZeroOffset) {
    Extent2D extent = { 1920, 1080 };
    Offset2D offset = { 0, 0 };
    Rect2D rect = Rect2D::Cut(extent, offset);
    EXPECT_EQ(rect.x, 0);
    EXPECT_EQ(rect.y, 0);
    EXPECT_EQ(rect.width, 1920);
    EXPECT_EQ(rect.height, 1080);
}

TEST(Box3DCut, BasicExtent) {
    Extent3D extent = { 100, 50, 20 };
    Box3D box = Box3D::Cut(extent);
    EXPECT_EQ(box.x, 0);
    EXPECT_EQ(box.y, 0);
    EXPECT_EQ(box.z, 0);
    EXPECT_EQ(box.width, 100);
    EXPECT_EQ(box.height, 50);
    EXPECT_EQ(box.depth, 20);
}

TEST(Box3DCut, WithOffset) {
    Extent3D extent = { 100, 50, 20 };
    Offset3D offset = { 10, 20, 5 };
    Box3D box = Box3D::Cut(extent, offset);
    EXPECT_EQ(box.x, 10);
    EXPECT_EQ(box.y, 20);
    EXPECT_EQ(box.z, 5);
    EXPECT_EQ(box.width, 100);
    EXPECT_EQ(box.height, 50);
    EXPECT_EQ(box.depth, 20);
}

TEST(Box3DCut, ZeroOffset) {
    Extent3D extent = { 256, 256, 1 };
    Offset3D offset = { 0, 0, 0 };
    Box3D box = Box3D::Cut(extent, offset);
    EXPECT_EQ(box.x, 0);
    EXPECT_EQ(box.y, 0);
    EXPECT_EQ(box.z, 0);
    EXPECT_EQ(box.width, 256);
    EXPECT_EQ(box.height, 256);
    EXPECT_EQ(box.depth, 1);
}
