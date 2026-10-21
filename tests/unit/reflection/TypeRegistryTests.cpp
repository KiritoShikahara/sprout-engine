#include "reflection/TypeInfo.hpp"

#include <gtest/gtest.h>
#include <string_view>

namespace {

using sprout::detail::TypeId;
using sprout::detail::TypeInfo;
using sprout::detail::FieldInfo;
using sprout::detail::TypeRegistry;
using sprout::detail::ModuleHandle;

struct EmptyType {};
struct SimpleType {
    int value = 42;
};
struct ComplexType {
    int a = 1;
    float b = 2.0f;
    double c = 3.0;
};

TEST(TypeIdTests, SameType_ReturnsSameId)
{
    EXPECT_EQ(TypeId<EmptyType>(), TypeId<EmptyType>());
    EXPECT_EQ(TypeId<SimpleType>(), TypeId<SimpleType>());
}

TEST(TypeIdTests, DifferentTypes_ReturnDifferentIds)
{
    EXPECT_NE(TypeId<EmptyType>(), TypeId<SimpleType>());
    EXPECT_NE(TypeId<SimpleType>(), TypeId<ComplexType>());
}

TEST(TypeIdTests, IdIsNonZero)
{
    EXPECT_NE(TypeId<EmptyType>(), 0);
    EXPECT_NE(TypeId<SimpleType>(), 0);
}

TEST(TypeRegistryTests, RegisterType_BasicRegistration)
{
    TypeRegistry registry;
    ModuleHandle module = registry.createModule("TestModule");

    const TypeInfo* info = registry.registerType<SimpleType>(module, "SimpleType");

    ASSERT_NE(info, nullptr);
    EXPECT_EQ(info->name(), "SimpleType");
    EXPECT_EQ(info->typeId(), TypeId<SimpleType>());
    EXPECT_EQ(info->size(), sizeof(SimpleType));
    EXPECT_EQ(info->module(), module);
}

TEST(TypeRegistryTests, RegisterType_DuplicateRegistration_ReturnsExisting)
{
    TypeRegistry registry;
    ModuleHandle module = registry.createModule("TestModule");

    const TypeInfo* info1 = registry.registerType<SimpleType>(module, "SimpleType");
    const TypeInfo* info2 = registry.registerType<SimpleType>(module, "SimpleType");

    EXPECT_EQ(info1, info2);
}

TEST(TypeRegistryTests, RegisterType_DifferentModules_CanRegisterSameType)
{
    TypeRegistry registry;
    ModuleHandle module1 = registry.createModule("Module1");
    ModuleHandle module2 = registry.createModule("Module2");

    const TypeInfo* info1 = registry.registerType<SimpleType>(module1, "SimpleType");
    const TypeInfo* info2 = registry.registerType<SimpleType>(module2, "SimpleType");

    EXPECT_NE(info1, info2);
    EXPECT_EQ(info1->module(), module1);
    EXPECT_EQ(info2->module(), module2);
}

TEST(TypeRegistryTests, FindType_ById_ReturnsRegisteredType)
{
    TypeRegistry registry;
    ModuleHandle module = registry.createModule("TestModule");
    registry.registerType<SimpleType>(module, "SimpleType");

    const TypeInfo* found = registry.findType(TypeId<SimpleType>());

    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->name(), "SimpleType");
}

TEST(TypeRegistryTests, FindType_ByName_ReturnsRegisteredType)
{
    TypeRegistry registry;
    ModuleHandle module = registry.createModule("TestModule");
    registry.registerType<SimpleType>(module, "SimpleType");

    const TypeInfo* found = registry.findType("SimpleType");

    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->typeId(), TypeId<SimpleType>());
}

TEST(TypeRegistryTests, FindType_Unregistered_ReturnsNull)
{
    TypeRegistry registry;

    EXPECT_EQ(registry.findType(TypeId<SimpleType>()), nullptr);
    EXPECT_EQ(registry.findType("NonExistent"), nullptr);
}

TEST(TypeRegistryTests, UnregisterType_ById_RemovesType)
{
    TypeRegistry registry;
    ModuleHandle module = registry.createModule("TestModule");
    registry.registerType<SimpleType>(module, "SimpleType");

    bool result = registry.unregisterType(TypeId<SimpleType>());

    EXPECT_TRUE(result);
    EXPECT_EQ(registry.findType(TypeId<SimpleType>()), nullptr);
}

TEST(TypeRegistryTests, UnregisterType_Unregistered_ReturnsFalse)
{
    TypeRegistry registry;

    EXPECT_FALSE(registry.unregisterType(TypeId<SimpleType>()));
}

TEST(TypeRegistryTests, UnregisterModule_RemovesAllTypesFromModule)
{
    TypeRegistry registry;
    ModuleHandle module = registry.createModule("TestModule");
    registry.registerType<SimpleType>(module, "SimpleType");
    registry.registerTypeWithFields<ComplexType>(
        module, "ComplexType",
        std::tuple<std::string, std::size_t, TypeId>{"a", offsetof(ComplexType, a), TypeId<int>()},
        std::tuple<std::string, std::size_t, TypeId>{"b", offsetof(ComplexType, b), TypeId<float>()},
        std::tuple<std::string, std::size_t, TypeId>{"c", offsetof(ComplexType, c), TypeId<double>()}
    );

    registry.unregisterModule(module);

    EXPECT_EQ(registry.findType(TypeId<SimpleType>()), nullptr);
    EXPECT_EQ(registry.findType(TypeId<ComplexType>()), nullptr);
}

