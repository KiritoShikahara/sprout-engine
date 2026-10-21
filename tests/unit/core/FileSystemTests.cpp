#include "core/fs/FileSystem.hpp"
#include "core/fs/NativeMount.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <gtest/gtest.h>

namespace {

using sprout::ErrorCode;
using sprout::detail::FileSystem;
using sprout::detail::IMount;
using sprout::detail::NativeMount;

std::string fromUtf8Literal(const char8_t* text)
{
    return std::string{reinterpret_cast<const char*>(text)};
}

std::filesystem::path osPath(std::string_view utf8)
{
#if defined(_WIN32)
    if (utf8.empty()) {
        return {};
    }
    const int size = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), nullptr, 0);
    if (size <= 0) {
        return {};
    }
    std::wstring wide(static_cast<std::size_t>(size), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), wide.data(), size);
    return std::filesystem::path{wide};
#else
    return std::filesystem::path{std::string{utf8}};
#endif
}

std::string nativePathToUtf8(const std::filesystem::path& path)
{
#if defined(_WIN32)
    const std::wstring native = path.native();
    if (native.empty()) {
        return {};
    }
    const int size = WideCharToMultiByte(CP_UTF8, 0, native.data(), static_cast<int>(native.size()),
        nullptr, 0, nullptr, nullptr);
    if (size <= 0) {
        return {};
    }
    std::string result(static_cast<std::size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, native.data(), static_cast<int>(native.size()), result.data(), size,
        nullptr, nullptr);
    return result;
#else
    return path.string();
#endif
}

bool contains(const std::vector<std::string>& values, std::string_view expected)
{
    return std::find(values.begin(), values.end(), expected) != values.end();
}

class MemoryMount final : public IMount {
public:
    void add(std::string path, std::string content)
    {
        m_files.insert_or_assign(std::move(path), std::move(content));
    }

    [[nodiscard]] bool exists(std::string_view relativePath) const override
    {
        if (m_files.find(std::string{relativePath}) != m_files.end()) {
            return true;
        }
        return isDirectory(relativePath);
    }

    [[nodiscard]] bool isDirectory(std::string_view relativePath) const override
    {
        if (relativePath.empty()) {
            return true;
        }
        const std::string prefix = std::string{relativePath} + "/";
        return std::any_of(m_files.begin(), m_files.end(),
            [&prefix](const auto& entry) { return entry.first.starts_with(prefix); });
    }

    [[nodiscard]] sprout::Result<std::vector<std::byte>> read(std::string_view relativePath) const override
    {
        const auto found = m_files.find(std::string{relativePath});
        if (found == m_files.end()) {
            return sprout::Error{ErrorCode::NotFound, std::string{relativePath}};
        }
        std::vector<std::byte> data;
        data.reserve(found->second.size());
        for (const char value : found->second) {
            data.push_back(static_cast<std::byte>(static_cast<unsigned char>(value)));
        }
        return data;
    }

    [[nodiscard]] sprout::Result<std::vector<std::string>> list(std::string_view relativeDirectory) const override
    {
        if (!isDirectory(relativeDirectory)) {
            return sprout::Error{ErrorCode::NotFound, std::string{relativeDirectory}};
        }
        const std::string prefix = relativeDirectory.empty() ? std::string{} : std::string{relativeDirectory} + "/";
        std::vector<std::string> names;
        for (const auto& entry : m_files) {
            if (!entry.first.starts_with(prefix)) {
                continue;
            }
            const std::string rest = entry.first.substr(prefix.size());
            if (rest.find('/') == std::string::npos) {
                names.push_back(rest);
            }
        }
        std::sort(names.begin(), names.end());
        return names;
    }

private:
    std::unordered_map<std::string, std::string> m_files;
};

class FileSystemTests : public ::testing::Test {
protected:
    void SetUp() override
    {
        static std::atomic<unsigned> counter{0};
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        const std::string name = "sprout_fs_" + std::to_string(stamp) + "_"
                                 + std::to_string(counter.fetch_add(1, std::memory_order_relaxed));
        m_root = std::filesystem::temp_directory_path() / osPath(name);

        std::error_code error;
        std::filesystem::create_directories(m_root, error);
        ASSERT_FALSE(error) << error.message();
    }

    void TearDown() override
    {
        std::error_code error;
        std::filesystem::remove_all(m_root, error);
    }

