#include "reflection/TypeInfo.hpp"

namespace sprout::detail {

TypeRegistry::TypeRegistry() = default;

TypeRegistry::~TypeRegistry() = default;

TypeRegistry::TypeRegistry(TypeRegistry&& other) noexcept
    : m_types{std::move(other.m_types)}
    , m_nameToId{std::move(other.m_nameToId)}
    , m_moduleTypes{std::move(other.m_moduleTypes)}
    , m_moduleNames{std::move(other.m_moduleNames)}
    , m_nextModuleId{other.m_nextModuleId}
{
}

TypeRegistry& TypeRegistry::operator=(TypeRegistry&& other) noexcept
{
    if (this != &other) {
        m_types = std::move(other.m_types);
        m_nameToId = std::move(other.m_nameToId);
        m_moduleTypes = std::move(other.m_moduleTypes);
        m_moduleNames = std::move(other.m_moduleNames);
        m_nextModuleId = other.m_nextModuleId;
    }
    return *this;
}

ModuleHandle TypeRegistry::createModule(std::string_view name)
{
    std::uint32_t id = ++m_nextModuleId;
    m_moduleNames.emplace(id, std::string{name});
    return ModuleHandle{id};
}

bool TypeRegistry::unregisterType(TypeId typeId)
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

void TypeRegistry::unregisterModule(ModuleHandle module)
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

}  // namespace sprout::detail