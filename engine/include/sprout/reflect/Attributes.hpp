#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace sprout::reflect {

/// @brief プロパティ属性の基底クラス。
class Attribute {
public:
    virtual ~Attribute() = default;
};

/// @brief 数値プロパティの範囲制限属性。
///
/// インスペクタでスライダーや入力範囲制限として使用される。
/// 整数型・浮動小数点型の両方に対応。
///
/// @code
/// struct MyBehavior : Behavior {
///     SPROUT_PROPERTY(int, health, 100, Range{0, 100});
///     SPROUT_PROPERTY(float, speed, 5.0f, Range{0.0f, 20.0f});
/// };
/// @endcode
struct RangeAttribute final : Attribute {
    /// 最小値（int32_t として格納、float の場合も変換して使用）。
    std::int64_t min;
    /// 最大値（int32_t として格納、float の場合も変換して使用）。
    std::int64_t max;

    /// コンストラクタ（整数範囲）。
    /// @param minValue 最小値。
    /// @param maxValue 最大値。
    constexpr RangeAttribute(std::int64_t minValue, std::int64_t maxValue) noexcept
        : min{minValue}, max{maxValue}
    {
    }

    /// コンストラクタ（浮動小数点範囲）。
    /// @param minValue 最小値。
    /// @param maxValue 最大値。
    constexpr RangeAttribute(float minValue, float maxValue) noexcept
        : min{static_cast<std::int64_t>(minValue * 1000000)},
          max{static_cast<std::int64_t>(maxValue * 1000000)}
    {
    }

    /// 浮動小数点として最小値を取得。
    /// @return 最小値（float）。
    [[nodiscard]] constexpr float minFloat() const noexcept
    {
        return static_cast<float>(min) / 1000000.0f;
    }

    /// 浮動小数点として最大値を取得。
    /// @return 最大値（float）。
    [[nodiscard]] constexpr float maxFloat() const noexcept
    {
        return static_cast<float>(max) / 1000000.0f;
    }

    /// 整数として最小値を取得。
    /// @return 最小値（int64_t）。
    [[nodiscard]] constexpr std::int64_t minInt() const noexcept
    {
        return min;
    }

    /// 整数として最大値を取得。
    /// @return 最大値（int64_t）。
    [[nodiscard]] constexpr std::int64_t maxInt() const noexcept
    {
        return max;
    }

    /// 等価比較。
    [[nodiscard]] constexpr bool operator==(const RangeAttribute& other) const noexcept = default;
};

/// @brief ツールチップ（ホバー説明）属性。
///
/// インスペクタでプロパティにマウスを合わせたときに表示される説明文。
///
/// @code
/// struct MyBehavior : Behavior {
///     SPROUT_PROPERTY(int, health, 100, Tooltip{"プレイヤーの体力。0 でゲームオーバー。"});
/// };
/// @endcode
struct TooltipAttribute final : Attribute {
    /// 表示する説明文。
    std::string text;

    /// コンストラクタ。
    /// @param tooltipText 説明文。
    explicit TooltipAttribute(std::string_view tooltipText)
        : text{tooltipText}
    {
    }

    /// 等価比較。
    [[nodiscard]] constexpr bool operator==(const TooltipAttribute& other) const noexcept = default;
};

/// @brief インスペクタでの見出し（グループ区切り）属性。
///
/// この属性が付いたプロパティの前に太字の見出しを表示し、プロパティをグループ化する。
///
/// @code
/// struct MyBehavior : Behavior {
///     SPROUT_PROPERTY(int, health, 100, Header{"ステータス"});
///     SPROUT_PROPERTY(int, mana, 50);
///     SPROUT_PROPERTY(float, speed, 5.0f, Header{"移動"});
/// };
/// @endcode
struct HeaderAttribute final : Attribute {
    /// 見出しテキスト。
    std::string text;

    /// コンストラクタ。
    /// @param headerText 見出しテキスト。
    explicit HeaderAttribute(std::string_view headerText)
        : text{headerText}
    {
    }

    /// 等価比較。
    [[nodiscard]] constexpr bool operator==(const HeaderAttribute& other) const noexcept = default;
};

/// @brief インスペクタで非表示にする属性。
///
/// この属性が付いたプロパティはインスペクタに表示されないが、
/// シリアライズ・デシリアライズの対象にはなる。
///
/// @code
/// struct MyBehavior : Behavior {
///     SPROUT_PROPERTY(int, internalId, 0, Hidden{}); // 表示されないが保存される
/// };
/// @endcode
struct HiddenAttribute final : Attribute {
    /// デフォルトコンストラクタ。
    constexpr HiddenAttribute() noexcept = default;

