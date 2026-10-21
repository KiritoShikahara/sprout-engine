#include "platform/Display.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <optional>
#include <string>
#include <string_view>
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
#else
#include <SFML/Window/VideoMode.hpp>
#endif

namespace {

#if defined(_WIN32)

/// DPI_AWARENESS_CONTEXT の値（Windows SDK の DPI_AWARENESS_CONTEXT_* に対応する）。
constexpr std::intptr_t kDpiContextUnaware = -1;
constexpr std::intptr_t kDpiContextSystemAware = -2;
constexpr std::intptr_t kDpiContextPerMonitorAware = -3;
constexpr std::intptr_t kDpiContextPerMonitorAwareV2 = -4;

/// DPI_AWARENESS の値（Windows SDK の DPI_AWARENESS_* に対応する）。
constexpr int kDpiAwarenessUnaware = 0;
constexpr int kDpiAwarenessSystemAware = 1;
constexpr int kDpiAwarenessPerMonitorAware = 2;

/// MONITOR_DPI_TYPE の MDT_EFFECTIVE_DPI。
constexpr int kMonitorDpiTypeEffective = 0;

/// 失敗時に使う DPI（96 = 拡大率 1.0）。
constexpr UINT kFallbackDpi = 96;

using SetProcessDpiAwarenessContextFunction = BOOL(WINAPI*)(void*);
using GetDpiAwarenessContextForProcessFunction = void*(WINAPI*)(HANDLE);
using GetThreadDpiAwarenessContextFunction = void*(WINAPI*)();
using GetAwarenessFromDpiAwarenessContextFunction = int(WINAPI*)(void*);
using AreDpiAwarenessContextsEqualFunction = BOOL(WINAPI*)(void*, void*);
using IsProcessDPIAwareFunction = BOOL(WINAPI*)();
using SetProcessDpiAwarenessFunction = HRESULT(WINAPI*)(int);
using GetProcessDpiAwarenessFunction = HRESULT(WINAPI*)(int*);
using GetDpiForMonitorFunction = HRESULT(WINAPI*)(HMONITOR, int, UINT*, UINT*);

/// @brief DPI 関連の Windows API。
///
/// 存在しない API があるため、実行時に動的ロードして無い場合は null にする。
struct DpiApi {
    SetProcessDpiAwarenessContextFunction setProcessDpiAwarenessContext{nullptr};
    GetDpiAwarenessContextForProcessFunction getDpiAwarenessContextForProcess{nullptr};
    GetThreadDpiAwarenessContextFunction getThreadDpiAwarenessContext{nullptr};
    GetAwarenessFromDpiAwarenessContextFunction getAwarenessFromDpiAwarenessContext{nullptr};
    AreDpiAwarenessContextsEqualFunction areDpiAwarenessContextsEqual{nullptr};
    IsProcessDPIAwareFunction isProcessDPIAware{nullptr};
    SetProcessDpiAwarenessFunction setProcessDpiAwareness{nullptr};
    GetProcessDpiAwarenessFunction getProcessDpiAwareness{nullptr};
    GetDpiForMonitorFunction getDpiForMonitor{nullptr};
};

template <typename TFunction>
TFunction loadFunction(HMODULE module, const char* name) noexcept
{
    if (module == nullptr) {
        return nullptr;
    }
    const FARPROC address = ::GetProcAddress(module, name);
    TFunction function = nullptr;
    static_assert(sizeof(function) == sizeof(address), "関数ポインタと FARPROC のサイズが一致しません");
    std::memcpy(&function, &address, sizeof(function));
    return function;
}

DpiApi loadDpiApi() noexcept
{
    DpiApi api;

    const HMODULE user32 = ::LoadLibraryW(L"user32.dll");
    api.setProcessDpiAwarenessContext =
        loadFunction<SetProcessDpiAwarenessContextFunction>(user32, "SetProcessDpiAwarenessContext");
    api.getDpiAwarenessContextForProcess =
        loadFunction<GetDpiAwarenessContextForProcessFunction>(user32, "GetDpiAwarenessContextForProcess");
    api.getThreadDpiAwarenessContext =
        loadFunction<GetThreadDpiAwarenessContextFunction>(user32, "GetThreadDpiAwarenessContext");
    api.getAwarenessFromDpiAwarenessContext =
        loadFunction<GetAwarenessFromDpiAwarenessContextFunction>(user32, "GetAwarenessFromDpiAwarenessContext");
    api.areDpiAwarenessContextsEqual =
        loadFunction<AreDpiAwarenessContextsEqualFunction>(user32, "AreDpiAwarenessContextsEqual");
    api.isProcessDPIAware = loadFunction<IsProcessDPIAwareFunction>(user32, "IsProcessDPIAware");

    const HMODULE shcore = ::LoadLibraryW(L"shcore.dll");
    api.setProcessDpiAwareness = loadFunction<SetProcessDpiAwarenessFunction>(shcore, "SetProcessDpiAwareness");
    api.getProcessDpiAwareness = loadFunction<GetProcessDpiAwarenessFunction>(shcore, "GetProcessDpiAwareness");
    api.getDpiForMonitor = loadFunction<GetDpiForMonitorFunction>(shcore, "GetDpiForMonitor");

    return api;
}

const DpiApi& dpiApi() noexcept
{
    static const DpiApi api = loadDpiApi();
    return api;
}

sprout::detail::DpiAwareness awarenessFromValue(int value) noexcept
{
    switch (value) {
    case kDpiAwarenessSystemAware:
        return sprout::detail::DpiAwareness::SystemAware;
    case kDpiAwarenessPerMonitorAware:
        return sprout::detail::DpiAwareness::PerMonitorAware;
    default:
        return sprout::detail::DpiAwareness::Unaware;
    }
}

sprout::detail::DpiAwareness awarenessFromContext(void* context) noexcept
{
    if (context == nullptr) {
        return sprout::detail::DpiAwareness::Unaware;
    }

    const DpiApi& api = dpiApi();
    if (api.areDpiAwarenessContextsEqual != nullptr &&
        api.areDpiAwarenessContextsEqual(context, reinterpret_cast<void*>(kDpiContextPerMonitorAwareV2)) != FALSE) {
        return sprout::detail::DpiAwareness::PerMonitorAwareV2;
    }
    if (api.getAwarenessFromDpiAwarenessContext != nullptr) {
        return awarenessFromValue(api.getAwarenessFromDpiAwarenessContext(context));
    }
    return sprout::detail::DpiAwareness::Unaware;
}

void* contextFromAwareness(sprout::detail::DpiAwareness awareness) noexcept
{
    switch (awareness) {
    case sprout::detail::DpiAwareness::PerMonitorAwareV2:
        return reinterpret_cast<void*>(kDpiContextPerMonitorAwareV2);
    case sprout::detail::DpiAwareness::PerMonitorAware:
        return reinterpret_cast<void*>(kDpiContextPerMonitorAware);
    case sprout::detail::DpiAwareness::SystemAware:
        return reinterpret_cast<void*>(kDpiContextSystemAware);
    case sprout::detail::DpiAwareness::Unaware:
    default:
        return reinterpret_cast<void*>(kDpiContextUnaware);
    }
}

int dpiAwarenessValue(sprout::detail::DpiAwareness awareness) noexcept
{
    switch (awareness) {
    case sprout::detail::DpiAwareness::SystemAware:
        return kDpiAwarenessSystemAware;
    case sprout::detail::DpiAwareness::PerMonitorAware:
    case sprout::detail::DpiAwareness::PerMonitorAwareV2:
        return kDpiAwarenessPerMonitorAware;
    case sprout::detail::DpiAwareness::Unaware:
    default:
        return kDpiAwarenessUnaware;
    }
}

sprout::detail::DpiAwareness currentDpiAwareness() noexcept
{
    const DpiApi& api = dpiApi();

    if (api.getDpiAwarenessContextForProcess != nullptr) {
        return awarenessFromContext(api.getDpiAwarenessContextForProcess(::GetCurrentProcess()));
    }
    if (api.getThreadDpiAwarenessContext != nullptr) {
        return awarenessFromContext(api.getThreadDpiAwarenessContext());
    }
    if (api.getProcessDpiAwareness != nullptr) {
        int value = kDpiAwarenessUnaware;
        if (SUCCEEDED(api.getProcessDpiAwareness(&value))) {
            return awarenessFromValue(value);
        }
    }
    if (api.isProcessDPIAware != nullptr && api.isProcessDPIAware() != FALSE) {
        return sprout::detail::DpiAwareness::SystemAware;
    }
    return sprout::detail::DpiAwareness::Unaware;
}

/// @brief OS に DPI 対応レベルを設定する。
/// @return OS 側が受け付けたなら true。
bool requestDpiAwareness(sprout::detail::DpiAwareness awareness) noexcept
{
    const DpiApi& api = dpiApi();

    if (api.setProcessDpiAwarenessContext != nullptr) {
        return api.setProcessDpiAwarenessContext(contextFromAwareness(awareness)) != FALSE;
    }
    if (api.setProcessDpiAwareness != nullptr) {
        return SUCCEEDED(api.setProcessDpiAwareness(dpiAwarenessValue(awareness)));
    }
    if (awareness == sprout::detail::DpiAwareness::SystemAware) {
        return ::SetProcessDPIAware() != FALSE;
    }
    return false;
}

std::string toUtf8(std::wstring_view text)
{
    if (text.empty()) {
        return {};
    }

    const int size =
        ::WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
    if (size <= 0) {
        return {};
    }

    std::string result(static_cast<std::size_t>(size), '\0');
    ::WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), result.data(), size, nullptr,
                          nullptr);
    return result;
}

