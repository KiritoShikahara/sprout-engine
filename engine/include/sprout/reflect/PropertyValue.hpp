#pragma once

#include "sprout/math/Vector2.hpp"
#include "sprout/math/Rect.hpp"
#include "sprout/math/Circle.hpp"
#include "sprout/math/Color.hpp"
#include "sprout/math/Angle.hpp"
#include "sprout/core/Result.hpp"

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace sprout::reflect {

/// @brief プロパティ値のバリアント型。
///
/// 以下の型を保持できる：
/// - std::monostate（空値）
/// - bool
/// - int32_t
/// - float
/// - std::string
/// - Vector2
/// - Vector2Int
/// - Color
/// - Rect
/// - Circle
/// - Angle
/// - Guid（アセット参照用）
/// - GameObjectRef（オブジェクト参照用）
/// - Enum（列挙型、int32_t として格納）
/// - std::vector<PropertyValue>（配列）
///
/// 型変換と比較をサポートする。
///
/// @code
/// sprout::reflect::PropertyValue value = 42;
/// sprout::reflect::PropertyValue vec = sprout::Vector2{1.0F, 2.0F};
/// if (value.is<int32_t>()) { int i = value.get<int32_t>(); }
/// sprout::reflect::PropertyValue converted = value.convertTo<float>();
/// bool equal = (value == sprout::reflect::PropertyValue{42});
/// @endcode
class PropertyValue {
public:
    using Guid = std::uint64_t;
    using GameObjectRef = std::uint64_t;
    using EnumValue = std::int32_t;
    using ArrayValue = std::vector<PropertyValue>;

    using VariantType = std::variant<
        std::monostate,
        bool,
        std::int32_t,
        float,
        std::string,
        sprout::Vector2,
        sprout::Vector2Int,
        sprout::Color,
        sprout::Rect,
        sprout::Circle,
        sprout::Angle,
        Guid,
        GameObjectRef,
        EnumValue,
        ArrayValue
    >;

    /// 空の値を生成する。
    constexpr PropertyValue() noexcept = default;

    /// 指定した値で生成する。
    template <typename T>
    constexpr PropertyValue(T&& value) noexcept
        : m_value{std::forward<T>(value)}
    {
    }

    /// コピー構築・代入・ムーブ構築・代入はデフォルト。
    PropertyValue(const PropertyValue&) = default;
    PropertyValue& operator=(const PropertyValue&) = default;
    PropertyValue(PropertyValue&&) noexcept = default;
    PropertyValue& operator=(PropertyValue&&) noexcept = default;

    /// 空値か判定する。
    /// @return 値を保持していなければ true。
    [[nodiscard]] constexpr bool isEmpty() const noexcept
    {
        return std::holds_alternative<std::monostate>(m_value);
    }

    /// 指定した型を保持しているか判定する。
    /// @tparam T 判定する型。
    /// @return 指定した型なら true。
    template <typename T>
    [[nodiscard]] constexpr bool is() const noexcept
    {
        return std::holds_alternative<T>(m_value);
    }

    /// 保持している値への const 参照を返す。型が一致しない場合は未定義動作。
    /// @tparam T 取得する型。
    /// @return 値への const 参照。
    template <typename T>
    [[nodiscard]] constexpr const T& get() const noexcept
    {
        return std::get<T>(m_value);
    }

    /// 保持している値への参照を返す。型が一致しない場合は未定義動作。
    /// @tparam T 取得する型。
    /// @return 値への参照。
    template <typename T>
    [[nodiscard]] constexpr T& get() noexcept
    {
        return std::get<T>(m_value);
    }

    /// 保持している値を取得しようとする。失敗時は Error を返す。
    /// @tparam T 取得する型。
    /// @return 成功時は値、失敗時は Error。
    template <typename T>
    [[nodiscard]] sprout::Result<const T&> tryGet() const noexcept
    {
        if (auto* ptr = std::get_if<T>(&m_value)) {
            return *ptr;
        }
        return sprout::Error{sprout::ErrorCode::InvalidArgument, "型が一致しません"};
    }

    /// 保持している値を取得しようとする（非 const 版）。失敗時は Error を返す。
    /// @tparam T 取得する型。
    /// @return 成功時は値、失敗時は Error。
    template <typename T>
    [[nodiscard]] sprout::Result<T&> tryGet() noexcept
    {
        if (auto* ptr = std::get_if<T>(&m_value)) {
            return *ptr;
        }
        return sprout::Error{sprout::ErrorCode::InvalidArgument, "型が一致しません"};
    }

    /// 指定した型に変換を試みる。変換不能な組み合わせは Error を返す。
    ///
    /// サポートされる変換：
    /// - 数値型相互（int32_t ⇄ float）
    /// - bool ⇄ 数値（0 以外なら true）
    /// - string ⇄ 数値・bool・各種数学型（パース・フォーマット）
    /// - Vector2 ⇄ Vector2Int
    /// - Color ⇄ string（16進数）
    /// - Rect/Circle ⇄ string
    /// - Angle ⇄ float（ラジアン）⇄ string（度）
    /// - Guid/GameObjectRef ⇄ string
    /// - EnumValue ⇄ int32_t
    /// - ArrayValue ⇄ string（JSON 配列風）
    ///
    /// @tparam T 変換先の型。
    /// @return 成功時は変換後の PropertyValue、失敗時は Error。
    template <typename T>
    [[nodiscard]] sprout::Result<PropertyValue> convertTo() const noexcept;

    /// 指定した型に変換を試み、結果を out に書き込む。
    /// @tparam T 変換先の型。
    /// @param out 変換結果の書き込み先。
    /// @return 成功時は true、失敗時は false。
    template <typename T>
    [[nodiscard]] bool tryConvertTo(PropertyValue& out) const noexcept;

    /// 等価比較。同一型かつ同一値なら true。
    /// @param other 比較対象。
    /// @return 等しければ true。
    [[nodiscard]] constexpr bool operator==(const PropertyValue& other) const noexcept
    {
        return m_value == other.m_value;
    }

    /// 非等価比較。
    /// @param other 比較対象。
    /// @return 等しくなければ true。
    [[nodiscard]] constexpr bool operator!=(const PropertyValue& other) const noexcept
    {
        return !(*this == other);
    }

    /// 内部バリアントへの const 参照を返す（上級者向け）。
    [[nodiscard]] constexpr const VariantType& variant() const noexcept
    {
        return m_value;
    }

    /// 内部バリアントへの参照を返す（上級者向け）。
    [[nodiscard]] constexpr VariantType& variant() noexcept
    {
        return m_value;
    }

private:
    VariantType m_value;
};

/// @brief PropertyValue を指定した型として扱うヘルパー。
/// @tparam T キャスト先の型。
/// @param value 対象の PropertyValue。
/// @return 型が一致すれば値への参照、不一致なら nullptr。
template <typename T>
[[nodiscard]] const T* propertyValueCast(const PropertyValue* value) noexcept
{
    if (!value) {
        return nullptr;
    }
    return std::get_if<T>(&value->variant());
}

/// @brief PropertyValue を指定した型として扱うヘルパー（非 const 版）。
template <typename T>
[[nodiscard]] T* propertyValueCast(PropertyValue* value) noexcept
{
    if (!value) {
        return nullptr;
    }
    return std::get_if<T>(&value->variant());
}

}  // namespace sprout::reflect

