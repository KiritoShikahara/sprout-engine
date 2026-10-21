#pragma once

#include "sprout/core/Result.hpp"
#include "sprout/math/Vector2.hpp"

#include <cstdint>
#include <string>
#include <string_view>

namespace sprout::detail {

/// @brief ウィンドウの初期設定。
struct WindowSettings {
    /// タイトルバーに表示する文字列。
    std::string title{"Sprout"};

    /// ウィンドウの大きさ（ピクセル）。
    Vector2Int size{1280, 720};

    /// フルスクリーンで生成するか。
    bool fullscreen{false};

    /// 垂直同期を有効にするか。
    bool verticalSync{true};
};

/// @brief ウィンドウの実装を差し替えるための境界。
///
/// 実機では SFML のウィンドウを使う。テストでは画面を持たない擬似実装に
/// 差し替えて設定の適用内容を検証する。
class WindowBackend {
public:
    virtual ~WindowBackend() = default;

    /// @brief 設定に従ってウィンドウを生成する。
    /// @param settings 適用する設定。
    /// @return 成功すれば空の結果、失敗すれば理由を持つ結果。
    virtual Result<void> open(const WindowSettings& settings) = 0;

    /// @brief ウィンドウを閉じる。
    virtual void close() = 0;

    /// @brief ウィンドウが開いているか判定する。
    /// @return 開いていれば true。
    [[nodiscard]] virtual bool isOpen() const = 0;

    /// @brief タイトルを設定する。
    /// @param title 新しいタイトル。
    virtual void setTitle(std::string_view title) = 0;

    /// @brief タイトルを返す。
    /// @return 現在のタイトル。
    [[nodiscard]] virtual std::string title() const = 0;

    /// @brief 解像度を設定する。
    /// @param size 新しい大きさ（ピクセル）。
    virtual void setSize(Vector2Int size) = 0;

    /// @brief 現在の解像度を返す。
    /// @return ウィンドウの大きさ（ピクセル）。
    [[nodiscard]] virtual Vector2Int size() const = 0;

    /// @brief フルスクリーンを切り替える。
    /// @param enabled true ならフルスクリーン、false ならウィンドウ表示。
    virtual void setFullscreen(bool enabled) = 0;

    /// @brief フルスクリーン中か判定する。
    /// @return フルスクリーンなら true。
    [[nodiscard]] virtual bool isFullscreen() const = 0;

    /// @brief 垂直同期を切り替える。
    /// @param enabled true なら有効、false なら無効。
    virtual void setVerticalSync(bool enabled) = 0;

    /// @brief 垂直同期が有効か判定する。
    /// @return 有効なら true。
    [[nodiscard]] virtual bool isVerticalSyncEnabled() const = 0;

    /// @brief ウィンドウアイコンを設定する。
    /// @param size アイコンの大きさ（ピクセル）。
    /// @param pixels RGBA を並べたピクセルデータ（size.x * size.y * 4 バイト）。
    virtual void setIcon(Vector2Int size, const std::uint8_t* pixels) = 0;
};

/// @brief SFML を使うウィンドウバックエンドを返す。
/// @return プロセス内で共有されるバックエンド。
WindowBackend& systemWindowBackend();

/// @brief ゲームウィンドウのラッパー。
///
/// 生成・解像度変更・フルスクリーン・垂直同期・アイコン設定を扱う。
/// ウィンドウの実体はバックエンドに委譲しており、SFML の型は公開しない。
///
/// @code
/// sprout::detail::Window window;
/// const auto result = window.open(sprout::detail::WindowSettings{.size = {1920, 1080}});
/// window.setVerticalSync(false);
/// @endcode
class Window {
public:
    /// システムの SFML バックエンドを使う。
    Window();

    /// バックエンドを差し替えて生成する（テスト用）。
    /// @param backend 使用するバックエンド。呼び出し側が寿命を管理する。
    explicit Window(WindowBackend& backend) noexcept;

    /// ウィンドウを閉じる。
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    /// @brief 設定に従ってウィンドウを生成する。
    /// @param settings 適用する設定。
    /// @return 成功すれば空の結果、失敗すれば理由を持つ結果。
    [[nodiscard]] Result<void> open(const WindowSettings& settings);

    /// @brief ウィンドウを閉じる。
    void close();

    /// @brief ウィンドウが開いているか判定する。
    /// @return 開いていれば true。
    [[nodiscard]] bool isOpen() const;

    /// @brief タイトルを設定する。
    /// @param title 新しいタイトル。
    /// @return 成功すれば空の結果、失敗すれば理由を持つ結果。
    [[nodiscard]] Result<void> setTitle(std::string_view title);

    /// @brief タイトルを返す。
    /// @return 現在のタイトル。
    [[nodiscard]] std::string title() const;

    /// @brief 解像度を変更する。
    /// @param size 新しい大きさ（ピクセル）。
    /// @return 成功すれば空の結果、失敗すれば理由を持つ結果。
    [[nodiscard]] Result<void> setSize(Vector2Int size);

    /// @brief 現在の解像度を返す。
    /// @return ウィンドウの大きさ（ピクセル）。
    [[nodiscard]] Vector2Int size() const;

    /// @brief フルスクリーンを切り替える。
    /// @param enabled true ならフルスクリーン、false ならウィンドウ表示。
    /// @return 成功すれば空の結果、失敗すれば理由を持つ結果。
    [[nodiscard]] Result<void> setFullscreen(bool enabled);

    /// @brief フルスクリーン中か判定する。
    /// @return フルスクリーンなら true。
    [[nodiscard]] bool isFullscreen() const;

    /// @brief 垂直同期を切り替える。
    /// @param enabled true なら有効、false なら無効。
    /// @return 成功すれば空の結果、失敗すれば理由を持つ結果。
    [[nodiscard]] Result<void> setVerticalSync(bool enabled);

    /// @brief 垂直同期が有効か判定する。
    /// @return 有効なら true。
    [[nodiscard]] bool isVerticalSyncEnabled() const;

    /// @brief ウィンドウアイコンを設定する。
    /// @param size アイコンの大きさ（ピクセル）。
    /// @param pixels RGBA を並べたピクセルデータ（size.x * size.y * 4 バイト）。
    /// @return 成功すれば空の結果、失敗すれば理由を持つ結果。
    [[nodiscard]] Result<void> setIcon(Vector2Int size, const std::uint8_t* pixels);

private:
    WindowBackend* m_backend;
};

}  // namespace sprout::detail