float monitorScaleFactor(HMONITOR monitor) noexcept
{
    const DpiApi& api = dpiApi();
    if (api.getDpiForMonitor == nullptr) {
        return 1.0F;
    }

    UINT dpiX = kFallbackDpi;
    UINT dpiY = kFallbackDpi;
    if (FAILED(api.getDpiForMonitor(monitor, kMonitorDpiTypeEffective, &dpiX, &dpiY)) || dpiX == 0U) {
        return 1.0F;
    }
    return sprout::detail::dpiScaleFromDpi(static_cast<unsigned int>(dpiX));
}

/// @brief EnumDisplayMonitors のコールバックに渡す情報。
struct MonitorEnumeration {
    std::vector<sprout::detail::MonitorInfo>* monitors{nullptr};
};

BOOL CALLBACK collectMonitor(HMONITOR monitor, HDC /*deviceContext*/, LPRECT /*bounds*/, LPARAM parameter)
{
    auto* enumeration = reinterpret_cast<MonitorEnumeration*>(parameter);
    if (enumeration == nullptr || enumeration->monitors == nullptr) {
        return FALSE;
    }

    MONITORINFOEXW info{};
    info.cbSize = static_cast<DWORD>(sizeof(MONITORINFOEXW));
    if (::GetMonitorInfoW(monitor, &info) == 0) {
        return TRUE;
    }

    sprout::detail::MonitorInfo monitorInfo;
    monitorInfo.index = static_cast<int>(enumeration->monitors->size());
    monitorInfo.name = toUtf8(std::wstring_view{info.szDevice});
    monitorInfo.position = sprout::Vector2Int{info.rcMonitor.left, info.rcMonitor.top};
    monitorInfo.size =
        sprout::Vector2Int{info.rcMonitor.right - info.rcMonitor.left, info.rcMonitor.bottom - info.rcMonitor.top};
    monitorInfo.scaleFactor = monitorScaleFactor(monitor);
    monitorInfo.primary = (info.dwFlags & MONITORINFOF_PRIMARY) != 0;

    enumeration->monitors->push_back(std::move(monitorInfo));
    return TRUE;
}

