#include "platform/Display.hpp"

#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace {

using sprout::Vector2Int;
using sprout::detail::Display;
using sprout::detail::DisplayBackend;
using sprout::detail::DpiAwareness;
using sprout::detail::MonitorInfo;

class FakeDisplayBackend final : public DisplayBackend {
public:
    [[nodiscard]] std::vector<MonitorInfo> monitors() const override
    {
        return m_monitors;
    }

    [[nodiscard]] DpiAwareness awareness() const override
    {
        return m_awareness;
    }

    bool setAwareness(DpiAwareness awareness) override
    {
        m_requested = awareness;
        if (!m_accepts) {
            return false;
        }
        m_awareness = awareness;
        return true;
    }

    void addMonitor(MonitorInfo monitor)
    {
        m_monitors.push_back(std::move(monitor));
    }

    void setAccepts(bool accepts)
    {
        m_accepts = accepts;
    }

    [[nodiscard]] std::optional<DpiAwareness> requested() const
    {
        return m_requested;
    }

private:
    std::vector<MonitorInfo> m_monitors;
    DpiAwareness m_awareness{DpiAwareness::Unaware};
    std::optional<DpiAwareness> m_requested;
    bool m_accepts{true};
};

MonitorInfo makeMonitor(int index, bool primary)
{
    MonitorInfo monitor;
    monitor.index = index;
    monitor.name = "Fake Monitor " + std::to_string(index);
    monitor.position = Vector2Int::zero();
    monitor.size = Vector2Int{1920, 1080};
    monitor.scaleFactor = 1.0F;
    monitor.primary = primary;
    return monitor;
}

TEST(DisplayTests, DpiScale_ReferenceDpi_ReturnsOne)
{
    EXPECT_FLOAT_EQ(sprout::detail::dpiScaleFromDpi(96), 1.0F);
}

TEST(DisplayTests, DpiScale_HigherDpi_ReturnsScaledValue)
{
    EXPECT_FLOAT_EQ(sprout::detail::dpiScaleFromDpi(120), 1.25F);
    EXPECT_FLOAT_EQ(sprout::detail::dpiScaleFromDpi(144), 1.5F);
    EXPECT_FLOAT_EQ(sprout::detail::dpiScaleFromDpi(192), 2.0F);
}

TEST(DisplayTests, DpiScale_ZeroDpi_ReturnsOne)
{
    EXPECT_FLOAT_EQ(sprout::detail::dpiScaleFromDpi(0), 1.0F);
}

TEST(DisplayTests, ToPhysicalPixels_ScalesLogicalPixels)
{
    const Vector2Int logical{100, 50};
    const Vector2Int expected{150, 75};

    EXPECT_EQ(sprout::detail::toPhysicalPixels(logical, 1.5F), expected);
}

TEST(DisplayTests, ToPhysicalPixels_NonPositiveScale_ReturnsUnchanged)
{
    const Vector2Int logical{100, 50};

    EXPECT_EQ(sprout::detail::toPhysicalPixels(logical, 0.0F), logical);
    EXPECT_EQ(sprout::detail::toPhysicalPixels(logical, -1.0F), logical);
}

TEST(DisplayTests, ToLogicalPixels_ScalesPhysicalPixels)
{
    const Vector2Int physical{150, 75};
    const Vector2Int expected{100, 50};

    EXPECT_EQ(sprout::detail::toLogicalPixels(physical, 1.5F), expected);
}

TEST(DisplayTests, ToLogicalPixels_NonPositiveScale_ReturnsUnchanged)
{
    const Vector2Int physical{150, 75};

    EXPECT_EQ(sprout::detail::toLogicalPixels(physical, 0.0F), physical);
    EXPECT_EQ(sprout::detail::toLogicalPixels(physical, -1.0F), physical);
}

TEST(DisplayTests, Monitors_ReturnsBackendMonitors)
{
    FakeDisplayBackend backend;
    backend.addMonitor(makeMonitor(0, true));
    backend.addMonitor(makeMonitor(1, false));
    const Display display{backend};

    const std::vector<MonitorInfo> monitors = display.monitors();

    ASSERT_EQ(monitors.size(), 2U);
    EXPECT_EQ(monitors[0].index, 0);
    EXPECT_TRUE(monitors[0].primary);
    EXPECT_EQ(monitors[1].index, 1);
    EXPECT_FALSE(monitors[1].primary);
}

TEST(DisplayTests, PrimaryMonitor_ReturnsPrimaryMonitor)
{
    FakeDisplayBackend backend;
    backend.addMonitor(makeMonitor(0, false));
    backend.addMonitor(makeMonitor(1, true));
    const Display display{backend};

    const std::optional<MonitorInfo> primary = display.primaryMonitor();

    ASSERT_TRUE(primary.has_value());
    EXPECT_EQ(primary->index, 1);
}

TEST(DisplayTests, PrimaryMonitor_NoPrimary_ReturnsNullopt)
{
    FakeDisplayBackend backend;
    backend.addMonitor(makeMonitor(0, false));
    const Display display{backend};

    EXPECT_FALSE(display.primaryMonitor().has_value());
}

