#include "Guid.hpp"

#include <random>

namespace {

constexpr char kHexDigits[] = "0123456789abcdef";

int hexDigit(char value) noexcept
{
    if (value >= '0' && value <= '9') {
        return value - '0';
    }
    if (value >= 'a' && value <= 'f') {
        return value - 'a' + 10;
    }
    if (value >= 'A' && value <= 'F') {
        return value - 'A' + 10;
    }
    return -1;
}

std::mt19937_64& randomEngine() noexcept
{
    static thread_local std::mt19937_64 engine = [] {
        std::random_device device;
        const std::uint64_t high = static_cast<std::uint64_t>(device()) << 32U;
        const std::uint64_t low = static_cast<std::uint64_t>(device());
        return std::mt19937_64{high | low};
    }();
    return engine;
}

}  // namespace

namespace sprout {

Guid Guid::generate() noexcept
{
    const std::uint64_t high = randomEngine()();
    const std::uint64_t low = randomEngine()();

    Bytes bytes{};
    for (std::size_t index = 0; index < 8; ++index) {
        const std::size_t shift = 56U - index * 8U;
        bytes[index] = static_cast<std::uint8_t>((high >> shift) & 0xFFU);
        bytes[index + 8] = static_cast<std::uint8_t>((low >> shift) & 0xFFU);
    }

    bytes[6] = static_cast<std::uint8_t>((bytes[6] & 0x0FU) | 0x40U);
    bytes[8] = static_cast<std::uint8_t>((bytes[8] & 0x3FU) | 0x80U);
    return Guid{bytes};
}

std::optional<Guid> Guid::parse(std::string_view text) noexcept
{
    if (text.size() != kStringLength) {
        return std::nullopt;
    }

    Bytes bytes{};
    std::size_t byteIndex = 0;
    for (std::size_t index = 0; index < text.size();) {
        if (index == 8 || index == 13 || index == 18 || index == 23) {
            if (text[index] != '-') {
                return std::nullopt;
            }
            ++index;
            continue;
        }

        const int high = hexDigit(text[index]);
        const int low = hexDigit(text[index + 1]);
        if (high < 0 || low < 0) {
            return std::nullopt;
        }
        bytes[byteIndex] = static_cast<std::uint8_t>((high << 4) | low);
        ++byteIndex;
        index += 2;
    }

    if (byteIndex != bytes.size()) {
        return std::nullopt;
    }
    return Guid{bytes};
}

std::string Guid::toString() const
{
    std::string result(kStringLength, '-');
    std::size_t output = 0;
    for (std::size_t index = 0; index < m_bytes.size(); ++index) {
        if (index == 4 || index == 6 || index == 8 || index == 10) {
            ++output;
        }
        result[output] = kHexDigits[m_bytes[index] >> 4U];
        ++output;
        result[output] = kHexDigits[m_bytes[index] & 0x0FU];
        ++output;
    }
    return result;
}

std::size_t Guid::hash() const noexcept
{
    constexpr std::uint64_t kOffsetBasis = 14695981039346656037ULL;
    constexpr std::uint64_t kPrime = 1099511628211ULL;

    std::uint64_t value = kOffsetBasis;
    for (const std::uint8_t byte : m_bytes) {
        value ^= byte;
        value *= kPrime;
    }
    return static_cast<std::size_t>(value);
}

}  // namespace sprout
