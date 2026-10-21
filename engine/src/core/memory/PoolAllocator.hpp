#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <new>

namespace sprout::detail {

/// @brief フレーム内の一時確保に使うバンプアロケータ。
///
/// 確保位置を進めるだけで解放は行わず、フレーム終了時に reset() で一括して戻す。
/// 内部バッファを所有する生成方法と、外部バッファを借りる生成方法がある。
/// 例外は投げず、容量不足時は nullptr を返す。
///
/// @code
/// sprout::detail::FrameAllocator allocator{4096};
/// void* block = allocator.allocate(128, 16);
/// allocator.reset();
/// @endcode
class FrameAllocator {
public:
    /// 内部バッファを確保して生成する。
    /// @param capacityBytes バッファの大きさ（バイト）。
    /// @param alignment 内部バッファのアラインメント。2 の累乗であること。
    explicit FrameAllocator(std::size_t capacityBytes, std::size_t alignment = alignof(std::max_align_t)) noexcept;

    /// 外部バッファを借りて生成する。バッファの所有権は呼び出し側が持つ。
    /// @param buffer 確保元のバッファ。
    /// @param capacityBytes バッファの大きさ（バイト）。
    FrameAllocator(void* buffer, std::size_t capacityBytes) noexcept;

    /// 内部バッファを解放する。外部バッファは解放しない。
    ~FrameAllocator();

    FrameAllocator(const FrameAllocator&) = delete;
    FrameAllocator& operator=(const FrameAllocator&) = delete;
    FrameAllocator(FrameAllocator&&) = delete;
    FrameAllocator& operator=(FrameAllocator&&) = delete;

    /// バッファの先頭から size バイトを確保する。
    /// @param size 確保する大きさ（バイト）。0 は不可。
    /// @param alignment 返すポインタのアラインメント。2 の累乗であること。
    /// @return 確保した領域。容量不足または引数が不正なら nullptr。
    [[nodiscard]] void* allocate(std::size_t size, std::size_t alignment = alignof(std::max_align_t)) noexcept;

    /// 確保位置を先頭へ戻す。バッファの内容は変更しない。
    void reset() noexcept;

    /// バッファの大きさを返す。
    /// @return 容量（バイト）。
    [[nodiscard]] std::size_t capacity() const noexcept
    {
        return m_capacity;
    }

    /// 現在確保済みの大きさを返す。
    /// @return 使用量（バイト）。
    [[nodiscard]] std::size_t used() const noexcept
    {
        return m_offset;
    }

    /// 残りの大きさを返す。
    /// @return 未使用量（バイト）。
    [[nodiscard]] std::size_t remaining() const noexcept
    {
        return m_capacity - m_offset;
    }

    /// ポインタが自身のバッファ内を指すか判定する。
    /// @param pointer 判定するポインタ。
    /// @return バッファ内なら true。
    [[nodiscard]] bool owns(const void* pointer) const noexcept;

private:
    std::byte* m_buffer{nullptr};
    std::size_t m_capacity{0};
    std::size_t m_offset{0};
    std::size_t m_alignment{0};
    bool m_owned{false};
};

/// @brief 固定サイズのブロックを再利用するプールアロケータ。
///
/// 同じ大きさのブロックを事前に確保し、解放したブロックをフリーリストへ戻す。
/// allocate() は O(1)、deallocate() は二重解放を検出するため O(n) で判定する。
/// 例外は投げず、空のときは nullptr を返す。
///
/// @code
/// sprout::detail::PoolAllocator pool{sizeof(Particle), 1024};
/// void* block = pool.allocate();
/// pool.deallocate(block);
/// @endcode
class PoolAllocator {
private:
    struct FreeNode {
        FreeNode* next;
    };

public:
    /// ブロックの既定アラインメント。
    static constexpr std::size_t kDefaultAlignment = alignof(std::max_align_t);

    /// ブロック数分の領域を確保して生成する。
    /// @param blockSize 1 ブロックの大きさ（バイト）。0 は不可。
    /// @param blockCount ブロック数。0 は不可。
    /// @param alignment 各ブロックのアラインメント。2 の累乗であること。
    explicit PoolAllocator(std::size_t blockSize, std::size_t blockCount,
                           std::size_t alignment = kDefaultAlignment) noexcept
        : m_blockSize{blockSize}
        , m_blockCount{blockCount}
        , m_alignment{adjustedAlignment(alignment)}
    {
        if (m_blockSize == 0 || m_blockCount == 0 || m_alignment == 0) {
            m_blockCount = 0;
            return;
        }

        const std::size_t minimumStride = m_blockSize < sizeof(FreeNode) ? sizeof(FreeNode) : m_blockSize;
        m_stride = alignUp(minimumStride, m_alignment);
        if (m_stride == 0 || m_blockCount > (std::numeric_limits<std::size_t>::max)() / m_stride) {
            m_blockCount = 0;
            return;
        }

        m_capacityBytes = m_stride * m_blockCount;
        m_buffer = static_cast<std::byte*>(
            ::operator new(m_capacityBytes, std::align_val_t{m_alignment}, std::nothrow));
        if (m_buffer == nullptr) {
            m_capacityBytes = 0;
            m_blockCount = 0;
            return;
        }
        reset();
    }