#endif

#if defined(_WIN32)

/// @brief Win32 の画面情報を返すバックエンド。
class SystemDisplayBackend final : public sprout::detail::DisplayBackend {
public:
    [[nodiscard]] std::vector<sprout::detail::MonitorInfo> monitors() const override
    {
        std::vector<sprout::detail::MonitorInfo> result;
        MonitorEnumeration enumeration{&result};
        ::EnumDisplayMonitors(nullptr, nullptr, &collectMonitor, reinterpret_cast<LPARAM>(&enumeration));
        return result;
    }

    [[nodiscard]] sprout::detail::DpiAwareness awareness() const override
    {
        return currentDpiAwareness();
    }

    bool setAwareness(sprout::detail::DpiAwareness awareness) override
    {
        if (requestDpiAwareness(awareness)) {
            m_appliedAwareness = awareness;
            m_hasAppliedAwareness = true;
            return true;
        }
        if (m_hasAppliedAwareness && m_appliedAwareness == awareness) {
            return true;
        }
        return currentDpiAwareness() == awareness;
    }

private:
    sprout::detail::DpiAwareness m_appliedAwareness{sprout::detail::DpiAwareness::Unaware};
    bool m_hasAppliedAwareness{false};
};

#else

/// @brief SFML 経由で画面情報を返すバックエンド。
class SystemDisplayBackend final : public sprout::detail::DisplayBackend {
public:
    [[nodiscard]] std::vector<sprout::detail::MonitorInfo> monitors() const override
    {
        std::vector<sprout::detail::MonitorInfo> result;

        const sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
        if (desktop.size.x == 0U || desktop.size.y == 0U) {
            return result;
        }

        sprout::detail::MonitorInfo monitorInfo;
        monitorInfo.index = 0;
        monitorInfo.name = "Desktop";
        monitorInfo.position = sprout::Vector2Int::zero();
        monitorInfo.size = sprout::Vector2Int{static_cast<int>(desktop.size.x), static_cast<int>(desktop.size.y)};
        monitorInfo.scaleFactor = 1.0F;
        monitorInfo.primary = true;

        result.push_back(std::move(monitorInfo));
        return result;
    }