TEST(TypeRegistryTests, TypeInfo_Fields_AreRegistered)
{
    TypeRegistry registry;
    ModuleHandle module = registry.createModule("TestModule");
    const TypeInfo* info = registry.registerTypeWithFields<ComplexType>(
        module, "ComplexType",
        std::tuple<std::string, std::size_t, TypeId>{"a", offsetof(ComplexType, a), TypeId<int>()},
        std::tuple<std::string, std::size_t, TypeId>{"b", offsetof(ComplexType, b), TypeId<float>()},
        std::tuple<std::string, std::size_t, TypeId>{"c", offsetof(ComplexType, c), TypeId<double>()}
    );

    const auto& fields = info->fields();
    EXPECT_EQ(fields.size(), 3);

    EXPECT_EQ(fields[0].name(), "a");
    EXPECT_EQ(fields[0].offset(), offsetof(ComplexType, a));
    EXPECT_EQ(fields[0].typeId(), TypeId<int>());

    EXPECT_EQ(fields[1].name(), "b");
    EXPECT_EQ(fields[1].offset(), offsetof(ComplexType, b));
    EXPECT_EQ(fields[1].typeId(), TypeId<float>());

    EXPECT_EQ(fields[2].name(), "c");
    EXPECT_EQ(fields[2].offset(), offsetof(ComplexType, c));
    EXPECT_EQ(fields[2].typeId(), TypeId<double>());
}

TEST(TypeRegistryTests, TypeInfo_FindField_ByName_ReturnsField)
{
    TypeRegistry registry;
    ModuleHandle module = registry.createModule("TestModule");
    const TypeInfo* info = registry.registerTypeWithFields<ComplexType>(
        module, "ComplexType",
        std::tuple<std::string, std::size_t, TypeId>{"a", offsetof(ComplexType, a), TypeId<int>()},
        std::tuple<std::string, std::size_t, TypeId>{"b", offsetof(ComplexType, b), TypeId<float>()},
        std::tuple<std::string, std::size_t, TypeId>{"c", offsetof(ComplexType, c), TypeId<double>()}
    );

    const FieldInfo* field = info->findField("b");

    ASSERT_NE(field, nullptr);
    EXPECT_EQ(field->name(), "b");
    EXPECT_EQ(field->typeId(), TypeId<float>());
}

TEST(TypeRegistryTests, TypeInfo_FindField_NonExistent_ReturnsNull)
{
    TypeRegistry registry;
    ModuleHandle module = registry.createModule("TestModule");
    const TypeInfo* info = registry.registerTypeWithFields<ComplexType>(
        module, "ComplexType",
        std::tuple<std::string, std::size_t, TypeId>{"a", offsetof(ComplexType, a), TypeId<int>()},
        std::tuple<std::string, std::size_t, TypeId>{"b", offsetof(ComplexType, b), TypeId<float>()},
        std::tuple<std::string, std::size_t, TypeId>{"c", offsetof(ComplexType, c), TypeId<double>()}
    );

    EXPECT_EQ(info->findField("nonexistent"), nullptr);
}

TEST(TypeRegistryTests, FieldInfo_Offset_AllowsDirectAccess)
{
    TypeRegistry registry;
    ModuleHandle module = registry.createModule("TestModule");
    registry.registerTypeWithFields<ComplexType>(
        module, "ComplexType",
        std::tuple<std::string, std::size_t, TypeId>{"a", offsetof(ComplexType, a), TypeId<int>()},
        std::tuple<std::string, std::size_t, TypeId>{"b", offsetof(ComplexType, b), TypeId<float>()},
        std::tuple<std::string, std::size_t, TypeId>{"c", offsetof(ComplexType, c), TypeId<double>()}
    );

    const TypeInfo* info = registry.findType(TypeId<ComplexType>());
    const FieldInfo* field = info->findField("a");

    ComplexType obj;
    int* valuePtr = reinterpret_cast<int*>(reinterpret_cast<char*>(&obj) + field->offset());
    *valuePtr = 123;

    EXPECT_EQ(obj.a, 123);
}

TEST(TypeRegistryTests, ModuleHandle_IsValid_AfterCreateModule)
{
    TypeRegistry registry;
    ModuleHandle module = registry.createModule("TestModule");

    EXPECT_TRUE(module.isValid());
}

TEST(TypeRegistryTests, ModuleHandle_DefaultConstructed_IsInvalid)
{
    ModuleHandle module;

    EXPECT_FALSE(module.isValid());
}

TEST(TypeRegistryTests, CreateModule_DuplicateName_ReturnsDifferentHandle)
{
    TypeRegistry registry;
    ModuleHandle module1 = registry.createModule("TestModule");
    ModuleHandle module2 = registry.createModule("TestModule");

    EXPECT_NE(module1, module2);
    EXPECT_TRUE(module1.isValid());
    EXPECT_TRUE(module2.isValid());
}

}  // namespace