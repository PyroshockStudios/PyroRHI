#include "gtest/gtest.h"
#include <PyroRHI/Api/Types.hpp>
#include <cmath>

using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;

TEST(Transform, IdentityMatrix) {
    const Transform& id = Transform::IDENTITY;
    EXPECT_FLOAT_EQ(id.matrix[0][0], 1.0f);
    EXPECT_FLOAT_EQ(id.matrix[0][1], 0.0f);
    EXPECT_FLOAT_EQ(id.matrix[0][2], 0.0f);
    EXPECT_FLOAT_EQ(id.matrix[0][3], 0.0f);
    EXPECT_FLOAT_EQ(id.matrix[1][0], 0.0f);
    EXPECT_FLOAT_EQ(id.matrix[1][1], 1.0f);
    EXPECT_FLOAT_EQ(id.matrix[1][2], 0.0f);
    EXPECT_FLOAT_EQ(id.matrix[1][3], 0.0f);
    EXPECT_FLOAT_EQ(id.matrix[2][0], 0.0f);
    EXPECT_FLOAT_EQ(id.matrix[2][1], 0.0f);
    EXPECT_FLOAT_EQ(id.matrix[2][2], 1.0f);
    EXPECT_FLOAT_EQ(id.matrix[2][3], 0.0f);
}

TEST(Transform, ZeroMatrix) {
    const Transform& zero = Transform::ZERO;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_FLOAT_EQ(zero.matrix[i][j], 0.0f);
        }
    }
}

TEST(Transform, ConstructorValues) {
    Transform t = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    EXPECT_FLOAT_EQ(t.matrix[0][0], 1.0f);
    EXPECT_FLOAT_EQ(t.matrix[0][1], 2.0f);
    EXPECT_FLOAT_EQ(t.matrix[0][2], 3.0f);
    EXPECT_FLOAT_EQ(t.matrix[0][3], 4.0f);
    EXPECT_FLOAT_EQ(t.matrix[1][0], 5.0f);
    EXPECT_FLOAT_EQ(t.matrix[1][1], 6.0f);
    EXPECT_FLOAT_EQ(t.matrix[1][2], 7.0f);
    EXPECT_FLOAT_EQ(t.matrix[1][3], 8.0f);
    EXPECT_FLOAT_EQ(t.matrix[2][0], 9.0f);
    EXPECT_FLOAT_EQ(t.matrix[2][1], 10.0f);
    EXPECT_FLOAT_EQ(t.matrix[2][2], 11.0f);
    EXPECT_FLOAT_EQ(t.matrix[2][3], 12.0f);
}

TEST(Transform, DefaultConstructorIsZero) {
    Transform t;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_FLOAT_EQ(t.matrix[i][j], 0.0f);
        }
    }
}
