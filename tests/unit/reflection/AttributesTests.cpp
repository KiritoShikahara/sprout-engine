#include "sprout/reflect/Attributes.hpp"

#include <gtest/gtest.h>
#include <string_view>

namespace {

using sprout::reflect::RangeAttribute;
using sprout::reflect::TooltipAttribute;
using sprout::reflect::HeaderAttribute;
using sprout::reflect::HiddenAttribute;
using sprout::reflect::ReadOnlyAttribute;
using sprout::reflect::MultilineAttribute;
using sprout::reflect::Attributes;

TEST(RangeAttributeTests, IntConstructor_StoresMinMax)
{
    RangeAttribute attr{0, 100};

    EXPECT_EQ(attr.minInt(), 0);
    EXPECT_EQ(attr.maxInt(), 100);
}

TEST(RangeAttributeTests, FloatConstructor_StoresScaledValues)
{
    RangeAttribute attr{0.0f, 20.0f};

    EXPECT_FLOAT_EQ(attr.minFloat(), 0.0f);
    EXPECT_FLOAT_EQ(attr.maxFloat(), 20.0f);
}

TEST(RangeAttributeTests, FloatConstructor_RoundTripsCorrectly)
{
    RangeAttribute attr{-5.5f, 10.25f};

    EXPECT_FLOAT_EQ(attr.minFloat(), -5.5f);
    EXPECT_FLOAT_EQ(attr.maxFloat(), 10.25f);
}

TEST(RangeAttributeTests, Equality_SameValues_ReturnsTrue)
{
    RangeAttribute attr1{0, 100};
    RangeAttribute attr2{0, 100};

    EXPECT_EQ(attr1, attr2);
}

TEST(RangeAttributeTests, Equality_DifferentValues_ReturnsFalse)
{
    RangeAttribute attr1{0, 100};
    RangeAttribute attr2{0, 50};

    EXPECT_NE(attr1, attr2);
}

TEST(TooltipAttributeTests, Constructor_StoresText)
{
    TooltipAttribute attr{"プレイヤーの体力。0 でゲームオーバー。"};

    EXPECT_EQ(attr.text, "プレイヤーの体力。0 でゲームオーバー。");
}

TEST(TooltipAttributeTests, EmptyString_IsAllowed)
{
    TooltipAttribute attr{""};

    EXPECT_EQ(attr.text, "");
}

TEST(TooltipAttributeTests, Equality_SameText_ReturnsTrue)
{
    TooltipAttribute attr1{"tooltip"};
    TooltipAttribute attr2{"tooltip"};

    EXPECT_EQ(attr1, attr2);
}

TEST(TooltipAttributeTests, Equality_DifferentText_ReturnsFalse)
{
    TooltipAttribute attr1{"tooltip1"};
    TooltipAttribute attr2{"tooltip2"};

    EXPECT_NE(attr1, attr2);
}

TEST(HeaderAttributeTests, Constructor_StoresText)
{
    HeaderAttribute attr{"ステータス"};

    EXPECT_EQ(attr.text, "ステータス");
}

TEST(HeaderAttributeTests, Equality_SameText_ReturnsTrue)
{
    HeaderAttribute attr1{"Header"};
    HeaderAttribute attr2{"Header"};

    EXPECT_EQ(attr1, attr2);
}

TEST(HeaderAttributeTests, Equality_DifferentText_ReturnsFalse)
{
    HeaderAttribute attr1{"Header1"};
    HeaderAttribute attr2{"Header2"};

    EXPECT_NE(attr1, attr2);
}

TEST(HiddenAttributeTests, DefaultConstructor_Works)
{
    HiddenAttribute attr{};

    (void)attr; // 空の属性なので比較のみ
    EXPECT_EQ(attr, HiddenAttribute{});
}

TEST(HiddenAttributeTests, Equality_AlwaysTrue)
{
    HiddenAttribute attr1{};
    HiddenAttribute attr2{};

    EXPECT_EQ(attr1, attr2);
}

TEST(ReadOnlyAttributeTests, DefaultConstructor_Works)
{
    ReadOnlyAttribute attr{};

    (void)attr;
    EXPECT_EQ(attr, ReadOnlyAttribute{});
}

TEST(ReadOnlyAttributeTests, Equality_AlwaysTrue)
{
    ReadOnlyAttribute attr1{};
    ReadOnlyAttribute attr2{};

    EXPECT_EQ(attr1, attr2);
}

TEST(MultilineAttributeTests, DefaultConstructor_Works)
{
    MultilineAttribute attr{};

    (void)attr;
    EXPECT_EQ(attr, MultilineAttribute{});
}

TEST(MultilineAttributeTests, Equality_AlwaysTrue)
{
    MultilineAttribute attr1{};
    MultilineAttribute attr2{};

    EXPECT_EQ(attr1, attr2);
}

TEST(AttributesTests, DefaultConstructor_Empty)
{
    Attributes attrs;

    EXPECT_TRUE(attrs.values.empty());
}

TEST(AttributesTests, SingleAttributeConstructor_StoresAttribute)
{
    Attributes attrs{RangeAttribute{0, 100}};

    ASSERT_EQ(attrs.values.size(), 1);
    EXPECT_TRUE(std::holds_alternative<RangeAttribute>(attrs.values[0]));
}

TEST(AttributesTests, MultipleAttributesConstructor_StoresAll)
{
    Attributes attrs{RangeAttribute{0, 100}, TooltipAttribute{"速度"}};

    ASSERT_EQ(attrs.values.size(), 2);
    EXPECT_TRUE(std::holds_alternative<RangeAttribute>(attrs.values[0]));
    EXPECT_TRUE(std::holds_alternative<TooltipAttribute>(attrs.values[1]));
}

TEST(AttributesTests, Add_AppendsAttribute)
{
    Attributes attrs;
    attrs.add(RangeAttribute{0, 100});
    attrs.add(TooltipAttribute{"速度"});

    ASSERT_EQ(attrs.values.size(), 2);
}

TEST(AttributesTests, Has_ReturnsTrueForPresentAttribute)
{
    Attributes attrs{RangeAttribute{0, 100}, TooltipAttribute{"速度"}};

    EXPECT_TRUE(attrs.has<RangeAttribute>());
    EXPECT_TRUE(attrs.has<TooltipAttribute>());
}

TEST(AttributesTests, Has_ReturnsFalseForAbsentAttribute)
{
    Attributes attrs{RangeAttribute{0, 100}};

    EXPECT_FALSE(attrs.has<TooltipAttribute>());
    EXPECT_FALSE(attrs.has<HeaderAttribute>());
}

TEST(AttributesTests, Get_ReturnsPointerToAttribute)
{
    Attributes attrs{RangeAttribute{0, 100}, TooltipAttribute{"速度"}};

    const RangeAttribute* range = attrs.get<RangeAttribute>();
    ASSERT_NE(range, nullptr);
    EXPECT_EQ(range->minInt(), 0);
    EXPECT_EQ(range->maxInt(), 100);
}

TEST(AttributesTests, Get_ReturnsNullForAbsentAttribute)
{
    Attributes attrs{RangeAttribute{0, 100}};

    const TooltipAttribute* tooltip = attrs.get<TooltipAttribute>();
    EXPECT_EQ(tooltip, nullptr);
}

TEST(AttributesTests, GetAll_ReturnsAllMatchingAttributes)
{
    Attributes attrs{TooltipAttribute{"first"}, TooltipAttribute{"second"}, RangeAttribute{0, 100}};

    auto tooltips = attrs.getAll<TooltipAttribute>();
    ASSERT_EQ(tooltips.size(), 2);
    EXPECT_EQ(tooltips[0]->text, "first");
    EXPECT_EQ(tooltips[1]->text, "second");
}

TEST(AttributesTests, GetAll_ReturnsEmptyForAbsentAttribute)
{
    Attributes attrs{RangeAttribute{0, 100}};

    auto tooltips = attrs.getAll<TooltipAttribute>();
    EXPECT_TRUE(tooltips.empty());
}

TEST(AttributesTests, Equality_SameAttributes_ReturnsTrue)
{
    Attributes attrs1{RangeAttribute{0, 100}, TooltipAttribute{"速度"}};
    Attributes attrs2{RangeAttribute{0, 100}, TooltipAttribute{"速度"}};

    EXPECT_EQ(attrs1, attrs2);
}

TEST(AttributesTests, Equality_DifferentOrder_ReturnsFalse)
{
    Attributes attrs1{RangeAttribute{0, 100}, TooltipAttribute{"速度"}};
    Attributes attrs2{TooltipAttribute{"速度"}, RangeAttribute{0, 100}};

    EXPECT_NE(attrs1, attrs2);
}

TEST(AttributesTests, Equality_DifferentValues_ReturnsFalse)
{
    Attributes attrs1{RangeAttribute{0, 100}};
    Attributes attrs2{RangeAttribute{0, 50}};

    EXPECT_NE(attrs1, attrs2);
}

TEST(AttributesTests, ComplexCombination_Works)
{
    Attributes attrs{
        RangeAttribute{0.0f, 100.0f},
        TooltipAttribute{"体力（0-100）"},
        HeaderAttribute{"ステータス"},
        ReadOnlyAttribute{}
    };

    ASSERT_EQ(attrs.values.size(), 4);
    EXPECT_TRUE(attrs.has<RangeAttribute>());
    EXPECT_TRUE(attrs.has<TooltipAttribute>());
    EXPECT_TRUE(attrs.has<HeaderAttribute>());
    EXPECT_TRUE(attrs.has<ReadOnlyAttribute>());
    EXPECT_FALSE(attrs.has<HiddenAttribute>());
    EXPECT_FALSE(attrs.has<MultilineAttribute>());

    const auto* range = attrs.get<RangeAttribute>();
    ASSERT_NE(range, nullptr);
    EXPECT_FLOAT_EQ(range->minFloat(), 0.0f);
    EXPECT_FLOAT_EQ(range->maxFloat(), 100.0f);
}

}  // namespace