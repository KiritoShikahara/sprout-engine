#include "PoolAllocator.hpp"

#include <cstdint>
#include <limits>
#include <new>

namespace {

constexpr bool isPowerOfTwo(std::size_t value) noexcept
{
    return value != 0 && (value & (value - 1)) == 0;
}

constexpr std::uintptr_t alignUpAddress(std::uintptr_t address, std::size_t alignment) noexcept
{
    const std::uintptr_t mask = static_cast<std::uintptr_t>(alignment) - 1;
    if (address > (std::numeric_limits<std::uintptr_t>::max)() - mask) {
        return 0;
    }
    return (address + mask) & ~mask;
}

}  // namespace

namespace sprout::detail {

FrameAllocator::FrameAllocator(std::size_t capacityBytes, std::size_t alignment) noexcept
    : m_capacity{capacityBytes}
    , m_alignment{alignment}
{
    if (m_capacity == 0 || !isPowerOfTwo(alignment)) {
        m_capacity = 0;
        m_alignment = 0;
        return;
    }

    m_buffer = static_cast<std::byte*>(::operator new(m_capacity, std::align_val_t{alignment}, std::nothrow));
    if (m_buffer == nullptr) {
        m_capacity = 0;
        m_alignment = 0;
        return;
    }
    m_owned = true;
}

FrameAllocator::FrameAllocator(void* buffer, std::size_t capacityBytes) noexcept
    : m_buffer{static_cast<std::byte*>(buffer)}
    , m_capacity{buffer == nullptr ? 0 : capacityBytes}
{
}

FrameAllocator::~FrameAllocator()
{
    if (m_owned && m_buffer != nullptr) {
        ::operator delete(m_buffer, std::align_val_t{m_alignment});
    }
}

void* FrameAllocator::allocate(std::size_t size, std::size_t alignment) noexcept
{
    if (m_buffer == nullptr || size == 0 || !isPowerOfTwo(alignment)) {
        return nullptr;
    }

    const std::uintptr_t begin = reinterpret_cast<std::uintptr_t>(m_buffer);
    const std::uintptr_t current = begin + m_offset;
    const std::uintptr_t alignedAddress = alignUpAddress(current, alignment);
    if (alignedAddress == 0) {
        return nullptr;
    }

    const std::size_t alignedOffset = static_cast<std::size_t>(alignedAddress - begin);
    if (alignedOffset > m_capacity || size > m_capacity - alignedOffset) {
        return nullptr;
    }

    m_offset = alignedOffset + size;
    return m_buffer + alignedOffset;
}

void FrameAllocator::reset() noexcept
{
    m_offset = 0;
}

bool FrameAllocator::owns(const void* pointer) const noexcept
{
    if (pointer == nullptr || m_buffer == nullptr) {
        return false;
    }
    const std::uintptr_t begin = reinterpret_cast<std::uintptr_t>(m_buffer);
    const std::uintptr_t address = reinterpret_cast<std::uintptr_t>(pointer);
    return address >= begin && address - begin < m_capacity;
}

}  // namespace sprout::detail
