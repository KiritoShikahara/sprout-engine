#include "sprout/reflect/PropertyValue.hpp"

#include <gtest/gtest.h>
#include <string>

namespace {

using sprout::reflect::PropertyValue;
using sprout::Vector2;
using sprout::Vector2Int;
using sprout::Color;
using sprout::Rect;
using sprout::Circle;
using sprout::Angle;

TEST(PropertyValueTests, DefaultConstructor_IsEmpty)
{
    PropertyValue value;
    EXPECT_TRUE(value.isEmpty());
}

TEST(PropertyValueTests, ConstructFromInt_StoresValue)
{
    PropertyValue value{std::int32_t{42}};
    EXPECT_FALSE(value.isEmpty());
    EXPECT_TRUE(value.is<std::int32_t>());
    EXPECT_EQ(value.get<std::int32_t>(), 42);
}

TEST(PropertyValueTests, ConstructFromFloat_StoresValue)
{
    PropertyValue value{3.14F};
    EXPECT_TRUE(value.is<float>());
    EXPECT_FLOAT_EQ(value.get<float>(), 3.14F);
}

TEST(PropertyValueTests, ConstructFromBool_StoresValue)
{
    PropertyValue value{true};
    EXPECT_TRUE(value.is<bool>());
    EXPECT_TRUE(value.get<bool>());
}

TEST(PropertyValueTests, ConstructFromString_StoresValue)
{
    PropertyValue value{std::string{"hello"}};
    EXPECT_TRUE(value.is<std::string>());
    EXPECT_EQ(value.get<std::string>(), "hello");
}

TEST(PropertyValueTests, ConstructFromVector2_StoresValue)
{
    PropertyValue value{Vector2{1.5F, -2.5F}};
    EXPECT_TRUE(value.is<Vector2>());
    const auto& vec = value.get<Vector2>();
    EXPECT_FLOAT_EQ(vec.x, 1.5F);
    EXPECT_FLOAT_EQ(vec.y, -2.5F);
}

TEST(PropertyValueTests, ConstructFromVector2Int_StoresValue)
{
    PropertyValue value{Vector2Int{3, -4}};
    EXPECT_TRUE(value.is<Vector2Int>());
    const auto& vec = value.get<Vector2Int>();
    EXPECT_EQ(vec.x, 3);
    EXPECT_EQ(vec.y, -4);
}

TEST(PropertyValueTests, ConstructFromColor_StoresValue)
{
    PropertyValue value{Color{255, 128, 64, 200}};
    EXPECT_TRUE(value.is<Color>());
    const auto& color = value.get<Color>();
    EXPECT_EQ(color.r, 255);
    EXPECT_EQ(color.g, 128);
    EXPECT_EQ(color.b, 64);
    EXPECT_EQ(color.a, 200);
}

TEST(PropertyValueTests, ConstructFromRect_StoresValue)
{
    PropertyValue value{Rect{{10.0F, 20.0F}, {100.0F, 50.0F}}};
    EXPECT_TRUE(value.is<Rect>());
    const auto& rect = value.get<Rect>();
    EXPECT_FLOAT_EQ(rect.position.x, 10.0F);
    EXPECT_FLOAT_EQ(rect.position.y, 20.0F);
    EXPECT_FLOAT_EQ(rect.size.x, 100.0F);
    EXPECT_FLOAT_EQ(rect.size.y, 50.0F);
}

TEST(PropertyValueTests, ConstructFromCircle_StoresValue)
{
    PropertyValue value{Circle{{5.0F, 5.0F}, 10.0F}};
    EXPECT_TRUE(value.is<Circle>());
    const auto& circle = value.get<Circle>();
    EXPECT_FLOAT_EQ(circle.center.x, 5.0F);
    EXPECT_FLOAT_EQ(circle.center.y, 5.0F);
    EXPECT_FLOAT_EQ(circle.radius, 10.0F);
}

TEST(PropertyValueTests, ConstructFromAngle_StoresValue)
{
    PropertyValue value{Angle::fromDegrees(90.0F)};
    EXPECT_TRUE(value.is<Angle>());
    const auto& angle = value.get<Angle>();
    EXPECT_FLOAT_EQ(angle.degrees(), 90.0F);
}

TEST(PropertyValueTests, ConstructFromGuid_StoresValue)
{
    PropertyValue value{PropertyValue::Guid{0x123456789ABCDEF0}};
    EXPECT_TRUE(value.is<PropertyValue::Guid>());
    EXPECT_EQ(value.get<PropertyValue::Guid>(), 0x123456789ABCDEF0);
}

TEST(PropertyValueTests, ConstructFromGameObjectRef_StoresValue)
{
    PropertyValue value{PropertyValue::GameObjectRef{42}};
    EXPECT_TRUE(value.is<PropertyValue::GameObjectRef>());
    EXPECT_EQ(value.get<PropertyValue::GameObjectRef>(), 42);
}

TEST(PropertyValueTests, ConstructFromEnumValue_StoresValue)
{
    PropertyValue value{PropertyValue::EnumValue{3}};
    EXPECT_TRUE(value.is<PropertyValue::EnumValue>());
    EXPECT_EQ(value.get<PropertyValue::EnumValue>(), 3);
}

TEST(PropertyValueTests, ConstructFromArray_StoresValue)
{
    PropertyValue::ArrayValue arr = {PropertyValue{1}, PropertyValue{2.5F}, PropertyValue{std::string{"test"}}};
    PropertyValue value{std::move(arr)};
    EXPECT_TRUE(value.is<PropertyValue::ArrayValue>());
    const auto& array = value.get<PropertyValue::ArrayValue>();
    EXPECT_EQ(array.size(), 3);
}

TEST(PropertyValueTests, CopyConstructor_CopiesValue)
{
    PropertyValue original{std::int32_t{123}};
    PropertyValue copy = original;
    EXPECT_TRUE(copy.is<std::int32_t>());
    EXPECT_EQ(copy.get<std::int32_t>(), 123);
}

TEST(PropertyValueTests, MoveConstructor_MovesValue)
{
    PropertyValue original{std::string{"move test"}};
    PropertyValue moved = std::move(original);
    EXPECT_TRUE(moved.is<std::string>());
    EXPECT_EQ(moved.get<std::string>(), "move test");
}

TEST(PropertyValueTests, Assignment_CopiesValue)
{
    PropertyValue a{std::int32_t{10}};
    PropertyValue b;
    b = a;
    EXPECT_TRUE(b.is<std::int32_t>());
    EXPECT_EQ(b.get<std::int32_t>(), 10);
}

TEST(PropertyValueTests, Equality_SameTypeAndValue_IsTrue)
{
    EXPECT_TRUE(PropertyValue{std::int32_t{42}} == PropertyValue{std::int32_t{42}});
    EXPECT_TRUE(PropertyValue{3.14F} == PropertyValue{3.14F});
    EXPECT_TRUE(PropertyValue{true} == PropertyValue{true});
    EXPECT_TRUE(PropertyValue{std::string{"test"}} == PropertyValue{std::string{"test"}});
    EXPECT_TRUE(PropertyValue{Vector2{1, 2}} == PropertyValue{Vector2{1, 2}});
    EXPECT_TRUE(PropertyValue{Color{255, 0, 0}} == PropertyValue{Color{255, 0, 0}});
}

TEST(PropertyValueTests, Equality_DifferentValue_IsFalse)
{
    EXPECT_FALSE(PropertyValue{std::int32_t{42}} == PropertyValue{std::int32_t{43}});
    EXPECT_FALSE(PropertyValue{3.14F} == PropertyValue{2.71F});
    EXPECT_FALSE(PropertyValue{true} == PropertyValue{false});
}

TEST(PropertyValueTests, Equality_DifferentType_IsFalse)
{
    EXPECT_FALSE(PropertyValue{std::int32_t{42}} == PropertyValue{42.0F});
    EXPECT_FALSE(PropertyValue{std::int32_t{1}} == PropertyValue{true});
    EXPECT_FALSE(PropertyValue{std::string{"42"}} == PropertyValue{std::int32_t{42}});
}

TEST(PropertyValueTests, Inequality_OppositeOfEquality)
{
    PropertyValue a{std::int32_t{42}};
    PropertyValue b{std::int32_t{42}};
    PropertyValue c{std::int32_t{43}};
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a != c);
}

