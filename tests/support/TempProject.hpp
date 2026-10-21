#pragma once

#include <chrono>
#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>

namespace sprout::test {

/// テスト専用の一時プロジェクトフォルダ。破棄時に自動で削除する。
class TempProject {
public:
    explicit TempProject(std::string_view name = "project") {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        m_path = std::filesystem::temp_directory_path() /
                 ("sprout-" + std::string(name) + "-" + std::to_string(stamp));
        std::filesystem::create_directories(m_path);
    }

    ~TempProject() {
        std::error_code error;
        std::filesystem::remove_all(m_path, error);
    }

    TempProject(const TempProject&) = delete;
    TempProject& operator=(const TempProject&) = delete;
    TempProject(TempProject&&) = delete;
    TempProject& operator=(TempProject&&) = delete;

    [[nodiscard]] const std::filesystem::path& path() const { return m_path; }

    [[nodiscard]] std::filesystem::path assetPath(std::string_view relative) const {
        return m_path / std::filesystem::path(relative);
    }

private:
    std::filesystem::path m_path;
};

}  // namespace sprout::test
