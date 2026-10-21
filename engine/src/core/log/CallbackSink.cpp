#include "sprout/core/Log.hpp"

#include <mutex>
#include <utility>

namespace sprout {

CallbackSink::CallbackSink(Callback callback)
    : m_callback(std::move(callback))
{
}

void CallbackSink::write(const LogRecord& record)
{
    const std::lock_guard lock(m_mutex);
    if (m_callback) {
        m_callback(record);
    }
}

}  // namespace sprout
