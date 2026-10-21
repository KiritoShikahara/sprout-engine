#include "core/memory/PoolAllocator.hpp"

#include <cstddef>
#include <cstdint>
#include <gtest/gtest.h>

namespace {

using sprout::detail::FrameAllocator;
using sprout::detail::PoolAllocator;

TEST(FrameAllocatorTests, Constructor_WithCapacity_IsEmpty)
{
    FrameAllocator allocator{128};

    EXPECT_EQ(allocator.capacity(), 128U);
    EXPECT_EQ(allocator.used(), 0U);
    EXPECT_EQ(allocator.remaining(), 128U);
}

TEST(FrameAllocatorTests, Allocate_Sequential_ReturnsDistinctBlocks)
{
    FrameAllocator allocator{256};

    void* first = allocator.allocate(16);
    void* second = allocator.allocate(16);

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    EXPECT_NE(first, second);
    EXPECT_GE(allocator.used(), 32U);
    EXPECT_LE(allocator.used(), allocator.capacity());
}

TEST(FrameAllocatorTests, Allocate_HonorsAlignment)
{
    FrameAllocator allocator{256, 64};

    void* first = allocator.allocate(1, 1);
    void* second = allocator.allocate(16, 32);

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(first) % 64U, 0U);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(second) % 32U, 0U);
}

TEST(FrameAllocatorTests, Allocate_OutOfMemory_ReturnsNullAndKeepsUsed)
{
    FrameAllocator allocator{64};

    ASSERT_NE(allocator.allocate(48), nullptr);
    const std::size_t usedBefore = allocator.used();

    EXPECT_EQ(allocator.allocate(32), nullptr);
    EXPECT_EQ(allocator.used(), usedBefore);
}

TEST(FrameAllocatorTests, Allocate_ZeroSize_ReturnsNull)
{
    FrameAllocator allocator{64};

    EXPECT_EQ(allocator.allocate(0), nullptr);
    EXPECT_EQ(allocator.used(), 0U);
}

TEST(FrameAllocatorTests, Allocate_InvalidAlignment_ReturnsNull)
{
    FrameAllocator allocator{64};

    EXPECT_EQ(allocator.allocate(8, 0), nullptr);
    EXPECT_EQ(allocator.allocate(8, 3), nullptr);
    EXPECT_EQ(allocator.used(), 0U);
}

TEST(FrameAllocatorTests, Reset_RestoresCapacityAndReusesBuffer)
{
    FrameAllocator allocator{128};

    void* first = allocator.allocate(32);
    ASSERT_NE(first, nullptr);
    EXPECT_GT(allocator.used(), 0U);

    allocator.reset();

    EXPECT_EQ(allocator.used(), 0U);
    EXPECT_EQ(allocator.remaining(), 128U);
    EXPECT_EQ(allocator.allocate(32), first);
}

TEST(FrameAllocatorTests, ExternalBuffer_AllocatesInsideBuffer)
{
    alignas(64) std::byte buffer[128]{};
    FrameAllocator allocator{buffer, sizeof(buffer)};

    void* block = allocator.allocate(16);

    ASSERT_NE(block, nullptr);
    EXPECT_EQ(allocator.capacity(), 128U);
    EXPECT_TRUE(allocator.owns(block));
    EXPECT_FALSE(allocator.owns(buffer + sizeof(buffer)));
    EXPECT_FALSE(allocator.owns(nullptr));
}

TEST(PoolAllocatorTests, Constructor_ReportsCapacity)
{
    PoolAllocator pool{32, 4};

    EXPECT_EQ(pool.blockSize(), 32U);
    EXPECT_EQ(pool.blockCount(), 4U);
    EXPECT_EQ(pool.usedCount(), 0U);
    EXPECT_EQ(pool.freeCount(), 4U);
}