TEST(PropertyValueTests, Is_TypeMatch_ReturnsTrue)
{
    PropertyValue value{std::int32_t{42}};
    EXPECT_TRUE(value.is<std::int32_t>());
    EXPECT_FALSE(value.is<float>());
}

TEST(PropertyValueTests, Get_ConstReference_ReturnsValue)
{
    const PropertyValue value{std::int32_t{99}};
    const std::int32_t& ref = value.get<std::int32_t>();
    EXPECT_EQ(ref, 99);
}

TEST(PropertyValueTests, Get_MutableReference_AllowsModification)
{
    PropertyValue value{std::int32_t{10}};
    std::int32_t& ref = value.get<std::int32_t>();
    ref = 20;
    EXPECT_EQ(value.get<std::int32_t>(), 20);
}

TEST(PropertyValueTests, TryGet_Success_ReturnsValue)
{
    PropertyValue value{std::int32_t{77}};
    auto result = value.tryGet<std::int32_t>();
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value(), 77);
}

TEST(PropertyValueTests, TryGet_TypeMismatch_ReturnsError)
{
    PropertyValue value{std::int32_t{77}};
    auto result = value.tryGet<float>();
    EXPECT_FALSE(result.isOk());
    EXPECT_EQ(result.error().code(), sprout::ErrorCode::InvalidArgument);
}

