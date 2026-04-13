#include "gtest/gtest.h"
#include <PyroRHI/Api/Types.hpp>

using namespace PyroshockStudios;
using namespace PyroshockStudios::RHI;
using namespace PyroshockStudios::Types;

TEST(LabelColorToU32, Red) {
    LabelColor c = LabelColor::RED;
    u32 val = c.ToU32();
    EXPECT_EQ((val >> 24) & 0xFF, 255);
    EXPECT_EQ((val >> 16) & 0xFF, 0);
    EXPECT_EQ((val >> 8) & 0xFF, 0);
    EXPECT_EQ(val & 0xFF, 255);
}

TEST(LabelColorToU32, Green) {
    LabelColor c = LabelColor::GREEN;
    u32 val = c.ToU32();
    EXPECT_EQ((val >> 24) & 0xFF, 0);
    EXPECT_EQ((val >> 16) & 0xFF, 255);
    EXPECT_EQ((val >> 8) & 0xFF, 0);
    EXPECT_EQ(val & 0xFF, 255);
}

TEST(LabelColorToU32, Blue) {
    LabelColor c = LabelColor::BLUE;
    u32 val = c.ToU32();
    EXPECT_EQ((val >> 24) & 0xFF, 0);
    EXPECT_EQ((val >> 16) & 0xFF, 0);
    EXPECT_EQ((val >> 8) & 0xFF, 255);
    EXPECT_EQ(val & 0xFF, 255);
}

TEST(LabelColorToU32, White) {
    LabelColor c = LabelColor::WHITE;
    u32 val = c.ToU32();
    EXPECT_EQ((val >> 24) & 0xFF, 255);
    EXPECT_EQ((val >> 16) & 0xFF, 255);
    EXPECT_EQ((val >> 8) & 0xFF, 255);
    EXPECT_EQ(val & 0xFF, 255);
}

TEST(LabelColorToU32, Black) {
    LabelColor c = LabelColor::BLACK;
    u32 val = c.ToU32();
    EXPECT_EQ((val >> 24) & 0xFF, 0);
    EXPECT_EQ((val >> 16) & 0xFF, 0);
    EXPECT_EQ((val >> 8) & 0xFF, 0);
    EXPECT_EQ(val & 0xFF, 255);
}

TEST(LabelColorToU32, Cyan) {
    LabelColor c = LabelColor::CYAN;
    u32 val = c.ToU32();
    EXPECT_EQ((val >> 24) & 0xFF, 0);
    EXPECT_EQ((val >> 16) & 0xFF, 255);
    EXPECT_EQ((val >> 8) & 0xFF, 255);
    EXPECT_EQ(val & 0xFF, 255);
}

TEST(LabelColorToU32, Magenta) {
    LabelColor c = LabelColor::MAGENTA;
    u32 val = c.ToU32();
    EXPECT_EQ((val >> 24) & 0xFF, 255);
    EXPECT_EQ((val >> 16) & 0xFF, 0);
    EXPECT_EQ((val >> 8) & 0xFF, 255);
    EXPECT_EQ(val & 0xFF, 255);
}

TEST(LabelColorToU32, Yellow) {
    LabelColor c = LabelColor::YELLOW;
    u32 val = c.ToU32();
    EXPECT_EQ((val >> 24) & 0xFF, 255);
    EXPECT_EQ((val >> 16) & 0xFF, 255);
    EXPECT_EQ((val >> 8) & 0xFF, 0);
    EXPECT_EQ(val & 0xFF, 255);
}

TEST(LabelColorToU32, Orange) {
    LabelColor c = LabelColor::ORANGE;
    u32 val = c.ToU32();
    EXPECT_EQ((val >> 24) & 0xFF, 255);
    EXPECT_EQ((val >> 8) & 0xFF, 0);
    EXPECT_EQ(val & 0xFF, 255);
}

TEST(LabelColorToU32, Teal) {
    LabelColor c = LabelColor::TEAL;
    u32 val = c.ToU32();
    EXPECT_EQ(val & 0xFF, 255);
}

TEST(LabelColorToU32, Violet) {
    LabelColor c = LabelColor::VIOLET;
    u32 val = c.ToU32();
    EXPECT_EQ(val & 0xFF, 255);
}

TEST(LabelColorToU32, Pink) {
    LabelColor c = LabelColor::PINK;
    u32 val = c.ToU32();
    EXPECT_EQ(val & 0xFF, 255);
}

TEST(LabelColorToU32, Turquoise) {
    LabelColor c = LabelColor::TURQUOISE;
    u32 val = c.ToU32();
    EXPECT_EQ(val & 0xFF, 255);
}

TEST(LabelColorToU32, CustomColor) {
    LabelColor c = { 0.0f, 0.0f, 0.0f, 0.0f };
    EXPECT_EQ(c.ToU32(), 0u);

    LabelColor c2 = { 1.0f, 1.0f, 1.0f, 1.0f };
    EXPECT_EQ(c2.ToU32(), 0xFFFFFFFF);
}

TEST(LabelColorToU32, FadedColors) {
    EXPECT_EQ(LabelColor::FADED_RED.ToU32() & 0xFF, 255);
    EXPECT_EQ(LabelColor::FADED_GREEN.ToU32() & 0xFF, 255);
    EXPECT_EQ(LabelColor::FADED_BLUE.ToU32() & 0xFF, 255);
    EXPECT_EQ(LabelColor::FADED_VIOLET.ToU32() & 0xFF, 255);
    EXPECT_EQ(LabelColor::FADED_MAGENTA.ToU32() & 0xFF, 255);
    EXPECT_EQ(LabelColor::FADED_ORANGE.ToU32() & 0xFF, 255);
    EXPECT_EQ(LabelColor::FADED_YELLOW.ToU32() & 0xFF, 255);
}

TEST(LabelColorComparison, Equal) {
    EXPECT_EQ(LabelColor::RED <=> LabelColor::RED, std::strong_ordering::equal);
}

TEST(LabelColorComparison, NotEqual) {
    EXPECT_NE(LabelColor::RED <=> LabelColor::BLUE, std::strong_ordering::equal);
}