// 実装部

#include <charconv>
#include <sstream>
#include <iomanip>

namespace sprout::reflect {

namespace detail {

template <typename T>
constexpr bool isIntegralType() noexcept
{
    return std::is_same_v<T, std::int32_t>;
}

template <typename T>
constexpr bool isFloatType() noexcept
{
    return std::is_same_v<T, float>;
}

template <typename T>
constexpr bool isStringType() noexcept
{
    return std::is_same_v<T, std::string>;
}

template <typename T>
constexpr bool isBoolType() noexcept
{
    return std::is_same_v<T, bool>;
}

template <typename T>
constexpr bool isVector2Type() noexcept
{
    return std::is_same_v<T, sprout::Vector2>;
}

template <typename T>
constexpr bool isVector2IntType() noexcept
{
    return std::is_same_v<T, sprout::Vector2Int>;
}

template <typename T>
constexpr bool isColorType() noexcept
{
    return std::is_same_v<T, sprout::Color>;
}

template <typename T>
constexpr bool isRectType() noexcept
{
    return std::is_same_v<T, sprout::Rect>;
}

template <typename T>
constexpr bool isCircleType() noexcept
{
    return std::is_same_v<T, sprout::Circle>;
}

template <typename T>
constexpr bool isAngleType() noexcept
{
    return std::is_same_v<T, sprout::Angle>;
}

template <typename T>
constexpr bool isGuidType() noexcept
{
    return std::is_same_v<T, Guid>;
}

template <typename T>
constexpr bool isGameObjectRefType() noexcept
{
    return std::is_same_v<T, GameObjectRef>;
}

template <typename T>
constexpr bool isEnumType() noexcept
{
    return std::is_same_v<T, EnumValue>;
}

template <typename T>
constexpr bool isArrayType() noexcept
{
    return std::is_same_v<T, ArrayValue>;
}

template <typename From, typename To>
struct NumericConverter;

template <>
struct NumericConverter<std::int32_t, float> {
    static constexpr float convert(std::int32_t value) noexcept { return static_cast<float>(value); }
};

template <>
struct NumericConverter<float, std::int32_t> {
    static constexpr std::int32_t convert(float value) noexcept { return static_cast<std::int32_t>(value); }
};

template <>
struct NumericConverter<bool, std::int32_t> {
    static constexpr std::int32_t convert(bool value) noexcept { return value ? 1 : 0; }
};

template <>
struct NumericConverter<bool, float> {
    static constexpr float convert(bool value) noexcept { return value ? 1.0F : 0.0F; }
};

template <>
struct NumericConverter<std::int32_t, bool> {
    static constexpr bool convert(std::int32_t value) noexcept { return value != 0; }
};

template <>
struct NumericConverter<float, bool> {
    static constexpr bool convert(float value) noexcept { return value != 0.0F; }
};

template <typename T>
concept ArithmeticType = std::is_arithmetic_v<T>;

template <typename From, typename To>
    requires (isIntegralType<From>() && isFloatType<To>()) ||
             (isFloatType<From>() && isIntegralType<To>()) ||
             (isBoolType<From>() && (isIntegralType<To>() || isFloatType<To>())) ||
             ((isIntegralType<From>() || isFloatType<From>()) && isBoolType<To>())
[[nodiscard]] sprout::Result<PropertyValue> convertNumeric(const PropertyValue& from) noexcept
{
    const From* fromPtr = std::get_if<From>(&from.variant());
    if (!fromPtr) {
        return sprout::Error{sprout::ErrorCode::InvalidArgument, "型が一致しません"};
    }
    To converted = NumericConverter<From, To>::convert(*fromPtr);
    return PropertyValue{converted};
}

template <typename T>
[[nodiscard]] sprout::Result<std::string> toString(const T& value) noexcept;

template <>
[[nodiscard]] sprout::Result<std::string> toString(const std::int32_t& value) noexcept
{
    return std::to_string(value);
}

template <>
[[nodiscard]] sprout::Result<std::string> toString(const float& value) noexcept
{
    std::ostringstream oss;
    oss << std::setprecision(9) << value;
    return oss.str();
}

template <>
[[nodiscard]] sprout::Result<std::string> toString(const bool& value) noexcept
{
    return value ? std::string{"true"} : std::string{"false"};
}

template <>
[[nodiscard]] sprout::Result<std::string> toString(const sprout::Vector2& value) noexcept
{
    std::ostringstream oss;
    oss << std::setprecision(9) << "{" << value.x << ", " << value.y << "}";
    return oss.str();
}

template <>
[[nodiscard]] sprout::Result<std::string> toString(const sprout::Vector2Int& value) noexcept
{
    return std::string{"{"} + std::to_string(value.x) + ", " + std::to_string(value.y) + "}";
}

template <>
[[nodiscard]] sprout::Result<std::string> toString(const sprout::Color& value) noexcept
{
    std::ostringstream oss;
    oss << std::hex << std::uppercase << std::setfill('0')
        << std::setw(2) << static_cast<int>(value.r)
        << std::setw(2) << static_cast<int>(value.g)
        << std::setw(2) << static_cast<int>(value.b)
        << std::setw(2) << static_cast<int>(value.a);
    return oss.str();
}

template <>
[[nodiscard]] sprout::Result<std::string> toString(const sprout::Rect& value) noexcept
{
    auto posStr = toString(value.position);
    auto sizeStr = toString(value.size);
    if (!posStr.isOk() || !sizeStr.isOk()) {
        return sprout::Error{sprout::ErrorCode::Parse, "Rect の文字列化に失敗"};
    }
    return "{" + posStr.value() + ", " + sizeStr.value() + "}";
}

template <>
[[nodiscard]] sprout::Result<std::string> toString(const sprout::Circle& value) noexcept
{
    auto centerStr = toString(value.center);
    if (!centerStr.isOk()) {
        return sprout::Error{sprout::ErrorCode::Parse, "Circle の文字列化に失敗"};
    }
    auto radiusStr = toString(value.radius);
    if (!radiusStr.isOk()) {
        return sprout::Error{sprout::ErrorCode::Parse, "Circle の文字列化に失敗"};
    }
    return "{" + centerStr.value() + ", " + radiusStr.value() + "}";
}

template <>
[[nodiscard]] sprout::Result<std::string> toString(const sprout::Angle& value) noexcept
{
    return toString(value.degrees());
}

template <>
[[nodiscard]] sprout::Result<std::string> toString(const Guid& value) noexcept
{
    std::ostringstream oss;
    oss << std::hex << std::uppercase << std::setfill('0') << std::setw(16) << value;
    return oss.str();
}

template <>
[[nodiscard]] sprout::Result<std::string> toString(const GameObjectRef& value) noexcept
{
    return std::to_string(value);
}

template <>
[[nodiscard]] sprout::Result<std::string> toString(const EnumValue& value) noexcept
{
    return std::to_string(value);
}

template <>
[[nodiscard]] sprout::Result<std::string> toString(const ArrayValue& value) noexcept
{
    std::string result = "[";
    for (std::size_t i = 0; i < value.size(); ++i) {
        auto elemStr = value[i].convertTo<std::string>();
        if (!elemStr.isOk()) {
            return sprout::Error{sprout::ErrorCode::Parse, "配列要素の文字列化に失敗"};
        }
        if (i > 0) {
            result += ", ";
        }
        result += elemStr.value();
    }
    result += "]";
    return result;
}

template <typename T>
[[nodiscard]] sprout::Result<T> fromString(const std::string& str) noexcept;

template <>
[[nodiscard]] sprout::Result<std::int32_t> fromString(const std::string& str) noexcept
{
    std::int32_t value = 0;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
    if (ec == std::errc{} && ptr == str.data() + str.size()) {
        return value;
    }
    return sprout::Error{sprout::ErrorCode::Parse, "int32_t へのパースに失敗: " + str};
}

template <>
[[nodiscard]] sprout::Result<float> fromString(const std::string& str) noexcept
{
    float value = 0.0F;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
    if (ec == std::errc{} && ptr == str.data() + str.size()) {
        return value;
    }
    return sprout::Error{sprout::ErrorCode::Parse, "float へのパースに失敗: " + str};
}

template <>
[[nodiscard]] sprout::Result<bool> fromString(const std::string& str) noexcept
{
    if (str == "true" || str == "1" || str == "yes" || str == "on") {
        return true;
    }
    if (str == "false" || str == "0" || str == "no" || str == "off") {
        return false;
    }
    return sprout::Error{sprout::ErrorCode::Parse, "bool へのパースに失敗: " + str};
}

template <>
[[nodiscard]] sprout::Result<sprout::Vector2> fromString(const std::string& str) noexcept
{
    if (str.size() < 5 || str.front() != '{' || str.back() != '}') {
        return sprout::Error{sprout::ErrorCode::Parse, "Vector2 形式ではありません: " + str};
    }
    std::string inner = str.substr(1, str.size() - 2);
    auto commaPos = inner.find(',');
    if (commaPos == std::string::npos) {
        return sprout::Error{sprout::ErrorCode::Parse, "Vector2 のカンマが見つかりません: " + str};
    }
    std::string xStr = inner.substr(0, commaPos);
    std::string yStr = inner.substr(commaPos + 1);
    auto xResult = fromString<float>(xStr);
    auto yResult = fromString<float>(yStr);
    if (!xResult.isOk() || !yResult.isOk()) {
        return sprout::Error{sprout::ErrorCode::Parse, "Vector2 成分のパースに失敗: " + str};
    }
    return sprout::Vector2{xResult.value(), yResult.value()};
}

template <>
[[nodiscard]] sprout::Result<sprout::Vector2Int> fromString(const std::string& str) noexcept
{
    if (str.size() < 5 || str.front() != '{' || str.back() != '}') {
        return sprout::Error{sprout::ErrorCode::Parse, "Vector2Int 形式ではありません: " + str};
    }
    std::string inner = str.substr(1, str.size() - 2);
    auto commaPos = inner.find(',');
    if (commaPos == std::string::npos) {
        return sprout::Error{sprout::ErrorCode::Parse, "Vector2Int のカンマが見つかりません: " + str};
    }
    std::string xStr = inner.substr(0, commaPos);
    std::string yStr = inner.substr(commaPos + 1);
    auto xResult = fromString<std::int32_t>(xStr);
    auto yResult = fromString<std::int32_t>(yStr);
    if (!xResult.isOk() || !yResult.isOk()) {
        return sprout::Error{sprout::ErrorCode::Parse, "Vector2Int 成分のパースに失敗: " + str};
    }
    return sprout::Vector2Int{xResult.value(), yResult.value()};
}

template <>
[[nodiscard]] sprout::Result<sprout::Color> fromString(const std::string& str) noexcept
{
    if (str.size() != 8) {
        return sprout::Error{sprout::ErrorCode::Parse, "Color 16進数形式ではありません（8文字必要）: " + str};
    }
    std::uint32_t rgba = 0;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), rgba, 16);
    if (ec != std::errc{} || ptr != str.data() + str.size()) {
        return sprout::Error{sprout::ErrorCode::Parse, "Color 16進数のパースに失敗: " + str};
    }
    return sprout::Color{rgba};
}