TEST(PoolAllocatorTests, Allocate_UntilEmpty_ReturnsDistinctBlocks)
{
    PoolAllocator pool{32, 4};
    void* blocks[4]{};

    for (void*& block : blocks) {
        block = pool.allocate();
        ASSERT_NE(block, nullptr);
        EXPECT_TRUE(pool.owns(block));
    }

    EXPECT_NE(blocks[0], blocks[1]);
    EXPECT_NE(blocks[0], blocks[2]);
    EXPECT_NE(blocks[0], blocks[3]);
    EXPECT_NE(blocks[1], blocks[2]);
    EXPECT_NE(blocks[1], blocks[3]);
    EXPECT_NE(blocks[2], blocks[3]);
    EXPECT_EQ(pool.usedCount(), 4U);
    EXPECT_EQ(pool.freeCount(), 0U);
    EXPECT_EQ(pool.allocate(), nullptr);
}

TEST(PoolAllocatorTests, Allocate_HonorsAlignment)
{
    PoolAllocator pool{24, 2, 64};

    void* first = pool.allocate();
    void* second = pool.allocate();

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(first) % 64U, 0U);
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(second) % 64U, 0U);
}

TEST(PoolAllocatorTests, Deallocate_ReturnsBlockToPool)
{
    PoolAllocator pool{32, 2};

    void* first = pool.allocate();
    void* second = pool.allocate();
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_TRUE(pool.deallocate(first));
    EXPECT_EQ(pool.usedCount(), 1U);
    EXPECT_EQ(pool.freeCount(), 1U);
    EXPECT_EQ(pool.allocate(), first);
    EXPECT_EQ(pool.usedCount(), 2U);
}

TEST(PoolAllocatorTests, Deallocate_DoubleFree_ReturnsFalse)
{
    PoolAllocator pool{32, 2};

    void* block = pool.allocate();
    ASSERT_NE(block, nullptr);
    EXPECT_TRUE(pool.deallocate(block));

    const std::size_t usedBefore = pool.usedCount();
    EXPECT_FALSE(pool.deallocate(block));
    EXPECT_EQ(pool.usedCount(), usedBefore);
}

TEST(PoolAllocatorTests, Deallocate_ForeignPointer_ReturnsFalse)
{
    PoolAllocator pool{32, 2};
    int value = 0;

    EXPECT_FALSE(pool.deallocate(&value));
    EXPECT_FALSE(pool.deallocate(nullptr));
}

TEST(PoolAllocatorTests, Deallocate_MisalignedPointer_ReturnsFalse)
{
    PoolAllocator pool{32, 2};

    void* block = pool.allocate();
    ASSERT_NE(block, nullptr);

    auto* bytes = static_cast<std::byte*>(block);
    EXPECT_FALSE(pool.deallocate(bytes + 1));
    EXPECT_EQ(pool.usedCount(), 1U);
}

TEST(PoolAllocatorTests, Reset_ReclaimsAllBlocks)
{
    PoolAllocator pool{32, 3};

    for (int index = 0; index < 3; ++index) {
        ASSERT_NE(pool.allocate(), nullptr);
    }
    EXPECT_EQ(pool.freeCount(), 0U);

    pool.reset();

    EXPECT_EQ(pool.usedCount(), 0U);
    EXPECT_EQ(pool.freeCount(), 3U);
    EXPECT_NE(pool.allocate(), nullptr);
}

TEST(PoolAllocatorTests, BlockSizeSmallerThanPointer_StillAllocatesDistinctBlocks)
{
    PoolAllocator pool{4, 2};

    void* first = pool.allocate();
    void* second = pool.allocate();

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    EXPECT_NE(first, second);
    EXPECT_TRUE(pool.owns(first));
    EXPECT_TRUE(pool.owns(second));
}

TEST(PoolAllocatorTests, InvalidArguments_AllocationFails)
{
    PoolAllocator empty{16, 0};
    PoolAllocator invalidAlignment{16, 2, 3};

    EXPECT_EQ(empty.allocate(), nullptr);
    EXPECT_EQ(invalidAlignment.allocate(), nullptr);
}

}  // namespace