TEST(PropertyValueTests, ConvertTo_SameType_ReturnsSame)
{
    PropertyValue value{std::int32_t{42}};
    auto result = value.convertTo<std::int32_t>();
    ASSERT_TRUE(result.isOk());
    EXPECT_TRUE(result.value().is<std::int32_t>());
    EXPECT_EQ(result.value().get<std::int32_t>(), 42);
}

TEST(PropertyValueTests, ConvertTo_IntToFloat_Succeeds)
{
    PropertyValue value{std::int32_t{42}};
    auto result = value.convertTo<float>();
    ASSERT_TRUE(result.isOk());
    EXPECT_TRUE(result.value().is<float>());
    EXPECT_FLOAT_EQ(result.value().get<float>(), 42.0F);
}

TEST(PropertyValueTests, ConvertTo_FloatToInt_Truncates)
{
    PropertyValue value{3.9F};
    auto result = value.convertTo<std::int32_t>();
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value().get<std::int32_t>(), 3);
}

TEST(PropertyValueTests, ConvertTo_IntToBool_NonZeroIsTrue)
{
    PropertyValue value{std::int32_t{5}};
    auto result = value.convertTo<bool>();
    ASSERT_TRUE(result.isOk());
    EXPECT_TRUE(result.value().get<bool>());
}

TEST(PropertyValueTests, ConvertTo_IntToBool_ZeroIsFalse)
{
    PropertyValue value{std::int32_t{0}};
    auto result = value.convertTo<bool>();
    ASSERT_TRUE(result.isOk());
    EXPECT_FALSE(result.value().get<bool>());
}

TEST(PropertyValueTests, ConvertTo_FloatToBool_NonZeroIsTrue)
{
    PropertyValue value{1.5F};
    auto result = value.convertTo<bool>();
    ASSERT_TRUE(result.isOk());
    EXPECT_TRUE(result.value().get<bool>());
}

TEST(PropertyValueTests, ConvertTo_BoolToInt_TrueIsOne)
{
    PropertyValue value{true};
    auto result = value.convertTo<std::int32_t>();
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value().get<std::int32_t>(), 1);
}

TEST(PropertyValueTests, ConvertTo_BoolToInt_FalseIsZero)
{
    PropertyValue value{false};
    auto result = value.convertTo<std::int32_t>();
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value().get<std::int32_t>(), 0);
}