    [[nodiscard]] sprout::detail::DpiAwareness awareness() const override
    {
        return sprout::detail::DpiAwareness::Unaware;
    }

    bool setAwareness(sprout::detail::DpiAwareness /*awareness*/) override
    {
        return false;
    }
};

#endif

}  // namespace

namespace sprout::detail {

DisplayBackend& systemDisplayBackend()
{
    static SystemDisplayBackend backend;
    return backend;
}

Display::Display() noexcept
    : m_backend{&systemDisplayBackend()}
{
}

Display::Display(DisplayBackend& backend) noexcept
    : m_backend{&backend}
{
}

std::vector<MonitorInfo> Display::monitors() const
{
    return m_backend->monitors();
}

std::optional<MonitorInfo> Display::primaryMonitor() const
{
    const std::vector<MonitorInfo> monitorList = monitors();
    const auto primary = std::find_if(monitorList.begin(), monitorList.end(),
                                      [](const MonitorInfo& monitor) { return monitor.primary; });
    if (primary == monitorList.end()) {
        return std::nullopt;
    }
    return *primary;
}

std::optional<MonitorInfo> Display::monitor(int index) const
{
    const std::vector<MonitorInfo> monitorList = monitors();
    if (index < 0 || index >= static_cast<int>(monitorList.size())) {
        return std::nullopt;
    }
    return monitorList[static_cast<std::size_t>(index)];
}

DisplayInfo Display::displayInfo() const
{
    const std::vector<MonitorInfo> monitorList = monitors();

    DisplayInfo info;
    info.monitorCount = static_cast<int>(monitorList.size());
    info.highDpiEnabled = awareness() != DpiAwareness::Unaware;
    if (monitorList.empty()) {
        return info;
    }

    int left = monitorList.front().position.x;
    int top = monitorList.front().position.y;
    int right = monitorList.front().position.x + monitorList.front().size.x;
    int bottom = monitorList.front().position.y + monitorList.front().size.y;
    for (const MonitorInfo& monitor : monitorList) {
        left = std::min(left, monitor.position.x);
        top = std::min(top, monitor.position.y);
        right = std::max(right, monitor.position.x + monitor.size.x);
        bottom = std::max(bottom, monitor.position.y + monitor.size.y);
    }

    info.origin = Vector2Int{left, top};
    info.size = Vector2Int{right - left, bottom - top};
    return info;
}

DpiAwareness Display::awareness() const
{
    return m_backend->awareness();
}

bool Display::enableHighDpi() const
{
    return m_backend->setAwareness(DpiAwareness::PerMonitorAwareV2);
}

bool enableHighDpiSupport()
{
    return systemDisplayBackend().setAwareness(DpiAwareness::PerMonitorAwareV2);
}

float dpiScaleFromDpi(unsigned int dpi) noexcept
{
    if (dpi == 0U) {
        return 1.0F;
    }
    return static_cast<float>(dpi) / static_cast<float>(kReferenceDpi);
}

Vector2Int toPhysicalPixels(Vector2Int logicalPixels, float scaleFactor) noexcept
{
    if (scaleFactor <= 0.0F) {
        return logicalPixels;
    }
    return {static_cast<int>(std::lround(static_cast<double>(logicalPixels.x) * scaleFactor)),
            static_cast<int>(std::lround(static_cast<double>(logicalPixels.y) * scaleFactor))};
}

Vector2Int toLogicalPixels(Vector2Int physicalPixels, float scaleFactor) noexcept
{
    if (scaleFactor <= 0.0F) {
        return physicalPixels;
    }
    return {static_cast<int>(std::lround(static_cast<double>(physicalPixels.x) / scaleFactor)),
            static_cast<int>(std::lround(static_cast<double>(physicalPixels.y) / scaleFactor))};
}

}  // namespace sprout::detail
