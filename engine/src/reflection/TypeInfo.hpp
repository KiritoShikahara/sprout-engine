#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "sprout/reflect/Attributes.hpp"

namespace sprout::detail {

using TypeId = std::uint64_t;

template <typename T>
constexpr TypeId TypeIdOf() noexcept
{
    return reinterpret_cast<std::uintptr_t>(&typeid(T));
}

template <typename T>
constexpr TypeId TypeId() noexcept
{
    return TypeIdOf<T>();
}

class ModuleHandle {
public:
    constexpr ModuleHandle() noexcept = default;
    constexpr explicit ModuleHandle(std::uint32_t id) noexcept : m_id{id} {}

    [[nodiscard]] constexpr bool isValid() const noexcept { return m_id != 0; }
    [[nodiscard]] constexpr std::uint32_t id() const noexcept { return m_id; }

    constexpr bool operator==(const ModuleHandle& other) const noexcept = default;

private:
    std::uint32_t m_id{0};
};

class FieldInfo {
public:
    FieldInfo() noexcept = default;
    FieldInfo(std::string name, std::size_t offset, TypeId typeId) noexcept
        : m_name{std::move(name)}, m_offset{offset}, m_typeId{typeId}
    {
    }
    FieldInfo(std::string name, std::size_t offset, TypeId typeId, sprout::reflect::Attributes attrs) noexcept
        : m_name{std::move(name)}, m_offset{offset}, m_typeId{typeId}, m_attributes{std::make_shared<sprout::reflect::Attributes>(std::move(attrs))}
    {
    }

    [[nodiscard]] std::string_view name() const noexcept { return m_name; }
    [[nodiscard]] std::size_t offset() const noexcept { return m_offset; }
    [[nodiscard]] TypeId typeId() const noexcept { return m_typeId; }
    [[nodiscard]] const sprout::reflect::Attributes* attributes() const noexcept
    {
        return m_attributes ? m_attributes.get() : nullptr;
    }

private:
    std::string m_name;
    std::size_t m_offset{0};
    TypeId m_typeId{0};
    std::shared_ptr<sprout::reflect::Attributes> m_attributes;
};

class TypeInfo {
public:
    TypeInfo() noexcept = default;
    TypeInfo(TypeId typeId, std::string name, std::size_t size, ModuleHandle module) noexcept
        : m_typeId{typeId}, m_name{std::move(name)}, m_size{size}, m_module{module}
    {
    }

    [[nodiscard]] TypeId typeId() const noexcept { return m_typeId; }
    [[nodiscard]] std::string_view name() const noexcept { return m_name; }
    [[nodiscard]] std::size_t size() const noexcept { return m_size; }
    [[nodiscard]] ModuleHandle module() const noexcept { return m_module; }

    [[nodiscard]] const std::vector<FieldInfo>& fields() const noexcept { return m_fields; }
    [[nodiscard]] const FieldInfo* findField(std::string_view name) const noexcept
    {
        for (const auto& field : m_fields) {
            if (field.name() == name) {
                return &field;
            }
        }
        return nullptr;
    }

    void addField(std::string name, std::size_t offset, TypeId typeId)
    {
        m_fields.emplace_back(std::move(name), offset, typeId);
    }

    void addField(std::string name, std::size_t offset, TypeId typeId, sprout::reflect::Attributes attrs)
    {
        m_fields.emplace_back(std::move(name), offset, typeId, std::move(attrs));
    }

private:
    TypeId m_typeId{0};
    std::string m_name;
    std::size_t m_size{0};
    ModuleHandle m_module;
    std::vector<FieldInfo> m_fields;
};

class TypeRegistry {
public:
    TypeRegistry() = default;
    ~TypeRegistry() = default;

    TypeRegistry(const TypeRegistry&) = delete;
    TypeRegistry& operator=(const TypeRegistry&) = delete;
    TypeRegistry(TypeRegistry&&) = default;
    TypeRegistry& operator=(TypeRegistry&&) = default;

    ModuleHandle createModule(std::string_view name)
    {
        std::uint32_t id = ++m_nextModuleId;
        m_moduleNames.emplace(id, std::string{name});
        return ModuleHandle{id};
    }

    template <typename T>
    const TypeInfo* registerType(ModuleHandle module, std::string_view name)
    {
        return registerTypeWithFields<T>(module, name);
    }

    template <typename T, typename... FieldDescriptors>
    const TypeInfo* registerTypeWithFields(ModuleHandle module, std::string_view name, FieldDescriptors&&... fields)
    {
        const TypeId typeId = TypeIdOf<T>();

        auto it = m_types.find(typeId);
        if (it != m_types.end()) {
            if (it->second.module() == module) {
                return &it->second;
            }
        }

        auto [insertIt, inserted] = m_types.emplace(
            typeId,
            TypeInfo{typeId, std::string{name}, sizeof(T), module}
        );

        if (!inserted) {
            insertIt->second = TypeInfo{typeId, std::string{name}, sizeof(T), module};
        }

        TypeInfo* info = &insertIt->second;
        (info->addField(std::get<0>(fields), std::get<1>(fields), std::get<2>(fields)), ...);
        m_nameToId.emplace(std::string{name}, typeId);
        m_moduleTypes[module.id()].push_back(typeId);

        return info;
    }

    const TypeInfo* findType(TypeId typeId) const noexcept
    {
        auto it = m_types.find(typeId);
        return (it != m_types.end()) ? &it->second : nullptr;
    }

    const TypeInfo* findType(std::string_view name) const noexcept
    {
        auto it = m_nameToId.find(std::string{name});
        if (it == m_nameToId.end()) {
            return nullptr;
        }
        return findType(it->second);
    }

    bool unregisterType(TypeId typeId)
    {
        auto it = m_types.find(typeId);
        if (it == m_types.end()) {
            return false;
        }

        ModuleHandle module = it->second.module();
        m_nameToId.erase(it->second.name());
        m_types.erase(it);

        auto moduleIt = m_moduleTypes.find(module.id());
        if (moduleIt != m_moduleTypes.end()) {
            auto& vec = moduleIt->second;
            vec.erase(std::remove(vec.begin(), vec.end(), typeId), vec.end());
        }

        return true;
    }

    void unregisterModule(ModuleHandle module)
    {
        auto it = m_moduleTypes.find(module.id());
        if (it == m_moduleTypes.end()) {
            return;
        }

        for (TypeId typeId : it->second) {
            auto typeIt = m_types.find(typeId);
            if (typeIt != m_types.end()) {
                m_nameToId.erase(typeIt->second.name());
                m_types.erase(typeIt);
            }
        }

        m_moduleTypes.erase(it);
        m_moduleNames.erase(module.id());
    }

private:
    std::unordered_map<TypeId, TypeInfo> m_types;
    std::unordered_map<std::string, TypeId> m_nameToId;
    std::unordered_map<std::uint32_t, std::vector<TypeId>> m_moduleTypes;
    std::unordered_map<std::uint32_t, std::string> m_moduleNames;
    std::uint32_t m_nextModuleId{0};
};

}  // namespace sprout::detail