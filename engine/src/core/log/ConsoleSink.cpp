#include "sprout/core/Log.hpp"

#include <mutex>
#include <ostream>

namespace sprout {

ConsoleSink::ConsoleSink(std::ostream& stream)
    : m_stream(stream)
{
}

void ConsoleSink::write(const LogRecord& record)
{
    const std::lock_guard lock(m_mutex);
    m_stream << formatLogRecord(record) << '\n';
}

void ConsoleSink::flush()
{
    const std::lock_guard lock(m_mutex);
    m_stream.flush();
}

}  // namespace sprout