TEST(PropertyValueTests, ConvertTo_BoolToFloat_TrueIsOne)
{
    PropertyValue value{true};
    auto result = value.convertTo<float>();
    ASSERT_TRUE(result.isOk());
    EXPECT_FLOAT_EQ(result.value().get<float>(), 1.0F);
}

TEST(PropertyValueTests, ConvertTo_StringToInt_ParsesNumber)
{
    PropertyValue value{std::string{"123"}};
    auto result = value.convertTo<std::int32_t>();
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value().get<std::int32_t>(), 123);
}

TEST(PropertyValueTests, ConvertTo_StringToInt_InvalidString_ReturnsError)
{
    PropertyValue value{std::string{"abc"}};
    auto result = value.convertTo<std::int32_t>();
    EXPECT_FALSE(result.isOk());
    EXPECT_EQ(result.error().code(), sprout::ErrorCode::Parse);
}

TEST(PropertyValueTests, ConvertTo_StringToFloat_ParsesNumber)
{
    PropertyValue value{std::string{"3.14"}};
    auto result = value.convertTo<float>();
    ASSERT_TRUE(result.isOk());
    EXPECT_FLOAT_EQ(result.value().get<float>(), 3.14F);
}

TEST(PropertyValueTests, ConvertTo_StringToBool_ParsesTrue)
{
    PropertyValue value{std::string{"true"}};
    auto result = value.convertTo<bool>();
    ASSERT_TRUE(result.isOk());
    EXPECT_TRUE(result.value().get<bool>());
}

TEST(PropertyValueTests, ConvertTo_StringToBool_ParsesFalse)
{
    PropertyValue value{std::string{"false"}};
    auto result = value.convertTo<bool>();
    ASSERT_TRUE(result.isOk());
    EXPECT_FALSE(result.value().get<bool>());
}

TEST(PropertyValueTests, ConvertTo_StringToBool_ParsesOneZero)
{
    EXPECT_TRUE(PropertyValue{std::string{"1"}}.convertTo<bool>()->get<bool>());
    EXPECT_FALSE(PropertyValue{std::string{"0"}}.convertTo<bool>()->get<bool>());
}

TEST(PropertyValueTests, ConvertTo_StringToBool_YesNoOnOff)
{
    EXPECT_TRUE(PropertyValue{std::string{"yes"}}.convertTo<bool>()->get<bool>());
    EXPECT_TRUE(PropertyValue{std::string{"on"}}.convertTo<bool>()->get<bool>());
    EXPECT_FALSE(PropertyValue{std::string{"no"}}.convertTo<bool>()->get<bool>());
    EXPECT_FALSE(PropertyValue{std::string{"off"}}.convertTo<bool>()->get<bool>());
}

TEST(PropertyValueTests, ConvertTo_IntToString_FormatsNumber)
{
    PropertyValue value{std::int32_t{42}};
    auto result = value.convertTo<std::string>();
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value().get<std::string>(), "42");
}

TEST(PropertyValueTests, ConvertTo_FloatToString_FormatsNumber)
{
    PropertyValue value{3.14F};
    auto result = value.convertTo<std::string>();
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value().get<std::string>(), "3.14");
}

TEST(PropertyValueTests, ConvertTo_BoolToString_FormatsTrueFalse)
{
    EXPECT_EQ(PropertyValue{true}.convertTo<std::string>()->get<std::string>(), "true");
    EXPECT_EQ(PropertyValue{false}.convertTo<std::string>()->get<std::string>(), "false");
}

TEST(PropertyValueTests, ConvertTo_Vector2ToVector2Int_Truncates)
{
    PropertyValue value{Vector2{1.9F, -2.9F}};
    auto result = value.convertTo<Vector2Int>();
    ASSERT_TRUE(result.isOk());
    const auto& vec = result.value().get<Vector2Int>();
    EXPECT_EQ(vec.x, 1);
    EXPECT_EQ(vec.y, -2);
}

