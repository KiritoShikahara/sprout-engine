#pragma once

#include "sprout/math/Vector2.hpp"

#include <optional>
#include <string>
#include <vector>

namespace sprout::detail {

/// @brief プロセスの DPI 対応レベル。
enum class DpiAwareness {
    /// @brief DPI を考慮しない。OS が描画を拡大する。
    Unaware,

    /// @brief システム DPI に固定する。
    SystemAware,

    /// @brief モニタごとに DPI を切り替える。
    PerMonitorAware,

    /// @brief PerMonitorAware に加えて非クライアント領域の描画も DPI 対応にする。
    PerMonitorAwareV2,
};

/// @brief 接続されているモニタの情報。
struct MonitorInfo {
    /// @brief 列挙順のインデックス（0 始まり）。
    int index{0};

    /// @brief デバイス名（UTF-8）。
    std::string name{};

    /// @brief 仮想デスクトップ上の原点（物理ピクセル）。
    Vector2Int position{};

    /// @brief 解像度（物理ピクセル）。
    Vector2Int size{};

    /// @brief 拡大率（96 DPI を 1.0 とする）。
    float scaleFactor{1.0F};

    /// @brief 主モニタなら true。
    bool primary{false};
};

/// @brief 画面（仮想デスクトップ）全体の情報。
struct DisplayInfo {
    /// @brief 全モニタを覆う領域の原点（物理ピクセル）。
    Vector2Int origin{};

    /// @brief 全モニタを覆う領域の大きさ（物理ピクセル）。
    Vector2Int size{};

    /// @brief 接続されているモニタ数。
    int monitorCount{0};

    /// @brief プロセスが高 DPI 対応なら true。
    bool highDpiEnabled{false};
};

/// @brief 画面情報の取得元を差し替えるための境界。
class DisplayBackend {
public:
    virtual ~DisplayBackend() = default;

    /// @brief 接続されているモニタを列挙する。
    /// @return モニタのリスト。接続されていない場合は空。
    [[nodiscard]] virtual std::vector<MonitorInfo> monitors() const = 0;

    /// @brief 現在の DPI 対応レベルを返す。
    [[nodiscard]] virtual DpiAwareness awareness() const = 0;

    /// @brief プロセスの DPI 対応レベルを設定する。
    /// @param awareness 設定するレベル。
    /// @return 設定できた（既に同じレベルだった場合を含む）なら true。
    virtual bool setAwareness(DpiAwareness awareness) = 0;
};

/// @brief OS の画面情報を返すバックエンド。
/// @return プロセス内で共有されるバックエンド。
DisplayBackend& systemDisplayBackend();

/// @brief 高 DPI 対応とモニタ情報の取得を担う。
///
/// 画面まわりの情報は OS へ委譲する。バックエンドを差し替えると、OS に
/// 依存しない決定的な結果を得られる。
class Display {
public:
    /// @brief システムのバックエンドを使う。
    Display() noexcept;

    /// @brief バックエンドを差し替えて生成する（テスト用）。
    /// @param backend 使用するバックエンド。呼び出し側が寿命を管理する。
    explicit Display(DisplayBackend& backend) noexcept;

    /// @brief 接続されているモニタを列挙する。
    /// @return モニタのリスト。接続されていない場合は空。
    [[nodiscard]] std::vector<MonitorInfo> monitors() const;

    /// @brief 主モニタを取得する。
    /// @return 主モニタ。存在しない場合は std::nullopt。
    [[nodiscard]] std::optional<MonitorInfo> primaryMonitor() const;

    /// @brief インデックス指定でモニタを取得する。
    /// @param index モニタのインデックス。
    /// @return 該当するモニタ。範囲外の場合は std::nullopt。
    [[nodiscard]] std::optional<MonitorInfo> monitor(int index) const;

    /// @brief 画面全体の情報を取得する。
    /// @return 全モニタを覆う領域とモニタ数。
    [[nodiscard]] DisplayInfo displayInfo() const;

    /// @brief 現在の DPI 対応レベルを取得する。
    [[nodiscard]] DpiAwareness awareness() const;

    /// @brief プロセス全体を Per-Monitor DPI 対応にする。
    /// @return 設定できた（既に同レベルだった場合を含む）なら true。
    bool enableHighDpi() const;

private:
    DisplayBackend* m_backend;
};

/// @brief プロセス全体を Per-Monitor DPI 対応にする。
///
/// ウィンドウを生成する前に呼ぶこと。設定できた（既に同レベルだった場合を
/// 含む）なら true を返す。
[[nodiscard]] bool enableHighDpiSupport();

/// @brief 拡大率 1.0 に対応する基準 DPI。
inline constexpr unsigned int kReferenceDpi = 96;

/// @brief DPI 値から拡大率を求める。
/// @param dpi モニタの有効 DPI。
/// @return dpi / 96。0 の場合は 1.0。
[[nodiscard]] float dpiScaleFromDpi(unsigned int dpi) noexcept;

/// @brief 論理ピクセル（Dip）を物理ピクセルへ変換する。
/// @param logicalPixels 論理ピクセルでの座標。
/// @param scaleFactor 拡大率（1.0 で等倍）。
/// @return 物理ピクセルでの座標。scaleFactor が 0 以下の場合は入力そのまま。
[[nodiscard]] Vector2Int toPhysicalPixels(Vector2Int logicalPixels, float scaleFactor) noexcept;

/// @brief 物理ピクセルを論理ピクセル（Dip）へ変換する。
/// @param physicalPixels 物理ピクセルでの座標。
/// @param scaleFactor 拡大率（1.0 で等倍）。
/// @return 論理ピクセルでの座標。scaleFactor が 0 以下の場合は入力そのまま。
[[nodiscard]] Vector2Int toLogicalPixels(Vector2Int physicalPixels, float scaleFactor) noexcept;

}  // namespace sprout::detail
