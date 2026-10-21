#include "core/fs/NativeMount.hpp"

#include <algorithm>
#include <cstddef>
#include <format>
#include <fstream>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
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

namespace sprout::detail {
namespace {

std::optional<std::filesystem::path> pathFromUtf8(std::string_view text)
{
#if defined(_WIN32)
    if (text.empty()) {
        return std::filesystem::path{};
    }
    if (text.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        return std::nullopt;
    }
    const int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(),
        static_cast<int>(text.size()), nullptr, 0);
    if (size <= 0) {
        return std::nullopt;
    }
    std::wstring wide(static_cast<std::size_t>(size), L'\0');
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), static_cast<int>(text.size()),
        wide.data(), size);
    return std::filesystem::path{std::move(wide)};
#else
    return std::filesystem::path{std::string{text}};
#endif
}

std::string pathToUtf8(const std::filesystem::path& path)
{
#if defined(_WIN32)
    const std::wstring native = path.native();
    if (native.empty()) {
        return {};
    }
    if (native.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
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

ErrorCode mapFileError(const std::error_code& error)
{
    if (error == std::errc::permission_denied) {
        return ErrorCode::PermissionDenied;
    }
    return ErrorCode::Io;
}

}  // namespace

NativeMount::NativeMount(std::filesystem::path root)
    : m_root{std::move(root)}
{
}

Result<std::shared_ptr<NativeMount>> NativeMount::create(std::string_view utf8Root)
{
    const std::optional<std::filesystem::path> root = pathFromUtf8(utf8Root);
    if (!root || root->empty()) {
        return Error{ErrorCode::InvalidArgument, std::format("ルートパスが不正です: {}", utf8Root)};
    }

    std::error_code error;
    const std::filesystem::file_status status = std::filesystem::status(*root, error);
    if (error || !std::filesystem::exists(status)) {
        return Error{ErrorCode::NotFound, std::format("ルートフォルダが見つかりません: {}", utf8Root)};
    }
    if (!std::filesystem::is_directory(status)) {
        return Error{ErrorCode::InvalidArgument, std::format("ルートがフォルダではありません: {}", utf8Root)};
    }
    return std::make_shared<NativeMount>(*root);
}

const std::filesystem::path& NativeMount::root() const noexcept
{
    return m_root;
}

std::optional<std::filesystem::path> NativeMount::resolve(std::string_view relativePath) const
{
    const std::optional<std::string> normalized = FileSystem::tryNormalize(relativePath);
    if (!normalized) {
        return std::nullopt;
    }
    if (normalized->empty()) {
        return m_root;
    }
    const std::optional<std::filesystem::path> relative = pathFromUtf8(*normalized);
    if (!relative) {
        return std::nullopt;
    }
    return m_root / *relative;
}

bool NativeMount::exists(std::string_view relativePath) const
{
    const std::optional<std::filesystem::path> path = resolve(relativePath);
    if (!path) {
        return false;
    }
    std::error_code error;
    const std::filesystem::file_status status = std::filesystem::status(*path, error);
    return !error && std::filesystem::exists(status);
}

bool NativeMount::isDirectory(std::string_view relativePath) const
{
    const std::optional<std::filesystem::path> path = resolve(relativePath);
    if (!path) {
        return false;
    }
    std::error_code error;
    const std::filesystem::file_status status = std::filesystem::status(*path, error);
    return !error && std::filesystem::is_directory(status);
}

Result<std::vector<std::byte>> NativeMount::read(std::string_view relativePath) const
{
    const std::optional<std::filesystem::path> path = resolve(relativePath);
    if (!path) {
        return Error{ErrorCode::InvalidArgument, std::format("パスが不正です: {}", relativePath)};
    }

    std::error_code error;
    if (std::filesystem::is_directory(*path, error)) {
        return Error{ErrorCode::InvalidArgument,
            std::format("フォルダは読み込めません: {}", pathToUtf8(*path))};
    }

    std::ifstream input{*path, std::ios::binary};
    if (!input) {
        if (!std::filesystem::exists(*path, error)) {
            return Error{ErrorCode::NotFound,
                std::format("ファイルが見つかりません: {}", pathToUtf8(*path))};
        }
        return Error{mapFileError(error),
            std::format("ファイルを開けません: {}", pathToUtf8(*path))};
    }

    std::vector<std::byte> data;
    char buffer[8192];
    while (input.read(buffer, static_cast<std::streamsize>(sizeof(buffer))) || input.gcount() > 0) {
        const std::streamsize count = input.gcount();
        data.reserve(data.size() + static_cast<std::size_t>(count));
        for (std::streamsize index = 0; index < count; ++index) {
            data.push_back(static_cast<std::byte>(static_cast<unsigned char>(buffer[index])));
        }
    }
    if (input.bad()) {
        return Error{ErrorCode::Io,
            std::format("ファイルの読み込みに失敗しました: {}", pathToUtf8(*path))};
    }
    return data;
}

Result<std::vector<std::string>> NativeMount::list(std::string_view relativeDirectory) const
{
    const std::optional<std::filesystem::path> path = resolve(relativeDirectory);
    if (!path) {
        return Error{ErrorCode::InvalidArgument, std::format("パスが不正です: {}", relativeDirectory)};
    }

    std::error_code error;
    const std::filesystem::file_status status = std::filesystem::status(*path, error);
    if (error || !std::filesystem::exists(status)) {
        return Error{ErrorCode::NotFound,
            std::format("フォルダが見つかりません: {}", pathToUtf8(*path))};
    }
    if (!std::filesystem::is_directory(status)) {
        return Error{ErrorCode::InvalidArgument,
            std::format("フォルダではありません: {}", pathToUtf8(*path))};
    }

    std::vector<std::string> names;
    std::filesystem::directory_iterator iterator{*path, error};
    const std::filesystem::directory_iterator end;
    if (error) {
        return Error{mapFileError(error),
            std::format("フォルダを列挙できません: {}", pathToUtf8(*path))};
    }
    while (iterator != end) {
        const std::string name = pathToUtf8(iterator->path().filename());
        if (!name.empty()) {
            names.push_back(name);
        }
        iterator.increment(error);
        if (error) {
            return Error{mapFileError(error),
                std::format("フォルダを列挙できません: {}", pathToUtf8(*path))};
        }
    }

    std::sort(names.begin(), names.end());
    return names;
}

}  // namespace sprout::detail
