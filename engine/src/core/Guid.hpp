#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>

namespace sprout {

/// @brief アセットや永続データを一意に識別する 128 ビット GUID。
///
/// 文字列表現は RFC 4122 の 8-4-4-4-12 形式（小文字 16 進）を使う。
/// generate() は RFC 4122 バージョン 4（乱数）の GUID を生成する。
///
/// @code
/// const sprout::Guid id = sprout::Guid::generate();
/// const std::string text = id.toString();
/// const std::optional<sprout::Guid> restored = sprout::Guid::parse(text);
/// const std::size_t hash = id.hash();
/// @endcode
class Guid {
public:
    /// 16 バイトの生データ。
    using Bytes = std::array<std::uint8_t, 16>;

    /// 文字列表現の長さ（ハイフン 4 個を含む）。
    static constexpr std::size_t kStringLength = 36;

    /// 全ビットがゼロの GUID を生成する。
    constexpr Guid() noexcept = default;

    /// 生データを指定して生成する。
    /// @param bytes 16 バイトのデータ。
    constexpr explicit Guid(const Bytes& bytes) noexcept
        : m_bytes{bytes}
    {
    }

    /// ランダムなバージョン 4 の GUID を生成する。
    /// @return 生成した GUID。全ビットがゼロになることはない。
    static Guid generate() noexcept;

    /// 文字列を GUID として解析する。
    /// @param text 8-4-4-4-12 形式（16 進は大文字小文字を問わない）の文字列。
    /// @return 解析できた GUID。形式が不正なら std::nullopt。
    static std::optional<Guid> parse(std::string_view text) noexcept;

    /// 全ビットがゼロの GUID を返す。
    /// @return nil GUID。
    static constexpr Guid nil() noexcept
    {
        return {};
    }

    /// 全ビットがゼロか判定する。
    /// @return ゼロなら true。
    [[nodiscard]] constexpr bool isNil() const noexcept
    {
        return m_bytes == Bytes{};
    }

    /// 生データを返す。
    /// @return 16 バイトのデータへの参照。
    [[nodiscard]] constexpr const Bytes& bytes() const noexcept
    {
        return m_bytes;
    }

    /// 8-4-4-4-12 形式の文字列に変換する。
    /// @return 小文字 16 進の文字列表現。
    [[nodiscard]] std::string toString() const;

    /// ハッシュ値を返す。
    /// @return 文字列表現に依存しないハッシュ値。
    [[nodiscard]] std::size_t hash() const noexcept;

    /// 全ビットが完全一致するか判定する。
    /// @param other 比較対象。
    /// @return 一致すれば true。
    constexpr bool operator==(const Guid& other) const noexcept = default;

private:
    Bytes m_bytes{};
};

}  // namespace sprout

/// @brief sprout::Guid のハッシュ。
template <>
struct std::hash<sprout::Guid> {
    /// @brief ハッシュ値を返す。
    /// @param value 対象の GUID。
    /// @return Guid::hash() と同じ値。
    std::size_t operator()(const sprout::Guid& value) const noexcept
    {
        return value.hash();
    }
};
