#include "core/Hash.hpp"

#include <mutex>
#include <string>
#include <unordered_map>

#include <xxhash.h>

namespace {

struct InternTable {
    std::mutex mutex;
    std::unordered_map<sprout::HashValue, std::string> byValue;
};

InternTable& internTable()
{
    static InternTable table;
    return table;
}

}  // namespace

namespace sprout {

HashValue hashBytes(const void* data, std::size_t size, HashValue seed) noexcept
{
    if (data == nullptr) {
        return static_cast<HashValue>(XXH64("", 0, static_cast<XXH64_hash_t>(seed)));
    }
    return static_cast<HashValue>(XXH64(data, size, static_cast<XXH64_hash_t>(seed)));
}

HashValue hash64(std::string_view text, HashValue seed) noexcept
{
    return hashBytes(text.data(), text.size(), seed);
}

StringId StringId::intern(std::string_view text)
{
    InternTable& table = internTable();
    const std::lock_guard lock(table.mutex);

    HashValue value = hash64(text);
    while (true) {
        if (value == kInvalidValue) {
            ++value;
            continue;
        }

        const auto found = table.byValue.find(value);
        if (found == table.byValue.end()) {
            table.byValue.emplace(value, std::string{text});
            return StringId{value};
        }
        if (std::string_view{found->second} == text) {
            return StringId{value};
        }
        ++value;
    }
}

std::string_view StringId::name() const noexcept
{
    if (!isValid()) {
        return {};
    }

    InternTable& table = internTable();
    const std::lock_guard lock(table.mutex);
    const auto found = table.byValue.find(m_value);
    if (found == table.byValue.end()) {
        return {};
    }
    return std::string_view{found->second};
}

const char* StringId::c_str() const noexcept
{
    const std::string_view text = name();
    return text.empty() ? "" : text.data();
}

std::string StringId::toString() const
{
    return std::string{name()};
}

std::size_t StringId::hash() const noexcept
{
    return static_cast<std::size_t>(m_value);
}

}  // namespace sprout