TEST(PropertyValueTests, ConvertTo_Vector2IntToVector2_Converts)
{
    PropertyValue value{Vector2Int{3, -4}};
    auto result = value.convertTo<Vector2>();
    ASSERT_TRUE(result.isOk());
    const auto& vec = result.value().get<Vector2>();
    EXPECT_FLOAT_EQ(vec.x, 3.0F);
    EXPECT_FLOAT_EQ(vec.y, -4.0F);
}

TEST(PropertyValueTests, ConvertTo_Vector2ToString_Formats)
{
    PropertyValue value{Vector2{1.5F, -2.5F}};
    auto result = value.convertTo<std::string>();
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value().get<std::string>(), "{1.5, -2.5}");
}

TEST(PropertyValueTests, ConvertTo_Vector2IntToString_Formats)
{
    PropertyValue value{Vector2Int{3, -4}};
    auto result = value.convertTo<std::string>();
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value().get<std::string>(), "{3, -4}");
}

TEST(PropertyValueTests, ConvertTo_StringToVector2_Parses)
{
    PropertyValue value{std::string{"{1.5, -2.5}"}};
    auto result = value.convertTo<Vector2>();
    ASSERT_TRUE(result.isOk());
    const auto& vec = result.value().get<Vector2>();
    EXPECT_FLOAT_EQ(vec.x, 1.5F);
    EXPECT_FLOAT_EQ(vec.y, -2.5F);
}

TEST(PropertyValueTests, ConvertTo_StringToVector2_InvalidFormat_ReturnsError)
{
    PropertyValue value{std::string{"invalid"}};
    auto result = value.convertTo<Vector2>();
    EXPECT_FALSE(result.isOk());
}

TEST(PropertyValueTests, ConvertTo_StringToVector2Int_Parses)
{
    PropertyValue value{std::string{"{3, -4}"}};
    auto result = value.convertTo<Vector2Int>();
    ASSERT_TRUE(result.isOk());
    const auto& vec = result.value().get<Vector2Int>();
    EXPECT_EQ(vec.x, 3);
    EXPECT_EQ(vec.y, -4);
}

TEST(PropertyValueTests, ConvertTo_ColorToString_FormatsHex)
{
    PropertyValue value{Color{255, 128, 64, 32}};
    auto result = value.convertTo<std::string>();
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value().get<std::string>(), "FF804020");
}

TEST(PropertyValueTests, ConvertTo_StringToColor_ParsesHex)
{
    PropertyValue value{std::string{"FF804020"}};
    auto result = value.convertTo<Color>();
    ASSERT_TRUE(result.isOk());
    const auto& color = result.value().get<Color>();
    EXPECT_EQ(color.r, 255);
    EXPECT_EQ(color.g, 128);
    EXPECT_EQ(color.b, 64);
    EXPECT_EQ(color.a, 32);
}

TEST(PropertyValueTests, ConvertTo_StringToColor_InvalidLength_ReturnsError)
{
    PropertyValue value{std::string{"FF8040"}};
    auto result = value.convertTo<Color>();
    EXPECT_FALSE(result.isOk());
}

TEST(PropertyValueTests, ConvertTo_RectToString_Formats)
{
    PropertyValue value{Rect{{10.0F, 20.0F}, {100.0F, 50.0F}}};
    auto result = value.convertTo<std::string>();
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value().get<std::string>(), "{{10, 20}, {100, 50}}");
}

TEST(PropertyValueTests, ConvertTo_StringToRect_Parses)
{
    PropertyValue value{std::string{"{{10, 20}, {100, 50}}"}};
    auto result = value.convertTo<Rect>();
    ASSERT_TRUE(result.isOk());
    const auto& rect = result.value().get<Rect>();
    EXPECT_FLOAT_EQ(rect.position.x, 10.0F);
    EXPECT_FLOAT_EQ(rect.position.y, 20.0F);
    EXPECT_FLOAT_EQ(rect.size.x, 100.0F);
    EXPECT_FLOAT_EQ(rect.size.y, 50.0F);
}

