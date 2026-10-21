#include "core/fs/FileSystem.hpp"

#include <algorithm>
#include <format>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace sprout::detail {
namespace {

bool prefixMatches(std::string_view prefix, std::string_view path) noexcept
{
    if (prefix.empty()) {
        return true;
    }
    if (path == prefix) {
        return true;
    }
    return path.size() > prefix.size() && path.starts_with(prefix) && path[prefix.size()] == '/';
}

}  // namespace

std::optional<std::string> FileSystem::tryNormalize(std::string_view path)
{
    if (path.find('\0') != std::string_view::npos) {
        return std::nullopt;
    }

    std::vector<std::string_view> segments;
    std::size_t start = 0;
    while (true) {
        const std::size_t end = path.find_first_of("/\\", start);
        const std::string_view segment =
            path.substr(start, end == std::string_view::npos ? std::string_view::npos : end - start);

        if (segment.empty() || segment == ".") {
            // 空セグメントと "." は無視する。
        } else if (segment == "..") {
            if (segments.empty()) {
                return std::nullopt;
            }
            segments.pop_back();
        } else {
            // 仮想パスにドライブ文字や代替データストリームを持ち込ませない。
            if (segment.find(':') != std::string_view::npos) {
                return std::nullopt;
            }
            segments.push_back(segment);
        }

        if (end == std::string_view::npos) {
            break;
        }
        start = end + 1;
    }

    std::string normalized;
    for (const std::string_view segment : segments) {
        if (!normalized.empty()) {
            normalized.push_back('/');
        }
        normalized.append(segment);
    }
    return normalized;
}

std::string FileSystem::normalize(std::string_view path)
{
    return tryNormalize(path).value_or(std::string{});
}

Result<void> FileSystem::mount(std::string_view mountPoint, std::shared_ptr<IMount> mount)
{
    const std::optional<std::string> prefix = tryNormalize(mountPoint);
    if (!prefix) {
        return Error{ErrorCode::InvalidArgument, std::format("マウントポイントが不正です: {}", mountPoint)};
    }
    if (!mount) {
        return Error{ErrorCode::InvalidArgument, "マウントする実体がありません"};
    }

    for (MountEntry& entry : m_mounts) {
        if (entry.prefix == *prefix) {
            entry.mount = std::move(mount);
            return {};
        }
    }
    m_mounts.push_back(MountEntry{*prefix, std::move(mount)});
    return {};
}

bool FileSystem::unmount(std::string_view mountPoint)
{
    const std::optional<std::string> prefix = tryNormalize(mountPoint);
    if (!prefix) {
        return false;
    }
    const auto found = std::find_if(m_mounts.begin(), m_mounts.end(),
        [&prefix](const MountEntry& entry) { return entry.prefix == *prefix; });
    if (found == m_mounts.end()) {
        return false;
    }
    m_mounts.erase(found);
    return true;
}

bool FileSystem::isMounted(std::string_view mountPoint) const
{
    const std::optional<std::string> prefix = tryNormalize(mountPoint);
    if (!prefix) {
        return false;
    }
    return std::any_of(m_mounts.begin(), m_mounts.end(),
        [&prefix](const MountEntry& entry) { return entry.prefix == *prefix; });
}

std::vector<std::string> FileSystem::mountPoints() const
{
    std::vector<std::string> points;
    points.reserve(m_mounts.size());
    for (const MountEntry& entry : m_mounts) {
        points.push_back(entry.prefix);
    }
    std::sort(points.begin(), points.end());
    return points;
}

const FileSystem::MountEntry* FileSystem::findMount(std::string_view normalizedPath, std::string& relativePath) const
{
    const MountEntry* best = nullptr;
    for (const MountEntry& entry : m_mounts) {
        if (!prefixMatches(entry.prefix, normalizedPath)) {
            continue;
        }
        if (best == nullptr || entry.prefix.size() > best->prefix.size()) {
            best = &entry;
        }
    }
    if (best == nullptr) {
        return nullptr;
    }

    if (best->prefix.empty()) {
        relativePath.assign(normalizedPath);
    } else if (normalizedPath.size() == best->prefix.size()) {
        relativePath.clear();
    } else {
        relativePath.assign(normalizedPath.substr(best->prefix.size() + 1));
    }
    return best;
}

bool FileSystem::exists(std::string_view path) const
{
    const std::optional<std::string> normalized = tryNormalize(path);
    if (!normalized) {
        return false;
    }
    std::string relativePath;
    const MountEntry* entry = findMount(*normalized, relativePath);
    if (entry == nullptr) {
        return false;
    }
    return entry->mount->exists(relativePath);
}

bool FileSystem::isDirectory(std::string_view path) const
{
    const std::optional<std::string> normalized = tryNormalize(path);
    if (!normalized) {
        return false;
    }
    std::string relativePath;
    const MountEntry* entry = findMount(*normalized, relativePath);
    if (entry == nullptr) {
        return false;
    }
    return entry->mount->isDirectory(relativePath);
}

Result<std::vector<std::byte>> FileSystem::read(std::string_view path) const
{
    const std::optional<std::string> normalized = tryNormalize(path);
    if (!normalized) {
        return Error{ErrorCode::InvalidArgument, std::format("パスが不正です: {}", path)};
    }
    std::string relativePath;
    const MountEntry* entry = findMount(*normalized, relativePath);
    if (entry == nullptr) {
        return Error{ErrorCode::NotFound, std::format("パスに対応するマウントがありません: {}", path)};
    }
    return entry->mount->read(relativePath);
}

Result<std::string> FileSystem::readText(std::string_view path) const
{
    const Result<std::vector<std::byte>> bytes = read(path);
    if (!bytes.isOk()) {
        return bytes.error();
    }

    std::string text;
    text.reserve(bytes.value().size());
    for (const std::byte value : bytes.value()) {
        text.push_back(static_cast<char>(value));
    }
    return text;
}

Result<std::vector<std::string>> FileSystem::list(std::string_view path) const
{
    const std::optional<std::string> normalized = tryNormalize(path);
    if (!normalized) {
        return Error{ErrorCode::InvalidArgument, std::format("パスが不正です: {}", path)};
    }
    std::string relativePath;
    const MountEntry* entry = findMount(*normalized, relativePath);
    if (entry == nullptr) {
        return Error{ErrorCode::NotFound, std::format("パスに対応するマウントがありません: {}", path)};
    }

    const Result<std::vector<std::string>> children = entry->mount->list(relativePath);
    if (!children.isOk()) {
        return children.error();
    }

    std::string directory = entry->prefix;
    if (!relativePath.empty()) {
        if (!directory.empty()) {
            directory.push_back('/');
        }
        directory.append(relativePath);
    }

    std::vector<std::string> paths;
    paths.reserve(children.value().size());
    for (const std::string& child : children.value()) {
        if (directory.empty()) {
            paths.push_back(child);
        } else {
            paths.push_back(directory + "/" + child);
        }
    }
    std::sort(paths.begin(), paths.end());
    return paths;
}

}  // namespace sprout::detail
