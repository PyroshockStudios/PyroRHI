#include "gtest/gtest.h"
#include <PyroRHI/Api/Types.hpp>

using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;

TEST(AccessOperators, BitwiseOr) {
    Access a = AccessConsts::VERTEX_SHADER_READ;
    Access b = AccessConsts::FRAGMENT_SHADER_WRITE;
    Access result = a | b;
    EXPECT_EQ(result.stages, PipelineStageFlagBits::VERTEX_SHADER | PipelineStageFlagBits::FRAGMENT_SHADER);
    EXPECT_EQ(result.type, AccessTypeFlagBits::READ | AccessTypeFlagBits::WRITE);
}

TEST(AccessOperators, BitwiseAnd) {
    Access a = { PipelineStageFlagBits::VERTEX_SHADER | PipelineStageFlagBits::FRAGMENT_SHADER, AccessTypeFlagBits::READ | AccessTypeFlagBits::WRITE };
    Access b = { PipelineStageFlagBits::FRAGMENT_SHADER, AccessTypeFlagBits::WRITE };
    Access result = a & b;
    EXPECT_EQ(result.stages, PipelineStageFlagBits::FRAGMENT_SHADER);
    EXPECT_EQ(result.type, AccessTypeFlagBits::WRITE);
}

TEST(AccessOperators, BitwiseXor) {
    Access a = { PipelineStageFlagBits::VERTEX_SHADER | PipelineStageFlagBits::FRAGMENT_SHADER, AccessTypeFlagBits::READ | AccessTypeFlagBits::WRITE };
    Access b = { PipelineStageFlagBits::FRAGMENT_SHADER, AccessTypeFlagBits::WRITE };
    Access result = a ^ b;
    EXPECT_EQ(result.stages, PipelineStageFlagBits::VERTEX_SHADER);
    EXPECT_EQ(result.type, AccessTypeFlagBits::READ);
}

TEST(AccessOperators, BitwiseNot) {
    Access a = { PipelineStageFlagBits::VERTEX_SHADER, AccessTypeFlagBits::READ };
    Access result = ~a;
    EXPECT_EQ(result.stages, ~PipelineStageFlagBits::VERTEX_SHADER);
    EXPECT_EQ(result.type, ~AccessTypeFlagBits::READ);
}

TEST(AccessOperators, OrAssignment) {
    Access a = AccessConsts::VERTEX_SHADER_READ;
    a |= AccessConsts::FRAGMENT_SHADER_WRITE;
    EXPECT_EQ(a.stages, PipelineStageFlagBits::VERTEX_SHADER | PipelineStageFlagBits::FRAGMENT_SHADER);
    EXPECT_EQ(a.type, AccessTypeFlagBits::READ | AccessTypeFlagBits::WRITE);
}

TEST(AccessOperators, AndAssignment) {
    Access a = { PipelineStageFlagBits::VERTEX_SHADER | PipelineStageFlagBits::FRAGMENT_SHADER, AccessTypeFlagBits::READ | AccessTypeFlagBits::WRITE };
    a &= { PipelineStageFlagBits::FRAGMENT_SHADER, AccessTypeFlagBits::WRITE };
    EXPECT_EQ(a.stages, PipelineStageFlagBits::FRAGMENT_SHADER);
    EXPECT_EQ(a.type, AccessTypeFlagBits::WRITE);
}

TEST(AccessOperators, XorAssignment) {
    Access a = { PipelineStageFlagBits::VERTEX_SHADER | PipelineStageFlagBits::FRAGMENT_SHADER, AccessTypeFlagBits::READ | AccessTypeFlagBits::WRITE };
    a ^= { PipelineStageFlagBits::FRAGMENT_SHADER, AccessTypeFlagBits::WRITE };
    EXPECT_EQ(a.stages, PipelineStageFlagBits::VERTEX_SHADER);
    EXPECT_EQ(a.type, AccessTypeFlagBits::READ);
}

TEST(AccessOperators, OperatorBool_TrueWhenBothSet) {
    Access a = AccessConsts::VERTEX_SHADER_READ;
    EXPECT_TRUE(static_cast<bool>(a));
}

TEST(AccessOperators, OperatorBool_FalseWhenStagesZero) {
    Access a = { PipelineStageFlagBits::NONE, AccessTypeFlagBits::READ };
    EXPECT_FALSE(static_cast<bool>(a));
}

TEST(AccessOperators, OperatorBool_FalseWhenTypeZero) {
    Access a = { PipelineStageFlagBits::VERTEX_SHADER, AccessTypeFlagBits::NONE };
    EXPECT_FALSE(static_cast<bool>(a));
}

TEST(AccessOperators, OperatorBool_FalseWhenBothZero) {
    Access a = AccessConsts::NONE;
    EXPECT_FALSE(static_cast<bool>(a));
}

TEST(AccessOperators, SpaceshipOperator_Equal) {
    Access a = AccessConsts::VERTEX_SHADER_READ;
    Access b = AccessConsts::VERTEX_SHADER_READ;
    EXPECT_EQ(a <=> b, std::strong_ordering::equal);
}

TEST(AccessOperators, SpaceshipOperator_Less) {
    Access a = AccessConsts::NONE;
    Access b = AccessConsts::VERTEX_SHADER_READ;
    EXPECT_EQ(a <=> b, std::strong_ordering::less);
}

TEST(AccessOperators, SpaceshipOperator_Greater) {
    Access a = AccessConsts::ALL_GRAPHICS_READ_WRITE;
    Access b = AccessConsts::VERTEX_SHADER_READ;
    EXPECT_EQ(a <=> b, std::strong_ordering::greater);
}

TEST(AccessOperators, CombinedReadWriteAccess) {
    Access rw = AccessConsts::VERTEX_SHADER_READ_WRITE;
    EXPECT_EQ(rw.stages, PipelineStageFlagBits::VERTEX_SHADER);
    EXPECT_EQ(rw.type, AccessTypeFlagBits::READ_WRITE);
    EXPECT_TRUE(static_cast<bool>(rw));
}