    /// 等価比較。
    [[nodiscard]] constexpr bool operator==(const HiddenAttribute&) const noexcept = default;
};

/// @brief インスペクタで読み取り専用にする属性。
///
/// この属性が付いたプロパティはインスペクタで編集不可（グレーアウト表示）になる。
/// 実行時の値変更は可能。
///
/// @code
/// struct MyBehavior : Behavior {
///     SPROUT_PROPERTY(int, instanceId, 0, ReadOnly{}); // 実行時に割り当てられる ID
/// };
/// @endcode
struct ReadOnlyAttribute final : Attribute {
    /// デフォルトコンストラクタ。
    constexpr ReadOnlyAttribute() noexcept = default;

    /// 等価比較。
    [[nodiscard]] constexpr bool operator==(const ReadOnlyAttribute&) const noexcept = default;
};

/// @brief 文字列プロパティを複数行入力にする属性。
///
/// この属性が付いた std::string プロパティは、インスペクタで複数行テキストエリアとして表示される。
///
/// @code
/// struct MyBehavior : Behavior {
///     SPROUT_PROPERTY(std::string, description, "", Multiline{});
/// };
/// @endcode
struct MultilineAttribute final : Attribute {
    /// デフォルトコンストラクタ。
    constexpr MultilineAttribute() noexcept = default;

    /// 等価比較。
    [[nodiscard]] constexpr bool operator==(const MultilineAttribute&) const noexcept = default;
};

/// @brief 属性の型安全な保持用バリアント。
using AttributeVariant = std::variant<
    RangeAttribute,
    TooltipAttribute,
    HeaderAttribute,
    HiddenAttribute,
    ReadOnlyAttribute,
    MultilineAttribute
>;

/// @brief 複数の属性を保持するコンテナ。
///
/// プロパティ登録時に複数の属性をまとめて指定するために使用する。
///
/// @code
/// SPROUT_PROPERTY(float, speed, 5.0f, Attributes{Range{0.0f, 20.0f}, Tooltip{"移動速度"}});
/// @endcode
struct Attributes {
    /// 保持する属性のリスト。
    std::vector<AttributeVariant> values;

    /// デフォルトコンストラクタ（空の属性リスト）。
    Attributes() noexcept = default;

    /// 単一属性から構築。
    template <typename T>
        requires std::is_base_of_v<Attribute, T>
    Attributes(T&& attr) noexcept
        : values{std::forward<T>(attr)}
    {
    }

    /// 複数属性から構築。
    template <typename... Ts>
        requires (std::is_base_of_v<Attribute, Ts> && ...)
    Attributes(Ts&&... attrs) noexcept
        : values{std::forward<Ts>(attrs)...}
    {
    }

    /// 属性を追加。
    template <typename T>
        requires std::is_base_of_v<Attribute, T>
    void add(T&& attr)
    {
        values.emplace_back(std::forward<T>(attr));
    }

    /// 指定した型の属性を持つか判定。
    /// @tparam T 判定する属性型。
    /// @return 持っていれば true。
    template <typename T>
    [[nodiscard]] bool has() const noexcept
    {
        for (const auto& v : values) {
            if (std::holds_alternative<T>(v)) {
                return true;
            }
        }
        return false;
    }

    /// 指定した型の属性を取得（最初のもの）。
    /// @tparam T 取得する属性型。
    /// @return 見つかった場合はポインタ、なければ nullptr。
    template <typename T>
    [[nodiscard]] const T* get() const noexcept
    {
        for (const auto& v : values) {
            if (auto* ptr = std::get_if<T>(&v)) {
                return ptr;
            }
        }
        return nullptr;
    }

    /// 指定した型の属性をすべて取得。
    /// @tparam T 取得する属性型。
    /// @return 見つかった属性のリスト。
    template <typename T>
    [[nodiscard]] std::vector<const T*> getAll() const noexcept
    {
        std::vector<const T*> result;
        for (const auto& v : values) {
            if (auto* ptr = std::get_if<T>(&v)) {
                result.push_back(ptr);
            }
        }
        return result;
    }

    /// 等価比較。
    [[nodiscard]] bool operator==(const Attributes& other) const noexcept
    {
        if (values.size() != other.values.size()) {
            return false;
        }
        for (std::size_t i = 0; i < values.size(); ++i) {
            if (values[i] != other.values[i]) {
                return false;
            }
        }
        return true;
    }
};

}  // namespace sprout::reflect