    /// 確保した領域を解放する。
    ~PoolAllocator()
    {
        if (m_buffer != nullptr) {
            ::operator delete(m_buffer, std::align_val_t{m_alignment});
        }
    }

    PoolAllocator(const PoolAllocator&) = delete;
    PoolAllocator& operator=(const PoolAllocator&) = delete;
    PoolAllocator(PoolAllocator&&) = delete;
    PoolAllocator& operator=(PoolAllocator&&) = delete;

    /// 空きブロックを 1 つ返す。
    /// @return 確保したブロック。空きが無ければ nullptr。
    [[nodiscard]] void* allocate() noexcept
    {
        if (m_freeList == nullptr) {
            return nullptr;
        }
        FreeNode* node = m_freeList;
        m_freeList = node->next;
        ++m_usedCount;
        return node;
    }

    /// ブロックをフリーリストへ戻す。
    /// @param pointer allocate() が返したポインタ。
    /// @return 戻せたら true。外部ポインタ・不正な位置・二重解放なら false。
    bool deallocate(void* pointer) noexcept
    {
        if (!owns(pointer)) {
            return false;
        }

        const std::uintptr_t begin = reinterpret_cast<std::uintptr_t>(m_buffer);
        const std::uintptr_t address = reinterpret_cast<std::uintptr_t>(pointer);
        const std::size_t offset = static_cast<std::size_t>(address - begin);
        if (offset % m_stride != 0) {
            return false;
        }

        for (FreeNode* node = m_freeList; node != nullptr; node = node->next) {
            if (static_cast<void*>(node) == pointer) {
                return false;
            }
        }

        auto* node = static_cast<FreeNode*>(pointer);
        node->next = m_freeList;
        m_freeList = node;
        --m_usedCount;
        return true;
    }

    /// 全てのブロックを未使用に戻す。
    void reset() noexcept
    {
        m_freeList = nullptr;
        m_usedCount = 0;
        if (m_buffer == nullptr || m_stride == 0) {
            return;
        }

        for (std::size_t index = 0; index < m_blockCount; ++index) {
            auto* node = ::new (static_cast<void*>(m_buffer + index * m_stride)) FreeNode{};
            node->next = m_freeList;
            m_freeList = node;
        }
    }

    /// ポインタが自身の領域内を指すか判定する。
    /// @param pointer 判定するポインタ。
    /// @return 領域内なら true。
    [[nodiscard]] bool owns(const void* pointer) const noexcept
    {
        if (pointer == nullptr || m_buffer == nullptr) {
            return false;
        }
        const std::uintptr_t begin = reinterpret_cast<std::uintptr_t>(m_buffer);
        const std::uintptr_t address = reinterpret_cast<std::uintptr_t>(pointer);
        return address >= begin && address - begin < m_capacityBytes;
    }

    /// 1 ブロックの大きさを返す。
    /// @return 生成時に指定した大きさ（バイト）。
    [[nodiscard]] std::size_t blockSize() const noexcept
    {
        return m_blockSize;
    }

    /// ブロック数を返す。
    /// @return 生成時に指定したブロック数。
    [[nodiscard]] std::size_t blockCount() const noexcept
    {
        return m_blockCount;
    }

    /// 各ブロックのアラインメントを返す。
    /// @return 実際に使用するアラインメント。
    [[nodiscard]] std::size_t alignment() const noexcept
    {
        return m_alignment;
    }

    /// 使用中のブロック数を返す。
    /// @return 確保済みの数。
    [[nodiscard]] std::size_t usedCount() const noexcept
    {
        return m_usedCount;
    }

    /// 空きブロック数を返す。
    /// @return 未使用の数。
    [[nodiscard]] std::size_t freeCount() const noexcept
    {
        return m_blockCount - m_usedCount;
    }

private:
    static constexpr bool isPowerOfTwo(std::size_t value) noexcept
    {
        return value != 0 && (value & (value - 1)) == 0;
    }

    static constexpr std::size_t adjustedAlignment(std::size_t alignment) noexcept
    {
        constexpr std::size_t kMinimumAlignment = alignof(FreeNode);
        if (!isPowerOfTwo(alignment)) {
            return 0;
        }
        return alignment < kMinimumAlignment ? kMinimumAlignment : alignment;
    }

    static constexpr std::size_t alignUp(std::size_t value, std::size_t alignment) noexcept
    {
        const std::size_t mask = alignment - 1;
        if (value > (std::numeric_limits<std::size_t>::max)() - mask) {
            return 0;
        }
        return (value + mask) & ~mask;
    }

    std::byte* m_buffer{nullptr};
    FreeNode* m_freeList{nullptr};
    std::size_t m_blockSize{0};
    std::size_t m_stride{0};
    std::size_t m_blockCount{0};
    std::size_t m_capacityBytes{0};
    std::size_t m_usedCount{0};
    std::size_t m_alignment{0};
};

}  // namespace sprout::detail