TEST(PropertyValueTests, ConvertTo_CircleToString_Formats)
{
    PropertyValue value{Circle{{5.0F, 5.0F}, 10.0F}};
    auto result = value.convertTo<std::string>();
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value().get<std::string>(), "{{5, 5}, 10}");
}

TEST(PropertyValueTests, ConvertTo_StringToCircle_Parses)
{
    PropertyValue value{std::string{"{{5, 5}, 10}"}};
    auto result = value.convertTo<Circle>();
    ASSERT_TRUE(result.isOk());
    const auto& circle = result.value().get<Circle>();
    EXPECT_FLOAT_EQ(circle.center.x, 5.0F);
    EXPECT_FLOAT_EQ(circle.center.y, 5.0F);
    EXPECT_FLOAT_EQ(circle.radius, 10.0F);
}

TEST(PropertyValueTests, ConvertTo_AngleToFloat_ReturnsRadians)
{
    PropertyValue value{Angle::fromDegrees(180.0F)};
    auto result = value.convertTo<float>();
    ASSERT_TRUE(result.isOk());
    EXPECT_FLOAT_EQ(result.value().get<float>(), 3.141592653589793F);
}

TEST(PropertyValueTests, ConvertTo_FloatToAngle_CreatesFromRadians)
{
    PropertyValue value{3.141592653589793F};
    auto result = value.convertTo<Angle>();
    ASSERT_TRUE(result.isOk());
    EXPECT_FLOAT_EQ(result.value().get<Angle>().degrees(), 180.0F);
}

TEST(PropertyValueTests, ConvertTo_AngleToString_FormatsDegrees)
{
    PropertyValue value{Angle::fromDegrees(90.0F)};
    auto result = value.convertTo<std::string>();
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value().get<std::string>(), "90");
}

TEST(PropertyValueTests, ConvertTo_StringToAngle_ParsesDegrees)
{
    PropertyValue value{std::string{"45"}};
    auto result = value.convertTo<Angle>();
    ASSERT_TRUE(result.isOk());
    EXPECT_FLOAT_EQ(result.value().get<Angle>().degrees(), 45.0F);
}

TEST(PropertyValueTests, ConvertTo_GuidToString_FormatsHex)
{
    PropertyValue value{PropertyValue::Guid{0x123456789ABCDEF0}};
    auto result = value.convertTo<std::string>();
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value().get<std::string>(), "123456789ABCDEF0");
}

TEST(PropertyValueTests, ConvertTo_StringToGuid_ParsesHex)
{
    PropertyValue value{std::string{"123456789ABCDEF0"}};
    auto result = value.convertTo<PropertyValue::Guid>();
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value().get<PropertyValue::Guid>(), 0x123456789ABCDEF0);
}

TEST(PropertyValueTests, ConvertTo_GameObjectRefToString_Formats)
{
    PropertyValue value{PropertyValue::GameObjectRef{42}};
    auto result = value.convertTo<std::string>();
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value().get<std::string>(), "42");
}

TEST(PropertyValueTests, ConvertTo_StringToGameObjectRef_Parses)
{
    PropertyValue value{std::string{"123"}};
    auto result = value.convertTo<PropertyValue::GameObjectRef>();
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value().get<PropertyValue::GameObjectRef>(), 123);
}

TEST(PropertyValueTests, ConvertTo_IntToGameObjectRef_Converts)
{
    PropertyValue value{std::int32_t{42}};
    auto result = value.convertTo<PropertyValue::GameObjectRef>();
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value().get<PropertyValue::GameObjectRef>(), 42);
}

TEST(PropertyValueTests, ConvertTo_EnumValueToInt_Converts)
{
    PropertyValue value{PropertyValue::EnumValue{5}};
    auto result = value.convertTo<std::int32_t>();
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value().get<std::int32_t>(), 5);
}

TEST(PropertyValueTests, ConvertTo_IntToEnumValue_Converts)
{
    PropertyValue value{std::int32_t{7}};
    auto result = value.convertTo<PropertyValue::EnumValue>();
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value().get<PropertyValue::EnumValue>(), 7);
}