template <>
[[nodiscard]] sprout::Result<sprout::Rect> fromString(const std::string& str) noexcept
{
    if (str.size() < 5 || str.front() != '{' || str.back() != '}') {
        return sprout::Error{sprout::ErrorCode::Parse, "Rect 形式ではありません: " + str};
    }
    std::string inner = str.substr(1, str.size() - 2);
    auto commaPos = inner.find(',');
    if (commaPos == std::string::npos) {
        return sprout::Error{sprout::ErrorCode::Parse, "Rect のカンマが見つかりません: " + str};
    }
    std::string posStr = inner.substr(0, commaPos);
    std::string sizeStr = inner.substr(commaPos + 1);
    auto posResult = fromString<sprout::Vector2>(posStr);
    auto sizeResult = fromString<sprout::Vector2>(sizeStr);
    if (!posResult.isOk() || !sizeResult.isOk()) {
        return sprout::Error{sprout::ErrorCode::Parse, "Rect 成分のパースに失敗: " + str};
    }
    return sprout::Rect{posResult.value(), sizeResult.value()};
}

template <>
[[nodiscard]] sprout::Result<sprout::Circle> fromString(const std::string& str) noexcept
{
    if (str.size() < 5 || str.front() != '{' || str.back() != '}') {
        return sprout::Error{sprout::ErrorCode::Parse, "Circle 形式ではありません: " + str};
    }
    std::string inner = str.substr(1, str.size() - 2);
    auto commaPos = inner.find(',');
    if (commaPos == std::string::npos) {
        return sprout::Error{sprout::ErrorCode::Parse, "Circle のカンマが見つかりません: " + str};
    }
    std::string centerStr = inner.substr(0, commaPos);
    std::string radiusStr = inner.substr(commaPos + 1);
    auto centerResult = fromString<sprout::Vector2>(centerStr);
    auto radiusResult = fromString<float>(radiusStr);
    if (!centerResult.isOk() || !radiusResult.isOk()) {
        return sprout::Error{sprout::ErrorCode::Parse, "Circle 成分のパースに失敗: " + str};
    }
    return sprout::Circle{centerResult.value(), radiusResult.value()};
}

