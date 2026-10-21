#pragma once

#include <cstdint>

namespace sprout {

/// @brief RGBA 色（各成分 8 ビット）。
///
/// 成分は 0〜255 の範囲で保持する。既定値は不透明な黒。HSV は色相を度、彩度・明度・不透明度を
/// 0〜1 の範囲で扱う。
///
/// @code
/// const sprout::Color red{255, 0, 0};
/// const sprout::Color tinted = sprout::Color::fromHsv({120.0F, 1.0F, 0.5F});
/// const sprout::Color mixed = sprout::Color::lerp(red, sprout::Color::white(), 0.5F);
/// const std::uint32_t packed = mixed.toHex(); // 0x...FF
/// @endcode
struct Color {
    /// HSV 表現。色相は度、彩度・明度・不透明度は 0〜1。
    struct Hsv {
        /// 色相（度）。0 以上 360 未満に正規化される。
        float hue{0.0F};

        /// 彩度（0〜1）。
        float saturation{0.0F};

        /// 明度（0〜1）。
        float value{0.0F};

        /// 不透明度（0〜1）。
        float alpha{1.0F};
    };

    /// 赤成分。
    std::uint8_t r{0};

    /// 緑成分。
    std::uint8_t g{0};

    /// 青成分。
    std::uint8_t b{0};

    /// 不透明度。
    std::uint8_t a{255};

    /// 不透明な黒を生成する。
    constexpr Color() noexcept = default;

    /// 成分を指定して生成する。
    /// @param red 赤成分。
    /// @param green 緑成分。
    /// @param blue 青成分。
    /// @param alpha 不透明度。
    constexpr Color(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha = 255) noexcept
        : r{red}
        , g{green}
        , b{blue}
        , a{alpha}
    {
    }

    /// 0xRRGGBBAA 形式の値を指定して生成する。
    /// @param rgba パックされた RGBA 値。
    constexpr explicit Color(std::uint32_t rgba) noexcept
        : r{static_cast<std::uint8_t>((rgba >> 24U) & 0xFFU)}
        , g{static_cast<std::uint8_t>((rgba >> 16U) & 0xFFU)}
        , b{static_cast<std::uint8_t>((rgba >> 8U) & 0xFFU)}
        , a{static_cast<std::uint8_t>(rgba & 0xFFU)}
    {
    }

    /// 不透明な黒を返す。
    /// @return {0, 0, 0, 255}。
    static constexpr Color black() noexcept
    {
        return {};
    }

    /// 不透明な白を返す。
    /// @return {255, 255, 255, 255}。
    static constexpr Color white() noexcept
    {
        return {255, 255, 255};
    }

    /// 不透明な赤を返す。
    /// @return {255, 0, 0, 255}。
    static constexpr Color red() noexcept
    {
        return {255, 0, 0};
    }

    /// 不透明な緑を返す。
    /// @return {0, 255, 0, 255}。
    static constexpr Color green() noexcept
    {
        return {0, 255, 0};
    }

    /// 不透明な青を返す。
    /// @return {0, 0, 255, 255}。
    static constexpr Color blue() noexcept
    {
        return {0, 0, 255};
    }

    /// 不透明な黄を返す。
    /// @return {255, 255, 0, 255}。
    static constexpr Color yellow() noexcept
    {
        return {255, 255, 0};
    }

    /// 不透明なマゼンタを返す。
    /// @return {255, 0, 255, 255}。
    static constexpr Color magenta() noexcept
    {
        return {255, 0, 255};
    }

    /// 不透明なシアンを返す。
    /// @return {0, 255, 255, 255}。
    static constexpr Color cyan() noexcept
    {
        return {0, 255, 255};
    }

    /// 透明な黒を返す。
    /// @return {0, 0, 0, 0}。
    static constexpr Color transparent() noexcept
    {
        return {0, 0, 0, 0};
    }

    /// 0xRRGGBBAA 形式の値から生成する。
    /// @param rgba パックされた RGBA 値。
    /// @return 対応する色。
    static constexpr Color fromHex(std::uint32_t rgba) noexcept
    {
        return Color{rgba};
    }

    /// 0xRRGGBBAA 形式の値に変換する。
    /// @return パックされた RGBA 値。
    constexpr std::uint32_t toHex() const noexcept
    {
        return (static_cast<std::uint32_t>(r) << 24U) | (static_cast<std::uint32_t>(g) << 16U)
               | (static_cast<std::uint32_t>(b) << 8U) | static_cast<std::uint32_t>(a);
    }

    /// HSV から生成する。範囲外の成分は有効範囲に丸められる。
    /// @param hsv HSV 表現。
    /// @return 対応する色。
    static Color fromHsv(const Hsv& hsv) noexcept;

    /// HSV の成分から生成する。範囲外の成分は有効範囲に丸められる。
    /// @param hue 色相（度）。
    /// @param saturation 彩度（0〜1）。
    /// @param value 明度（0〜1）。
    /// @param alpha 不透明度（0〜1）。
    /// @return 対応する色。
    static Color fromHsv(float hue, float saturation, float value, float alpha = 1.0F) noexcept;

    /// HSV 表現に変換する。
    /// @return 対応する HSV 表現。彩度がゼロの場合は色相もゼロになる。
    Hsv toHsv() const noexcept;

    /// 2 色を線形補間する。
    /// @param from 開始色。
    /// @param to 終了色。
    /// @param t 補間係数。0〜1 の範囲外は端点に丸められる。
    /// @return 補間した色。
    static Color lerp(const Color& from, const Color& to, float t) noexcept;

    /// 全成分が完全一致するか判定する。
    /// @param other 比較対象。
    /// @return 一致すれば true。
    constexpr bool operator==(const Color& other) const noexcept = default;
};

}  // namespace sprout
