#include "core/Hash.hpp"

#include <cstddef>
#include <gtest/gtest.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace {

using sprout::hash64;
using sprout::hashBytes;
using sprout::HashValue;
using sprout::StringId;

TEST(HashTests, KnownVectors_MatchXxHash64)
{
    EXPECT_EQ(hash64(""), 0xEF46DB3751D8E999ULL);
    EXPECT_EQ(hash64("a"), 0xD24EC4F1A98C6E5BULL);
    EXPECT_EQ(hash64("abc"), 0x44BC2CF5AD770999ULL);
    EXPECT_EQ(hash64("message digest"), 0x066ED728FCEEB3BEULL);
}

TEST(HashTests, KnownVector_LongInput_UsesBlockPath)
{
    const std::string text(100, 'x');

    EXPECT_EQ(hash64(text), 0x92F0DE5A88A3C094ULL);
}

TEST(HashTests, Seed_ProducesXxHash64Value)
{
    EXPECT_EQ(hash64("abc", 1), 0xBEA9CA8199328908ULL);
    EXPECT_NE(hash64("abc"), hash64("abc", 1));
}

TEST(HashTests, ByteRange_MatchesStringOverload)
{
    const std::string text{"Sprout Engine"};

    EXPECT_EQ(hashBytes(text.data(), text.size()), hash64(std::string_view{text}));
}

TEST(HashTests, NullData_WithZeroSize_MatchesEmptyString)
{
    EXPECT_EQ(hashBytes(static_cast<const void*>(nullptr), 0U), hash64(""));
}

TEST(HashTests, DifferentInputs_ProduceDifferentValues)
{
    EXPECT_NE(hash64("sprout"), hash64("Sprout"));
    EXPECT_NE(hash64("abc"), hash64("abcd"));
}

TEST(StringIdTests, DefaultConstructor_IsInvalid)
{
    const StringId id;

    EXPECT_FALSE(id.isValid());
    EXPECT_FALSE(static_cast<bool>(id));
    EXPECT_EQ(id.value(), StringId::kInvalidValue);
    EXPECT_TRUE(id.name().empty());
}

TEST(StringIdTests, Intern_SameText_ReturnsSameId)
{
    const StringId first = StringId::intern("Player");
    const StringId second = StringId::intern("Player");

    EXPECT_TRUE(first.isValid());
    EXPECT_EQ(first, second);
    EXPECT_EQ(first.value(), second.value());
}

TEST(StringIdTests, Intern_DifferentText_ReturnsDifferentIds)
{
    const StringId first = StringId::intern("Player");
    const StringId second = StringId::intern("Enemy");

    EXPECT_NE(first, second);
    EXPECT_NE(first.value(), second.value());
}

TEST(StringIdTests, Name_ReturnsInternedText)
{
    const StringId id = StringId::intern("Assets/Player.png");

    EXPECT_EQ(id.name(), "Assets/Player.png");
    EXPECT_STREQ(id.c_str(), "Assets/Player.png");
    EXPECT_EQ(id.toString(), "Assets/Player.png");
}

TEST(StringIdTests, Intern_MultipleTexts_KeepTheirOwnNames)
{
    const StringId first = StringId::intern("Alpha");
    const StringId second = StringId::intern("Beta");

    EXPECT_EQ(first.name(), "Alpha");
    EXPECT_EQ(second.name(), "Beta");
}

TEST(StringIdTests, Intern_EmptyString_IsValid)
{
    const StringId id = StringId::intern("");

    EXPECT_TRUE(id.isValid());
    EXPECT_TRUE(id.name().empty());
    EXPECT_STREQ(id.c_str(), "");
}

TEST(StringIdTests, Intern_EmbeddedNull_PreservesLength)
{
    const std::string_view text{"a\0b", 3};
    const StringId id = StringId::intern(text);

    EXPECT_EQ(id.name(), text);
    EXPECT_EQ(id.name().size(), 3U);
    EXPECT_NE(id, StringId::intern("a"));
}

TEST(StringIdTests, Value_MatchesXxHash64ForUniqueText)
{
    const StringId id = StringId::intern("StringId.ValueMatch");

    EXPECT_EQ(id.value(), hash64("StringId.ValueMatch"));
}

TEST(StringIdTests, Hash_WorksInUnorderedContainers)
{
    std::unordered_set<StringId> ids;
    std::unordered_map<StringId, int> counts;

    ids.insert(StringId::intern("Key"));
    ids.insert(StringId::intern("Key"));
    counts[StringId::intern("Key")] = 1;

    EXPECT_EQ(ids.size(), 1U);
    EXPECT_EQ(ids.count(StringId::intern("Key")), 1U);
    EXPECT_EQ(counts.size(), 1U);
    EXPECT_EQ(counts.at(StringId::intern("Key")), 1);
}

TEST(StringIdTests, DefaultHash_MatchesValueHash)
{
    const StringId id = StringId::intern("Hashed");

    EXPECT_EQ(std::hash<StringId>{}(id), id.hash());
}

}  // namespace