    void writeFile(std::string_view relativePath, std::string_view content)
    {
        const std::filesystem::path path = m_root / osPath(relativePath);
        std::error_code error;
        if (!path.parent_path().empty()) {
            std::filesystem::create_directories(path.parent_path(), error);
        }
        ASSERT_FALSE(error) << error.message();

        std::ofstream output{path, std::ios::binary};
        ASSERT_TRUE(output.good());
        output.write(content.data(), static_cast<std::streamsize>(content.size()));
        ASSERT_TRUE(output.good());
    }

    std::filesystem::path m_root;
};

TEST(FileSystemPathTests, Normalize_UnifiesSeparatorsAndRemovesRedundantSegments)
{
    EXPECT_EQ(FileSystem::normalize("assets\\player.png"), "assets/player.png");
    EXPECT_EQ(FileSystem::normalize("//assets///./sprites/../player.png"), "assets/player.png");
    EXPECT_EQ(FileSystem::normalize("assets/sub/"), "assets/sub");
    EXPECT_EQ(FileSystem::normalize("."), "");
    EXPECT_EQ(FileSystem::normalize(""), "");
}

TEST(FileSystemPathTests, Normalize_PreservesUtf8JapaneseSegments)
{
    const std::string path = fromUtf8Literal(u8"アセット\\日本語/スプライト.png");

    EXPECT_EQ(FileSystem::normalize(path), fromUtf8Literal(u8"アセット/日本語/スプライト.png"));
}

TEST(FileSystemPathTests, Normalize_RejectsEscapingPathsAndDriveLetters)
{
    EXPECT_FALSE(FileSystem::tryNormalize("../secret.txt").has_value());
    EXPECT_FALSE(FileSystem::tryNormalize("assets/../../secret.txt").has_value());
    EXPECT_FALSE(FileSystem::tryNormalize("C:\\assets\\player.png").has_value());
    EXPECT_TRUE(FileSystem::normalize("../secret.txt").empty());
}

TEST_F(FileSystemTests, NativeMount_ReadsJapanesePathsAndUtf8Content)
{
    const std::string rootName = fromUtf8Literal(u8"日本語ルート");
    const std::string textPath = fromUtf8Literal(u8"こんにちは.txt");
    const std::string folderName = fromUtf8Literal(u8"データ");
    const std::string configName = fromUtf8Literal(u8"設定.ini");
    const std::string message = fromUtf8Literal(u8"やあ、世界！");
    const std::string config = fromUtf8Literal(u8"key=値");

    writeFile(rootName + "/" + textPath, message);
    writeFile(rootName + "/" + folderName + "/" + configName, config);

    const sprout::Result<std::shared_ptr<NativeMount>> native =
        NativeMount::create(nativePathToUtf8(m_root / osPath(rootName)));
    ASSERT_TRUE(native.isOk()) << native.error().message();

    FileSystem files;
    ASSERT_TRUE(files.mount("", native.value()).isOk());

    EXPECT_TRUE(files.exists(textPath));
    EXPECT_TRUE(files.isDirectory(folderName));
    EXPECT_FALSE(files.exists(fromUtf8Literal(u8"存在しない.txt")));

    const sprout::Result<std::string> readText = files.readText(textPath);
    ASSERT_TRUE(readText.isOk()) << readText.error().message();
    EXPECT_EQ(readText.value(), message);

    const sprout::Result<std::vector<std::byte>> bytes = files.read(textPath);
    ASSERT_TRUE(bytes.isOk()) << bytes.error().message();
    ASSERT_EQ(bytes.value().size(), message.size());
    for (std::size_t index = 0; index < message.size(); ++index) {
        EXPECT_EQ(static_cast<unsigned int>(bytes.value()[index]),
            static_cast<unsigned int>(static_cast<unsigned char>(message[index])));
    }

    const sprout::Result<std::vector<std::string>> rootEntries = files.list("");
    ASSERT_TRUE(rootEntries.isOk()) << rootEntries.error().message();
    EXPECT_TRUE(contains(rootEntries.value(), textPath));
    EXPECT_TRUE(contains(rootEntries.value(), folderName));

    const sprout::Result<std::vector<std::string>> dataEntries = files.list(folderName);
    ASSERT_TRUE(dataEntries.isOk()) << dataEntries.error().message();
    EXPECT_EQ(dataEntries.value(), std::vector<std::string>{folderName + "/" + configName});
}

