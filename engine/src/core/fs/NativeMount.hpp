#pragma once

#include "core/fs/FileSystem.hpp"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace sprout::detail {

/// @brief OS のフォルダをそのまま公開するマウント。
///
/// パスは UTF-8 で受け取り、Windows では UTF-16 に変換して
/// 日本語を含むファイル名を扱う。
class NativeMount final : public IMount {
public:
    /// @brief ルートフォルダを指定して生成する。
    /// @param root ルートフォルダのネイティブパス。
    explicit NativeMount(std::filesystem::path root);

    /// @brief UTF-8 のルートパスから生成する。存在するフォルダでなければ失敗する。
    /// @param utf8Root ルートフォルダ（UTF-8）。
    /// @return 生成したマウント。不正なパスは InvalidArgument、不在は NotFound。
    static Result<std::shared_ptr<NativeMount>> create(std::string_view utf8Root);

    /// @brief ルートフォルダを返す。
    [[nodiscard]] const std::filesystem::path& root() const noexcept;

    [[nodiscard]] bool exists(std::string_view relativePath) const override;
    [[nodiscard]] bool isDirectory(std::string_view relativePath) const override;
    [[nodiscard]] Result<std::vector<std::byte>> read(std::string_view relativePath) const override;
    [[nodiscard]] Result<std::vector<std::string>> list(std::string_view relativeDirectory) const override;

private:
    [[nodiscard]] std::optional<std::filesystem::path> resolve(std::string_view relativePath) const;

    std::filesystem::path m_root;
};

}  // namespace sprout::detail
