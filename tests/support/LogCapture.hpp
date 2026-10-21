#pragma once

#include <cstddef>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

namespace sprout::test {

/// ログシンクへ渡されたメッセージをスレッド安全に収集する。
class LogCapture {
public:
    void append(std::string_view message) {
        const std::lock_guard lock(m_mutex);
        m_messages.emplace_back(message);
    }

    void clear() {
        const std::lock_guard lock(m_mutex);
        m_messages.clear();
    }

    [[nodiscard]] bool contains(std::string_view text) const {
        const std::lock_guard lock(m_mutex);
        for (const std::string& message : m_messages) {
            if (message.find(text) != std::string::npos) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] std::size_t size() const {
        const std::lock_guard lock(m_mutex);
        return m_messages.size();
    }

    [[nodiscard]] std::vector<std::string> messages() const {
        const std::lock_guard lock(m_mutex);
        return m_messages;
    }

private:
    mutable std::mutex m_mutex;
    std::vector<std::string> m_messages;
};

}  // namespace sprout::test