template <>
[[nodiscard]] sprout::Result<sprout::Angle> fromString(const std::string& str) noexcept
{
    auto degreesResult = fromString<float>(str);
    if (!degreesResult.isOk()) {
        return sprout::Error{sprout::ErrorCode::Parse, "Angle (度) のパースに失敗: " + str};
    }
    return sprout::Angle::fromDegrees(degreesResult.value());
}

template <>
[[nodiscard]] sprout::Result<Guid> fromString(const std::string& str) noexcept
{
    if (str.size() != 16) {
        return sprout::Error{sprout::ErrorCode::Parse, "Guid 16進数形式ではありません（16文字必要）: " + str};
    }
    Guid value = 0;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value, 16);
    if (ec != std::errc{} || ptr != str.data() + str.size()) {
        return sprout::Error{sprout::ErrorCode::Parse, "Guid 16進数のパースに失敗: " + str};
    }
    return value;
}

template <>
[[nodiscard]] sprout::Result<GameObjectRef> fromString(const std::string& str) noexcept
{
    auto result = fromString<std::uint64_t>(str);
    if (!result.isOk()) {
        return sprout::Error{sprout::ErrorCode::Parse, "GameObjectRef のパースに失敗: " + str};
    }
    return result.value();
}

template <>
[[nodiscard]] sprout::Result<EnumValue> fromString(const std::string& str) noexcept
{
    return fromString<std::int32_t>(str);
}