TEST(DisplayTests, Monitor_ValidIndex_ReturnsMonitor)
{
    FakeDisplayBackend backend;
    backend.addMonitor(makeMonitor(0, true));
    backend.addMonitor(makeMonitor(1, false));
    const Display display{backend};

    const std::optional<MonitorInfo> monitor = display.monitor(1);

    ASSERT_TRUE(monitor.has_value());
    EXPECT_EQ(monitor->index, 1);
}

TEST(DisplayTests, Monitor_OutOfRangeIndex_ReturnsNullopt)
{
    FakeDisplayBackend backend;
    backend.addMonitor(makeMonitor(0, true));
    const Display display{backend};

    EXPECT_FALSE(display.monitor(1).has_value());
    EXPECT_FALSE(display.monitor(-1).has_value());
}

TEST(DisplayTests, DisplayInfo_SingleMonitor_CoversMonitor)
{
    FakeDisplayBackend backend;
    MonitorInfo monitor = makeMonitor(0, true);
    monitor.position = Vector2Int{100, 50};
    monitor.size = Vector2Int{1920, 1080};
    backend.addMonitor(monitor);
    const Display display{backend};

    const sprout::detail::DisplayInfo info = display.displayInfo();

    EXPECT_EQ(info.monitorCount, 1);
    EXPECT_EQ(info.origin, (Vector2Int{100, 50}));
    EXPECT_EQ(info.size, (Vector2Int{1920, 1080}));
}

TEST(DisplayTests, DisplayInfo_MultipleMonitors_CoversAll)
{
    FakeDisplayBackend backend;
    MonitorInfo left = makeMonitor(0, true);
    left.position = Vector2Int{0, 0};
    left.size = Vector2Int{1920, 1080};
    backend.addMonitor(left);

    MonitorInfo right = makeMonitor(1, false);
    right.position = Vector2Int{1920, -200};
    right.size = Vector2Int{2560, 1440};
    backend.addMonitor(right);
    const Display display{backend};

    const sprout::detail::DisplayInfo info = display.displayInfo();

    EXPECT_EQ(info.monitorCount, 2);
    EXPECT_EQ(info.origin, (Vector2Int{0, -200}));
    EXPECT_EQ(info.size, (Vector2Int{4480, 1440}));
}

TEST(DisplayTests, DisplayInfo_NoMonitors_ReportsEmpty)
{
    FakeDisplayBackend backend;
    const Display display{backend};

    const sprout::detail::DisplayInfo info = display.displayInfo();

    EXPECT_EQ(info.monitorCount, 0);
    EXPECT_EQ(info.origin, Vector2Int::zero());
    EXPECT_EQ(info.size, Vector2Int::zero());
}

TEST(DisplayTests, DisplayInfo_NotDpiAware_ReportsHighDpiDisabled)
{
    FakeDisplayBackend backend;
    backend.addMonitor(makeMonitor(0, true));
    const Display display{backend};

    EXPECT_FALSE(display.displayInfo().highDpiEnabled);
}

TEST(DisplayTests, DisplayInfo_DpiAware_ReportsHighDpiEnabled)
{
    FakeDisplayBackend backend;
    backend.addMonitor(makeMonitor(0, true));
    backend.setAwareness(DpiAwareness::PerMonitorAwareV2);
    const Display display{backend};

    EXPECT_TRUE(display.displayInfo().highDpiEnabled);
}

TEST(DisplayTests, Awareness_ReturnsBackendAwareness)
{
    FakeDisplayBackend backend;
    backend.setAwareness(DpiAwareness::SystemAware);
    const Display display{backend};

    EXPECT_EQ(display.awareness(), DpiAwareness::SystemAware);
}

TEST(DisplayTests, EnableHighDpi_RequestsPerMonitorAwareV2)
{
    FakeDisplayBackend backend;
    const Display display{backend};

    EXPECT_TRUE(display.enableHighDpi());

    const std::optional<DpiAwareness> requested = backend.requested();
    ASSERT_TRUE(requested.has_value());
    EXPECT_EQ(*requested, DpiAwareness::PerMonitorAwareV2);
}

TEST(DisplayTests, EnableHighDpi_BackendRejects_ReturnsFalse)
{
    FakeDisplayBackend backend;
    backend.setAccepts(false);
    const Display display{backend};

    EXPECT_FALSE(display.enableHighDpi());
}

TEST(DisplayTests, SystemDisplay_Monitors_AreConsistent)
{
    const Display display;

    for (const MonitorInfo& monitor : display.monitors()) {
        EXPECT_GT(monitor.size.x, 0);
        EXPECT_GT(monitor.size.y, 0);
        EXPECT_GT(monitor.scaleFactor, 0.0F);
    }
}

TEST(DisplayTests, SystemDisplay_EnableHighDpi_EnablesAwareness)
{
    const Display display;

    EXPECT_TRUE(display.enableHighDpi());
    EXPECT_NE(display.awareness(), DpiAwareness::Unaware);
}

}  // namespace
