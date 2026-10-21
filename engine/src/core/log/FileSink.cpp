#include "sprout/core/Log.hpp"

#include <filesystem>
#include <fstream>
#include <mutex>

namespace sprout {

FileSink::FileSink(std::filesystem::path path, bool append)
    : m_stream(path, std::ios::binary | (append ? std::ios::app : std::ios::trunc))
{
}

void FileSink::write(const LogRecord& record)
{
    const std::lock_guard lock(m_mutex);
    if (!m_stream.is_open()) {
        return;
    }
    m_stream << formatLogRecord(record) << '\n';
    m_stream.flush();
}

void FileSink::flush()
{
    const std::lock_guard lock(m_mutex);
    if (m_stream.is_open()) {
        m_stream.flush();
    }
}

bool FileSink::isOpen() const
{
    const std::lock_guard lock(m_mutex);
    return m_stream.is_open();
}

}  // namespace sprout