TEST(PropertyValueTests, ConvertTo_StringToEnumValue_Parses)
{
    PropertyValue value{std::string{"3"}};
    auto result = value.convertTo<PropertyValue::EnumValue>();
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value().get<PropertyValue::EnumValue>(), 3);
}

TEST(PropertyValueTests, ConvertTo_ArrayToString_FormatsJsonLike)
{
    PropertyValue::ArrayValue arr = {PropertyValue{1}, PropertyValue{2}, PropertyValue{3}};
    PropertyValue value{std::move(arr)};
    auto result = value.convertTo<std::string>();
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value().get<std::string>(), "[1, 2, 3]");
}

TEST(PropertyValueTests, ConvertTo_StringToArray_Parses)
{
    PropertyValue value{std::string{"[1, 2, 3]"}};
    auto result = value.convertTo<PropertyValue::ArrayValue>();
    ASSERT_TRUE(result.isOk());
    const auto& arr = result.value().get<PropertyValue::ArrayValue>();
    EXPECT_EQ(arr.size(), 3);
    EXPECT_EQ(arr[0].get<std::int32_t>(), 1);
    EXPECT_EQ(arr[1].get<std::int32_t>(), 2);
    EXPECT_EQ(arr[2].get<std::int32_t>(), 3);
}

TEST(PropertyValueTests, ConvertTo_EmptyArray_ReturnsEmptyBrackets)
{
    PropertyValue value{PropertyValue::ArrayValue{}};
    auto result = value.convertTo<std::string>();
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value().get<std::string>(), "[]");
}

TEST(PropertyValueTests, ConvertTo_StringToEmptyArray_Parses)
{
    PropertyValue value{std::string{"[]"}};
    auto result = value.convertTo<PropertyValue::ArrayValue>();
    ASSERT_TRUE(result.isOk());
    EXPECT_TRUE(result.value().get<PropertyValue::ArrayValue>().empty());
}

TEST(PropertyValueTests, TryConvertTo_Success_ReturnsTrue)
{
    PropertyValue value{std::int32_t{42}};
    PropertyValue out;
    EXPECT_TRUE(value.tryConvertTo<float>(out));
    EXPECT_TRUE(out.is<float>());
    EXPECT_FLOAT_EQ(out.get<float>(), 42.0F);
}

TEST(PropertyValueTests, TryConvertTo_Failure_ReturnsFalseAndLeavesOutUnchanged)
{
    PropertyValue value{std::string{"invalid"}};
    PropertyValue out{std::int32_t{99}};
    EXPECT_FALSE(value.tryConvertTo<std::int32_t>(out));
    EXPECT_TRUE(out.is<std::int32_t>());
    EXPECT_EQ(out.get<std::int32_t>(), 99);
}

TEST(PropertyValueTests, PropertyValueCast_Helper_ReturnsPointer)
{
    PropertyValue value{std::int32_t{42}};
    const std::int32_t* ptr = sprout::reflect::propertyValueCast<std::int32_t>(&value);
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(*ptr, 42);
}

TEST(PropertyValueTests, PropertyValueCast_Helper_WrongType_ReturnsNull)
{
    PropertyValue value{std::int32_t{42}};
    const float* ptr = sprout::reflect::propertyValueCast<float>(&value);
    EXPECT_EQ(ptr, nullptr);
}

TEST(PropertyValueTests, PropertyValueCast_Helper_NullInput_ReturnsNull)
{
    const std::int32_t* ptr = sprout::reflect::propertyValueCast<std::int32_t>(nullptr);
    EXPECT_EQ(ptr, nullptr);
}

TEST(PropertyValueTests, VariantAccess_ReturnsInternalVariant)
{
    PropertyValue value{std::int32_t{42}};
    const auto& variant = value.variant();
    EXPECT_TRUE(std::holds_alternative<std::int32_t>(variant));
    EXPECT_EQ(std::get<std::int32_t>(variant), 42);
}

}  // namespace