TEST_F(FileSystemTests, NativeMount_MissingFile_ReturnsNotFound)
{
    writeFile("root.txt", "root");
    const auto native = NativeMount::create(nativePathToUtf8(m_root));
    ASSERT_TRUE(native.isOk());

    FileSystem files;
    ASSERT_TRUE(files.mount("", native.value()).isOk());

    const sprout::Result<std::string> missing = files.readText("missing.txt");
    ASSERT_FALSE(missing.isOk());
    EXPECT_EQ(missing.error().code(), ErrorCode::NotFound);
}

TEST_F(FileSystemTests, NativeMount_RejectsTraversalOutsideRoot)
{
    writeFile("mounted/inside.txt", "inside");
    writeFile("secret.txt", "secret");

    const auto native = NativeMount::create(nativePathToUtf8(m_root / "mounted"));
    ASSERT_TRUE(native.isOk());
    FileSystem files;
    ASSERT_TRUE(files.mount("", native.value()).isOk());

    const sprout::Result<std::string> escaped = files.readText("../secret.txt");
    ASSERT_FALSE(escaped.isOk());
    EXPECT_EQ(escaped.error().code(), ErrorCode::InvalidArgument);
    EXPECT_FALSE(files.exists("../secret.txt"));
    EXPECT_FALSE(native.value()->exists("../secret.txt"));
}

TEST_F(FileSystemTests, NativeMount_CreateRejectsMissingOrInvalidRoot)
{
    const auto missing = NativeMount::create(nativePathToUtf8(m_root / "missing"));
    ASSERT_FALSE(missing.isOk());
    EXPECT_EQ(missing.error().code(), ErrorCode::NotFound);

    const auto invalid = NativeMount::create("");
    ASSERT_FALSE(invalid.isOk());
    EXPECT_EQ(invalid.error().code(), ErrorCode::InvalidArgument);
}

TEST_F(FileSystemTests, Mount_UsesLongestPrefixAndSwitchesNativeToPack)
{
    writeFile("assets/player.txt", "native-player");
    writeFile("assets/audio/theme.txt", "native-theme");
    writeFile("readme.txt", "root");

    const auto native = NativeMount::create(nativePathToUtf8(m_root));
    ASSERT_TRUE(native.isOk());

    FileSystem files;
    ASSERT_TRUE(files.mount("", native.value()).isOk());
    EXPECT_FALSE(files.mount("../assets", std::make_shared<MemoryMount>()).isOk());
    EXPECT_TRUE(files.readText("readme.txt").isOk());

    auto pack = std::make_shared<MemoryMount>();
    pack->add("player.txt", "pack-player");
    ASSERT_TRUE(files.mount("assets", pack).isOk());
    EXPECT_TRUE(files.isMounted("assets"));
    EXPECT_EQ(files.readText("assets/player.txt").value(), "pack-player");

    auto nested = std::make_shared<MemoryMount>();
    nested->add("data.bin", "nested-data");
    ASSERT_TRUE(files.mount("assets/audio", nested).isOk());
    EXPECT_EQ(files.readText("assets/audio/data.bin").value(), "nested-data");
    EXPECT_EQ(files.readText("assets/audio/theme.txt").error().code(), ErrorCode::NotFound);

    auto replacement = std::make_shared<MemoryMount>();
    replacement->add("player.txt", "pack-player-v2");
    ASSERT_TRUE(files.mount("assets", replacement).isOk());
    EXPECT_EQ(files.readText("assets/player.txt").value(), "pack-player-v2");
    EXPECT_EQ(files.mountPoints().size(), std::size_t{3});

    EXPECT_TRUE(files.unmount("assets"));
    EXPECT_FALSE(files.isMounted("assets"));
    EXPECT_FALSE(files.unmount("assets"));
    EXPECT_EQ(files.readText("assets/player.txt").value(), "native-player");
}

TEST(FileSystemMountTests, ReadWithoutMount_ReturnsNotFound)
{
    FileSystem files;
    const sprout::Result<std::vector<std::byte>> data = files.read("player.txt");

    ASSERT_FALSE(data.isOk());
    EXPECT_EQ(data.error().code(), ErrorCode::NotFound);
    EXPECT_FALSE(files.exists("player.txt"));
}

}  // namespace