template <>
[[nodiscard]] sprout::Result<ArrayValue> fromString(const std::string& str) noexcept
{
    if (str.size() < 2 || str.front() != '[' || str.back() != ']') {
        return sprout::Error{sprout::ErrorCode::Parse, "配列形式ではありません: " + str};
    }
    std::string inner = str.substr(1, str.size() - 2);
    if (inner.empty()) {
        return ArrayValue{};
    }
    ArrayValue result;
    std::size_t start = 0;
    int depth = 0;
    for (std::size_t i = 0; i <= inner.size(); ++i) {
        if (i == inner.size() || (inner[i] == ',' && depth == 0)) {
            std::string elemStr = inner.substr(start, i - start);
            // Trim whitespace
            auto trim = [](std::string& s) {
                std::size_t first = s.find_first_not_of(" \t\n\r");
                if (first == std::string::npos) { s.clear(); return; }
                std::size_t last = s.find_last_not_of(" \t\n\r");
                s = s.substr(first, last - first + 1);
            };
            trim(elemStr);
            PropertyValue elem;
            // Try to parse as int first, then float, then bool, then string
            auto intResult = elem.convertTo<std::int32_t>();
            if (intResult.isOk()) {
                // We need to parse from string, not convert empty
                auto parseResult = detail::fromString<std::int32_t>(elemStr);
                if (parseResult.isOk()) {
                    result.emplace_back(parseResult.value());
                } else {
                    return sprout::Error{sprout::ErrorCode::Parse, "配列要素のパースに失敗: " + elemStr};
                }
            } else {
                // For simplicity, just try int parsing
                auto parseResult = detail::fromString<std::int32_t>(elemStr);
                if (parseResult.isOk()) {
                    result.emplace_back(parseResult.value());
                } else {
                    return sprout::Error{sprout::ErrorCode::Parse, "配列要素のパースに失敗: " + elemStr};
                }
            }
            start = i + 1;
        } else if (inner[i] == '{' || inner[i] == '[') {
            ++depth;
        } else if (inner[i] == '}' || inner[i] == ']') {
            --depth;
        }
    }
    return result;
}

}  // namespace detail

