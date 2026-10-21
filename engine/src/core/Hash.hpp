#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>

namespace sprout {

/// @brief 64 ビットのハッシュ値。
using HashValue = std::uint64_t;

/// @brief hash64() の既定シード。
inline constexpr HashValue kDefaultHashSeed = 0;

/// @brief バイト列を xxHash64 でハッシュする。
/// @param data 対象のバイト列。nullptr の場合は空列として扱う。
/// @param size バイト数。
/// @param seed シード値。同じ入力でもシードが違えば別の値になる。
/// @return xxHash64 のハッシュ値。
[[nodiscard]] HashValue hashBytes(const void* data, std::size_t size, HashValue seed = kDefaultHashSeed) noexcept;

/// @brief 文字列を xxHash64 でハッシュする。
/// @param text 対象の文字列。末尾の NUL は含めない。
/// @param seed シード値。同じ入力でもシードが違えば別の値になる。
/// @return xxHash64 のハッシュ値。
[[nodiscard]] HashValue hash64(std::string_view text, HashValue seed = kDefaultHashSeed) noexcept;

/// @brief 文字列をインターンして 64 ビット ID を割り当てる型。
///
/// 同じ内容の文字列には常に同じ ID を返す。ID から元の文字列を参照できる。
/// スレッド安全。
///
/// @code
/// const sprout::StringId id = sprout::StringId::intern("Player");
/// const std::string_view name = id.name(); // "Player"
/// @endcode
class StringId {
public:
    /// @brief 無効な ID を表す値。
    static constexpr HashValue kInvalidValue = 0;

    /// @brief 無効な ID を生成する。
    constexpr StringId() noexcept = default;

    /// @brief 値から生成する。
    /// @param value ハッシュ値。
    constexpr explicit StringId(HashValue value) noexcept
        : m_value{value}
    {
    }

    /// @brief 文字列をインターンして ID を返す。
    /// @param text 対象の文字列。
    /// @return インターン済みの ID。同じ内容なら常に同じ値。
    [[nodiscard]] static StringId intern(std::string_view text);

    /// @brief ハッシュ値を返す。
    /// @return 64 ビットの値。
    [[nodiscard]] constexpr HashValue value() const noexcept
    {
        return m_value;
    }

    /// @brief インターンした文字列を返す。
    /// @return 文字列への参照。インターン表が保持するためプログラム終了まで有効。
    ///         未知の値の場合は空。
    [[nodiscard]] std::string_view name() const noexcept;

    /// @brief インターンした文字列へのポインタを返す。
    /// @return NUL 終端された文字列。未知の値の場合は空文字列。
    [[nodiscard]] const char* c_str() const noexcept;

    /// @brief インターンした文字列を std::string で返す。
    /// @return 文字列のコピー。未知の値の場合は空。
    [[nodiscard]] std::string toString() const;

    /// @brief 有効な ID か判定する。
    /// @return 既定構築した ID なら false。
    [[nodiscard]] constexpr bool isValid() const noexcept
    {
        return m_value != kInvalidValue;
    }

    /// @brief 有効な ID か判定する。
    /// @return 既定構築した ID なら false。
    explicit constexpr operator bool() const noexcept
    {
        return isValid();
    }

    /// @brief 同じ値を持つか判定する。
    /// @param other 比較対象。
    /// @return 値が一致すれば true。
    constexpr bool operator==(const StringId& other) const noexcept = default;

    /// @brief ハッシュ表で使うハッシュ値を返す。
    /// @return 64 ビット値を std::size_t に変換した値。
    [[nodiscard]] std::size_t hash() const noexcept;

private:
    HashValue m_value{kInvalidValue};
};

}  // namespace sprout

/// @brief sprout::StringId のハッシュ。
template <>
struct std::hash<sprout::StringId> {
    /// @brief ハッシュ値を返す。
    /// @param value 対象の ID。
    /// @return StringId::hash() と同じ値。
    std::size_t operator()(const sprout::StringId& value) const noexcept
    {
        return value.hash();
    }
};
