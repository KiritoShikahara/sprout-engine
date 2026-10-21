#pragma once

#include "sprout/core/Result.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace sprout::detail {

/// @brief 仮想パス配下の実体（ネイティブフォルダ / パック）を表す。
///
/// 渡されるパスは正規化済みの相対パス（'/' 区切り）で、マウントルートは空文字列。
class IMount {
public:
    virtual ~IMount() = default;

    /// @brief ファイルまたはフォルダが存在するか判定する。
    [[nodiscard]] virtual bool exists(std::string_view relativePath) const = 0;

    /// @brief フォルダか判定する。
    [[nodiscard]] virtual bool isDirectory(std::string_view relativePath) const = 0;

    /// @brief ファイル全体を読み込む。
    [[nodiscard]] virtual Result<std::vector<std::byte>> read(std::string_view relativePath) const = 0;

    /// @brief フォルダ直下の名前（ファイル名 / フォルダ名）を列挙する。
    [[nodiscard]] virtual Result<std::vector<std::string>> list(std::string_view relativeDirectory) const = 0;
};

/// @brief 仮想ファイルシステム。
///
/// 仮想パスをマウントポイントの最長一致で解決する。同じマウントポイントへ
/// 再マウントすると差し替えるため、ネイティブフォルダからパックへの切替に使える。
///
/// @code
/// sprout::detail::FileSystem files;
/// files.mount("", sprout::detail::NativeMount::create("Assets").value());
/// const auto text = files.readText("日本語/メッセージ.txt");
/// @endcode
class FileSystem {
public:
    FileSystem() = default;

    /// @brief 仮想パスを正規化する。
    ///
    /// '\\' を '/' に統一し、連続する区切り・"." を除去し、".." を解決する。
    /// 先頭の '/' は仮想ルートとして除去する。ルートより上へ出るパスや
    /// ドライブ文字・NUL を含むパスは不正。
    ///
    /// @param path 正規化するパス（UTF-8）。
    /// @return 正規化したパス。不正なら std::nullopt。
    static std::optional<std::string> tryNormalize(std::string_view path);

    /// @brief tryNormalize() の簡易版。
    /// @param path 正規化するパス（UTF-8）。
    /// @return 正規化したパス。不正なら空文字列。
    static std::string normalize(std::string_view path);

    /// @brief マウントポイントに実体を割り当てる。
    ///
    /// 同じマウントポイントが既にあれば差し替える。
    ///
    /// @param mountPoint 仮想パス上のマウントポイント（"" はルート）。
    /// @param mount マウントする実体。
    Result<void> mount(std::string_view mountPoint, std::shared_ptr<IMount> mount);

    /// @brief マウントを解除する。
    /// @param mountPoint 解除するマウントポイント。
    /// @return 解除できたら true。
    bool unmount(std::string_view mountPoint);

    /// @brief マウントされているか判定する。
    [[nodiscard]] bool isMounted(std::string_view mountPoint) const;

    /// @brief 現在のマウントポイント一覧（昇順）。
    [[nodiscard]] std::vector<std::string> mountPoints() const;

    /// @brief 仮想パスが存在するか判定する。不正・未マウントなら false。
    [[nodiscard]] bool exists(std::string_view path) const;

    /// @brief 仮想パスがフォルダか判定する。不正・未マウントなら false。
    [[nodiscard]] bool isDirectory(std::string_view path) const;

    /// @brief ファイル全体を読み込む。
    [[nodiscard]] Result<std::vector<std::byte>> read(std::string_view path) const;

    /// @brief ファイル全体を UTF-8 テキストとして読み込む。
    [[nodiscard]] Result<std::string> readText(std::string_view path) const;

    /// @brief フォルダ直下を仮想パスで列挙する（昇順）。
    [[nodiscard]] Result<std::vector<std::string>> list(std::string_view path) const;

private:
    struct MountEntry {
        std::string prefix;
        std::shared_ptr<IMount> mount;
    };

    [[nodiscard]] const MountEntry* findMount(std::string_view normalizedPath, std::string& relativePath) const;

    std::vector<MountEntry> m_mounts;
};

}  // namespace sprout::detail