template <typename T>
[[nodiscard]] sprout::Result<PropertyValue> PropertyValue::convertTo() const noexcept
{
    if (is<T>()) {
        return *this;
    }

    if (detail::isStringType<T>()) {
        if (is<std::string>()) {
            return *this;
        }
        if (is<std::int32_t>()) {
            auto strResult = detail::toString(get<std::int32_t>());
            if (strResult.isOk()) return PropertyValue{std::move(strResult.value())};
            return strResult.error();
        }
        if (is<float>()) {
            auto strResult = detail::toString(get<float>());
            if (strResult.isOk()) return PropertyValue{std::move(strResult.value())};
            return strResult.error();
        }
        if (is<bool>()) {
            auto strResult = detail::toString(get<bool>());
            if (strResult.isOk()) return PropertyValue{std::move(strResult.value())};
            return strResult.error();
        }
        if (is<sprout::Vector2>()) {
            auto strResult = detail::toString(get<sprout::Vector2>());
            if (strResult.isOk()) return PropertyValue{std::move(strResult.value())};
            return strResult.error();
        }
        if (is<sprout::Vector2Int>()) {
            auto strResult = detail::toString(get<sprout::Vector2Int>());
            if (strResult.isOk()) return PropertyValue{std::move(strResult.value())};
            return strResult.error();
        }
        if (is<sprout::Color>()) {
            auto strResult = detail::toString(get<sprout::Color>());
            if (strResult.isOk()) return PropertyValue{std::move(strResult.value())};
            return strResult.error();
        }
        if (is<sprout::Rect>()) {
            auto strResult = detail::toString(get<sprout::Rect>());
            if (strResult.isOk()) return PropertyValue{std::move(strResult.value())};
            return strResult.error();
        }
        if (is<sprout::Circle>()) {
            auto strResult = detail::toString(get<sprout::Circle>());
            if (strResult.isOk()) return PropertyValue{std::move(strResult.value())};
            return strResult.error();
        }
        if (is<sprout::Angle>()) {
            auto strResult = detail::toString(get<sprout::Angle>());
            if (strResult.isOk()) return PropertyValue{std::move(strResult.value())};
            return strResult.error();
        }
        if (is<Guid>()) {
            auto strResult = detail::toString(get<Guid>());
            if (strResult.isOk()) return PropertyValue{std::move(strResult.value())};
            return strResult.error();
        }
        if (is<GameObjectRef>()) {
            auto strResult = detail::toString(get<GameObjectRef>());
            if (strResult.isOk()) return PropertyValue{std::move(strResult.value())};
            return strResult.error();
        }
        if (is<EnumValue>()) {
            auto strResult = detail::toString(get<EnumValue>());
            if (strResult.isOk()) return PropertyValue{std::move(strResult.value())};
            return strResult.error();
        }
        if (is<ArrayValue>()) {
            auto strResult = detail::toString(get<ArrayValue>());
            if (strResult.isOk()) return PropertyValue{std::move(strResult.value())};
            return strResult.error();
        }
        return sprout::Error{sprout::ErrorCode::NotSupported, "文字列への変換がサポートされていません"};
    }

    if (detail::isIntegralType<T>()) {
        if (is<float>()) {
            auto result = detail::convertNumeric<std::int32_t, float>(*this);
            if (result.isOk()) return result.value();
            return result.error();
        }
        if (is<bool>()) {
            auto result = detail::convertNumeric<bool, std::int32_t>(*this);
            if (result.isOk()) return result.value();
            return result.error();
        }
        if (is<std::string>()) {
            auto result = detail::fromString<std::int32_t>(get<std::string>());
            if (result.isOk()) return PropertyValue{result.value()};
            return result.error();
        }
        if (is<EnumValue>()) return PropertyValue{get<EnumValue>()};
        if (is<GameObjectRef>()) return PropertyValue{static_cast<std::int32_t>(get<GameObjectRef>())};
        return sprout::Error{sprout::ErrorCode::NotSupported, "int32_t への変換がサポートされていません"};
    }

    if (detail::isFloatType<T>()) {
        if (is<std::int32_t>()) {
            auto result = detail::convertNumeric<std::int32_t, float>(*this);
            if (result.isOk()) return result.value();
            return result.error();
        }
        if (is<bool>()) {
            auto result = detail::convertNumeric<bool, float>(*this);
            if (result.isOk()) return result.value();
            return result.error();
        }
        if (is<std::string>()) {
            auto result = detail::fromString<float>(get<std::string>());
            if (result.isOk()) return PropertyValue{result.value()};
            return result.error();
        }
        if (is<sprout::Angle>()) return PropertyValue{get<sprout::Angle>().radians()};
        return sprout::Error{sprout::ErrorCode::NotSupported, "float への変換がサポートされていません"};
    }

    if (detail::isBoolType<T>()) {
        if (is<std::int32_t>()) {
            auto result = detail::convertNumeric<std::int32_t, bool>(*this);
            if (result.isOk()) return result.value();
            return result.error();
        }
        if (is<float>()) {
            auto result = detail::convertNumeric<float, bool>(*this);
            if (result.isOk()) return result.value();
            return result.error();
        }
        if (is<std::string>()) {
            auto result = detail::fromString<bool>(get<std::string>());
            if (result.isOk()) return PropertyValue{result.value()};
            return result.error();
        }
        return sprout::Error{sprout::ErrorCode::NotSupported, "bool への変換がサポートされていません"};
    }

    if (detail::isVector2Type<T>()) {
        if (is<sprout::Vector2Int>()) {
            const auto& v = get<sprout::Vector2Int>();
            return PropertyValue{sprout::Vector2{static_cast<float>(v.x), static_cast<float>(v.y)}};
        }
        if (is<std::string>()) return detail::fromString<sprout::Vector2>(get<std::string>()).transform([](auto&& v) { return PropertyValue{v}; });
        return sprout::Error{sprout::ErrorCode::NotSupported, "Vector2 への変換がサポートされていません"};
    }

    if (detail::isVector2IntType<T>()) {
        if (is<sprout::Vector2>()) {
            const auto& v = get<sprout::Vector2>();
            return PropertyValue{sprout::Vector2Int{static_cast<std::int32_t>(v.x), static_cast<std::int32_t>(v.y)}};
        }
        if (is<std::string>()) return detail::fromString<sprout::Vector2Int>(get<std::string>()).transform([](auto&& v) { return PropertyValue{v}; });
        return sprout::Error{sprout::ErrorCode::NotSupported, "Vector2Int への変換がサポートされていません"};
    }

    if (detail::isColorType<T>()) {
        if (is<std::string>()) return detail::fromString<sprout::Color>(get<std::string>()).transform([](auto&& v) { return PropertyValue{v}; });
        return sprout::Error{sprout::ErrorCode::NotSupported, "Color への変換がサポートされていません"};
    }

    if (detail::isRectType<T>()) {
        if (is<std::string>()) return detail::fromString<sprout::Rect>(get<std::string>()).transform([](auto&& v) { return PropertyValue{v}; });
        return sprout::Error{sprout::ErrorCode::NotSupported, "Rect への変換がサポートされていません"};
    }

    if (detail::isCircleType<T>()) {
        if (is<std::string>()) return detail::fromString<sprout::Circle>(get<std::string>()).transform([](auto&& v) { return PropertyValue{v}; });
        return sprout::Error{sprout::ErrorCode::NotSupported, "Circle への変換がサポートされていません"};
    }

    if (detail::isAngleType<T>()) {
        if (is<float>()) return PropertyValue{sprout::Angle::fromRadians(get<float>())};
        if (is<std::string>()) return detail::fromString<sprout::Angle>(get<std::string>()).transform([](auto&& v) { return PropertyValue{v}; });
        return sprout::Error{sprout::ErrorCode::NotSupported, "Angle への変換がサポートされていません"};
    }

    if (detail::isGuidType<T>()) {
        if (is<std::string>()) return detail::fromString<Guid>(get<std::string>()).transform([](auto&& v) { return PropertyValue{v}; });
        return sprout::Error{sprout::ErrorCode::NotSupported, "Guid への変換がサポートされていません"};
    }

    if (detail::isGameObjectRefType<T>()) {
        if (is<std::string>()) return detail::fromString<GameObjectRef>(get<std::string>()).transform([](auto&& v) { return PropertyValue{v}; });
        if (is<std::int32_t>()) return PropertyValue{static_cast<GameObjectRef>(get<std::int32_t>())};
        return sprout::Error{sprout::ErrorCode::NotSupported, "GameObjectRef への変換がサポートされていません"};
    }

    if (detail::isEnumType<T>()) {
        if (is<std::int32_t>()) return PropertyValue{static_cast<EnumValue>(get<std::int32_t>())};
        if (is<std::string>()) return detail::fromString<EnumValue>(get<std::string>()).transform([](auto&& v) { return PropertyValue{v}; });
        return sprout::Error{sprout::ErrorCode::NotSupported, "EnumValue への変換がサポートされていません"};
    }

    if (detail::isArrayType<T>()) {
        if (is<std::string>()) return detail::fromString<ArrayValue>(get<std::string>()).transform([](auto&& v) { return PropertyValue{v}; });
        return sprout::Error{sprout::ErrorCode::NotSupported, "ArrayValue への変換がサポートされていません"};
    }

    return sprout::Error{sprout::ErrorCode::NotSupported, "未対応の型変換"};
}

template <typename T>
[[nodiscard]] bool PropertyValue::tryConvertTo(PropertyValue& out) const noexcept
{
    auto result = convertTo<T>();
    if (result.isOk()) {
        out = std::move(result.value());
        return true;
    }
    return false;
}

}  // namespace sprout::reflect