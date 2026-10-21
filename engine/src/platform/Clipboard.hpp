#pragma once

#include <string>
#include <string_view>

namespace sprout::detail {

/// @brief OS クリップボードの読み書き先を差し替えるための境界。
class ClipboardBackend {
public:
    virtual ~ClipboardBackend() = default;

    /// @brief クリップボードの UTF-8 テキストを読み取る。
    /// @return テキスト。クリップボードが空なら空文字列。
    [[nodiscard]] virtual std::string read() const = 0;

    /// @brief UTF-8 テキストをクリップボードへ書き込む。
    /// @param text 書き込むテキスト。空文字列ならクリップボードを消去する。
    virtual void write(std::string_view text) = 0;
};

/// @brief SFML 経由で OS のクリップボードを使うバックエンドを返す。
/// @return プロセス内で共有されるバックエンド。
ClipboardBackend& systemClipboardBackend();

/// @brief テキストのコピー＆ペーストに使うクリップボード。
///
/// 文字列は UTF-8 で扱う。OS のクリップボードを触るため、呼び出しは
/// メインスレッドから行うこと。
///
/// @code
/// sprout::detail::Clipboard clipboard;
/// clipboard.setText("こんにちは");
/// const std::string pasted = clipboard.text();
/// @endcode
class Clipboard {
public:
    /// システムクリップボードを使う。
    Clipboard() noexcept;

    /// 読み書き先を差し替えて生成する（テスト用）。
    /// @param backend 使用するバックエンド。呼び出し側が寿命を管理する。
    explicit Clipboard(ClipboardBackend& backend) noexcept;

    /// @brief クリップボードのテキストを取得する。
    /// @return UTF-8 テキスト。空なら空文字列。
    [[nodiscard]] std::string text() const;

    /// @brief テキストをクリップボードへ設定する。
    /// @param text UTF-8 テキスト。空文字列なら消去する。
    void setText(std::string_view text);

    /// @brief クリップボードに空でないテキストがあるか判定する。
    /// @return テキストがあれば true。
    [[nodiscard]] bool hasText() const;

    /// @brief クリップボードを空にする。
    void clear();

private:
    ClipboardBackend* m_backend;
};

}  // namespace sprout::detail
