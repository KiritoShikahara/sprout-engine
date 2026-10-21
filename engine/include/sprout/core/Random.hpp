#pragma once

#include "sprout/math/Circle.hpp"
#include "sprout/math/Vector2.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <random>
#include <span>

namespace sprout {

/// @brief シード指定可能な擬似乱数生成器。
///
/// 同じシードからは常に同じ列が得られるため、リプレイやテストの再現に使える。
/// スレッドごとに別インスタンスを持つこと（内部状態は共有されない）。
///
/// @code
/// sprout::Random random{42};
/// const int dice = random.nextInt(1, 6);                 // 1〜6（両端を含む）
/// const float speed = random.range(2.0F, 5.0F);          // [2, 5)
/// if (random.chance(0.25F)) { /* 25% で実行 */ }
/// const sprout::Vector2 spawn = random.pointInCircle({{0.0F, 0.0F}, 10.0F});
/// @endcode
class Random {
public:
    /// 非決定的なシードで生成する。
    Random() noexcept;

    /// シードを指定して生成する。
    /// @param seed 乱数列を決める値。同じ値なら同じ列になる。
    explicit Random(std::uint64_t seed) noexcept;

    /// シードを設定し、乱数列を先頭からやり直す。
    /// @param seed 乱数列を決める値。
    void setSeed(std::uint64_t seed) noexcept;

    /// 現在のシードを返す。
    /// @return setSeed() またはコンストラクタに渡した値。
    std::uint64_t seed() const noexcept;

    /// 次の 64 ビット値を返す。
    /// @return 一様乱数。
    std::uint64_t nextUint64() noexcept;

    /// 次の 32 ビット値を返す。
    /// @return 一様乱数。
    std::uint32_t nextUint32() noexcept;

    /// 整数を一様に選ぶ。最小値・最大値の両方を含む。
    /// @param minimum 下限。
    /// @param maximum 上限。minimum より小さい場合は自動的に入れ替える。
    /// @return minimum 以上 maximum 以下の整数。
    int nextInt(int minimum, int maximum) noexcept;

    /// [0, 1) の浮動小数点値を返す。
    /// @return 0 以上 1 未満の一様乱数。
    float nextFloat() noexcept;

    /// [0, maximum) の浮動小数点値を返す。
    /// @param maximum 上限（含まない）。
    /// @return 0 以上 maximum 未満の一様乱数。
    float range(float maximum) noexcept;

    /// [minimum, maximum) の浮動小数点値を返す。
    /// @param minimum 下限（含む）。
    /// @param maximum 上限（含まない）。minimum より小さい場合は自動的に入れ替える。
    /// @return minimum 以上 maximum 未満の一様乱数。
    float range(float minimum, float maximum) noexcept;

    /// 指定した確率で true を返す。
    /// @param probability 確率。0 以下なら常に false、1 以上なら常に true。
    /// @return probability の確率で true。
    bool chance(float probability) noexcept;

    /// 重みに比例した確率で要素のインデックスを選ぶ。
    ///
    /// 重みが 0 以下、または有限でない要素は選ばれない。
    ///
    /// @param weights 各要素の重み。
    /// @return 選ばれたインデックス。有効な重みが無い場合は std::nullopt。
    std::optional<std::size_t> weightedIndex(std::span<const float> weights) noexcept;

    /// 重みに比例した確率で要素を選ぶ。
    /// @param items 選択候補。
    /// @param weights 各要素の重み。items と同じ長さであること。
    /// @return 選ばれた要素。候補が無い、または長さが一致しない場合は std::nullopt。
    template <typename T>
    std::optional<T> chooseWeighted(std::span<const T> items, std::span<const float> weights)
    {
        if (weights.size() != items.size()) {
            return std::nullopt;
        }
        const std::optional<std::size_t> index = weightedIndex(weights);
        if (!index.has_value() || *index >= items.size()) {
            return std::nullopt;
        }
        return items[*index];
    }

    /// 円の内側の点を一様に選ぶ。
    /// @param circle 対象の円。半径が 0 以下なら中心を返す。
    /// @return 円の内側（境界を含む）の点。
    Vector2 pointInCircle(const Circle& circle) noexcept;

private:
    std::mt19937_64 m_engine;
    std::uint64_t m_seed{0};
};

}  // namespace sprout
