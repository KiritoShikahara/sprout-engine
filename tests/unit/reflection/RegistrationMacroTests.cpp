#include "reflection/Registration.hpp"
#include "reflection/TypeInfo.hpp"

#include <gtest/gtest.h>
#include <string_view>

namespace {

using sprout::detail::TypeId;
using sprout::detail::TypeInfo;
using sprout::detail::FieldInfo;
using sprout::detail::TypeRegistry;
using sprout::detail::ModuleHandle;
using sprout::reflect::RangeAttribute;
using sprout::reflect::TooltipAttribute;
using sprout::reflect::HeaderAttribute;
using sprout::reflect::HiddenAttribute;
using sprout::reflect::ReadOnlyAttribute;
using sprout::reflect::MultilineAttribute;
using sprout::reflect::Attributes;

struct EmptyBehavior {
    SPROUT_BEHAVIOR(EmptyBehavior)
};

struct SimpleBehavior {
    SPROUT_PROPERTY(int, health, 100);
    SPROUT_PROPERTY(float, speed, 5.0f);
    SPROUT_BEHAVIOR(SimpleBehavior)
};

struct BehaviorWithAttributes {
    SPROUT_PROPERTY(RangeAttribute{0, 100}, TooltipAttribute{"プレイヤーの体力"}, int, health, 100);
    SPROUT_PROPERTY(RangeAttribute{0.0f, 20.0f}, HeaderAttribute{"移動"}, float, speed, 5.0f);
    SPROUT_PROPERTY(HiddenAttribute{}, int, internalId, 0);
    SPROUT_PROPERTY(ReadOnlyAttribute{}, MultilineAttribute{}, std::string, description, "");
    SPROUT_BEHAVIOR(BehaviorWithAttributes)
};

struct InheritanceBase {
    SPROUT_PROPERTY(int, baseValue, 42);
};
struct InheritanceDerived : InheritanceBase {
    SPROUT_PROPERTY(float, derivedValue, 1.5f);
    SPROUT_BEHAVIOR(InheritanceDerived)
};

TEST(RegistrationMacroTests, EmptyBehavior_RegistersType)
{
    TypeRegistry registry;
    ModuleHandle module = registry.createModule("TestModule");

    const TypeInfo* info = registry.registerType<EmptyBehavior>(module, "EmptyBehavior");

    ASSERT_NE(info, nullptr);
    EXPECT_EQ(info->name(), "EmptyBehavior");
    EXPECT_EQ(info->typeId(), TypeId<EmptyBehavior>());
    EXPECT_EQ(info->size(), sizeof(EmptyBehavior));
    EXPECT_EQ(info->fields().size(), 0);
}

TEST(RegistrationMacroTests, SimpleBehavior_RegistersFields)
{
    TypeRegistry registry;
    ModuleHandle module = registry.createModule("TestModule");

    const TypeInfo* info = registry.registerType<SimpleBehavior>(module, "SimpleBehavior");

    ASSERT_NE(info, nullptr);
    EXPECT_EQ(info->fields().size(), 2);

    const FieldInfo* healthField = info->findField("health");
    ASSERT_NE(healthField, nullptr);
    EXPECT_EQ(healthField->name(), "health");
    EXPECT_EQ(healthField->offset(), offsetof(SimpleBehavior, health));
    EXPECT_EQ(healthField->typeId(), TypeId<int>());

    const FieldInfo* speedField = info->findField("speed");
    ASSERT_NE(speedField, nullptr);
    EXPECT_EQ(speedField->name(), "speed");
    EXPECT_EQ(speedField->offset(), offsetof(SimpleBehavior, speed));
    EXPECT_EQ(speedField->typeId(), TypeId<float>());
}

TEST(RegistrationMacroTests, BehaviorWithAttributes_RegistersAttributes)
{
    TypeRegistry registry;
    ModuleHandle module = registry.createModule("TestModule");

    const TypeInfo* info = registry.registerType<BehaviorWithAttributes>(module, "BehaviorWithAttributes");

    ASSERT_NE(info, nullptr);
    EXPECT_EQ(info->fields().size(), 4);

    const FieldInfo* healthField = info->findField("health");
    ASSERT_NE(healthField, nullptr);
    const auto* healthAttrs = healthField->attributes();
    ASSERT_NE(healthAttrs, nullptr);
    EXPECT_TRUE(healthAttrs->has<RangeAttribute>());
    EXPECT_TRUE(healthAttrs->has<TooltipAttribute>());
    EXPECT_FALSE(healthAttrs->has<HeaderAttribute>());
    const auto* range = healthAttrs->get<RangeAttribute>();
    ASSERT_NE(range, nullptr);
    EXPECT_EQ(range->minInt(), 0);
    EXPECT_EQ(range->maxInt(), 100);
    const auto* tooltip = healthAttrs->get<TooltipAttribute>();
    ASSERT_NE(tooltip, nullptr);
    EXPECT_EQ(tooltip->text, "プレイヤーの体力");

    const FieldInfo* speedField = info->findField("speed");
    ASSERT_NE(speedField, nullptr);
    const auto* speedAttrs = speedField->attributes();
    ASSERT_NE(speedAttrs, nullptr);
    EXPECT_TRUE(speedAttrs->has<RangeAttribute>());
    EXPECT_TRUE(speedAttrs->has<HeaderAttribute>());
    const auto* range2 = speedAttrs->get<RangeAttribute>();
    ASSERT_NE(range2, nullptr);
    EXPECT_FLOAT_EQ(range2->minFloat(), 0.0f);
    EXPECT_FLOAT_EQ(range2->maxFloat(), 20.0f);
    const auto* header = speedAttrs->get<HeaderAttribute>();
    ASSERT_NE(header, nullptr);
    EXPECT_EQ(header->text, "移動");

    const FieldInfo* internalField = info->findField("internalId");
    ASSERT_NE(internalField, nullptr);
    const auto* internalAttrs = internalField->attributes();
    ASSERT_NE(internalAttrs, nullptr);
    EXPECT_TRUE(internalAttrs->has<HiddenAttribute>());

    const FieldInfo* descField = info->findField("description");
    ASSERT_NE(descField, nullptr);
    const auto* descAttrs = descField->attributes();
    ASSERT_NE(descAttrs, nullptr);
    EXPECT_TRUE(descAttrs->has<ReadOnlyAttribute>());
    EXPECT_TRUE(descAttrs->has<MultilineAttribute>());
}

TEST(RegistrationMacroTests, Inheritance_RegistersBaseAndDerivedFields)
{
    TypeRegistry registry;
    ModuleHandle module = registry.createModule("TestModule");

    const TypeInfo* info = registry.registerType<InheritanceDerived>(module, "InheritanceDerived");

    ASSERT_NE(info, nullptr);
    EXPECT_EQ(info->fields().size(), 2);

    const FieldInfo* baseField = info->findField("baseValue");
    ASSERT_NE(baseField, nullptr);
    EXPECT_EQ(baseField->name(), "baseValue");
    EXPECT_EQ(baseField->offset(), offsetof(InheritanceDerived, baseValue));
    EXPECT_EQ(baseField->typeId(), TypeId<int>());

    const FieldInfo* derivedField = info->findField("derivedValue");
    ASSERT_NE(derivedField, nullptr);
    EXPECT_EQ(derivedField->name(), "derivedValue");
    EXPECT_EQ(derivedField->offset(), offsetof(InheritanceDerived, derivedValue));
    EXPECT_EQ(derivedField->typeId(), TypeId<float>());
}

TEST(RegistrationMacroTests, FieldOffset_AllowsDirectAccess)
{
    TypeRegistry registry;
    ModuleHandle module = registry.createModule("TestModule");
    registry.registerType<SimpleBehavior>(module, "SimpleBehavior");

    const TypeInfo* info = registry.findType(TypeId<SimpleBehavior>());
    const FieldInfo* field = info->findField("health");

    SimpleBehavior obj;
    int* valuePtr = reinterpret_cast<int*>(reinterpret_cast<char*>(&obj) + field->offset());
    *valuePtr = 999;

    EXPECT_EQ(obj.health, 999);
}

TEST(RegistrationMacroTests, DuplicateRegistration_ReturnsSameTypeInfo)
{
    TypeRegistry registry;
    ModuleHandle module = registry.createModule("TestModule");

    const TypeInfo* info1 = registry.registerType<SimpleBehavior>(module, "SimpleBehavior");
    const TypeInfo* info2 = registry.registerType<SimpleBehavior>(module, "SimpleBehavior");

    EXPECT_EQ(info1, info2);
}

}  // namespace