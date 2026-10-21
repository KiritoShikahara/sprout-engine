#include "core/Guid.hpp"

#include <gtest/gtest.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace {

using sprout::Guid;

constexpr std::string_view kSampleText = "6f1c0a2b-3c4d-4e5f-8a9b-0c1d2e3f4a5b";

Guid sampleGuid()
{
    const std::optional<Guid> parsed = Guid::parse(kSampleText);
    EXPECT_TRUE(parsed.has_value());
    return parsed.value_or(Guid{});
}

TEST(GuidTests, DefaultConstructor_IsNil)
{
    const Guid guid;

    EXPECT_TRUE(guid.isNil());
    EXPECT_EQ(guid.bytes(), Guid::Bytes{});
    EXPECT_EQ(guid.toString(), "00000000-0000-0000-0000-000000000000");
}

TEST(GuidTests, Nil_ReturnsNilGuid)
{
    EXPECT_TRUE(Guid::nil().isNil());
}

TEST(GuidTests, Generate_ProducesVersion4Variant1Guid)
{
    for (int iteration = 0; iteration < 128; ++iteration) {
        const Guid guid = Guid::generate();

        EXPECT_FALSE(guid.isNil());
        EXPECT_EQ(guid.bytes()[6] & 0xF0U, 0x40U);
        EXPECT_EQ(guid.bytes()[8] & 0xC0U, 0x80U);
        EXPECT_EQ(guid.toString().size(), Guid::kStringLength);
    }
}

TEST(GuidTests, Generate_ProducesDistinctValues)
{
    std::unordered_set<Guid> values;
    constexpr std::size_t kCount = 1024;
    values.reserve(kCount);

    for (std::size_t index = 0; index < kCount; ++index) {
        values.insert(Guid::generate());
    }

    EXPECT_EQ(values.size(), kCount);
}

TEST(GuidTests, Parse_CanonicalString_StoresBytesInOrder)
{
    const Guid guid = sampleGuid();

    const Guid::Bytes expected = {
        0x6F, 0x1C, 0x0A, 0x2B, 0x3C, 0x4D, 0x4E, 0x5F, 0x8A, 0x9B, 0x0C, 0x1D, 0x2E, 0x3F, 0x4A, 0x5B,
    };
    EXPECT_EQ(guid.bytes(), expected);
    EXPECT_FALSE(guid.isNil());
}

TEST(GuidTests, ToString_ReturnsCanonicalLowercaseForm)
{
    EXPECT_EQ(sampleGuid().toString(), kSampleText);
}

TEST(GuidTests, Parse_ToString_RoundTrips)
{
    const Guid original = sampleGuid();
    const std::optional<Guid> restored = Guid::parse(original.toString());

    ASSERT_TRUE(restored.has_value());
    EXPECT_EQ(*restored, original);
    EXPECT_EQ(restored->toString(), original.toString());
}

TEST(GuidTests, Parse_AcceptsUppercaseHex)
{
    const Guid lowercase = sampleGuid();
    const std::string uppercase = "6F1C0A2B-3C4D-4E5F-8A9B-0C1D2E3F4A5B";

    const std::optional<Guid> parsed = Guid::parse(uppercase);

    ASSERT_TRUE(parsed.has_value());
    EXPECT_EQ(*parsed, lowercase);
}

TEST(GuidTests, Parse_InvalidStrings_ReturnsNullopt)
{
    const std::vector<std::string> invalidTexts = {
        "",
        "6f1c0a2b-3c4d-4e5f-8a9b-0c1d2e3f4a5",
        "6f1c0a2b-3c4d-4e5f-8a9b-0c1d2e3f4a5bc",
        "6f1c0a2b3c4d4e5f8a9b0c1d2e3f4a5b00",
        "6f1c0a2b_3c4d-4e5f-8a9b-0c1d2e3f4a5b",
        "6f1c0a2b-3c4d-4e5f-8a9b-0c1d2e3f4a5g",
        "6f1c0a2b-3c4d-4e5f-8a9b0c1d-2e3f4a5b",
        "6f1c0a2b-3c4d4-e5f-8a9b-0c1d2e3f4a5b",
    };

    for (const std::string& text : invalidTexts) {
        EXPECT_FALSE(Guid::parse(text).has_value()) << text;
    }
}

TEST(GuidTests, Equality_SameBytes_ReturnsTrue)
{
    const Guid first = sampleGuid();
    const Guid second = Guid::parse(kSampleText).value_or(Guid{});

    EXPECT_EQ(first, second);
}

TEST(GuidTests, Equality_DifferentBytes_ReturnsFalse)
{
    const Guid first = sampleGuid();
    const Guid second = Guid::generate();

    EXPECT_NE(first, second);
}

TEST(GuidTests, Hash_SameValue_ReturnsSameHash)
{
    const Guid first = sampleGuid();
    const Guid second = Guid::parse(kSampleText).value_or(Guid{});

    EXPECT_EQ(first.hash(), second.hash());
    EXPECT_EQ(std::hash<Guid>{}(first), first.hash());
}

TEST(GuidTests, Hash_DifferentValues_AreSpreadAcrossBuckets)
{
    const std::vector<std::string> texts = {
        "00000000-0000-0000-0000-000000000001",
        "00000000-0000-0000-0000-000000000002",
        "01020304-0506-0708-090a-0b0c0d0e0f10",
        "ffffffff-ffff-ffff-ffff-ffffffffffff",
        "6f1c0a2b-3c4d-4e5f-8a9b-0c1d2e3f4a5b",
    };

    std::unordered_set<std::size_t> hashes;
    for (const std::string& text : texts) {
        const Guid guid = Guid::parse(text).value_or(Guid{});
        hashes.insert(guid.hash());
    }

    EXPECT_EQ(hashes.size(), texts.size());
}

TEST(GuidTests, Hash_WorksInUnorderedSet)
{
    std::unordered_set<Guid> values;
    const Guid guid = sampleGuid();

    values.insert(guid);
    values.insert(guid);

    EXPECT_EQ(values.size(), 1U);
    EXPECT_EQ(values.count(sampleGuid()), 1U);
}

}  // namespace
