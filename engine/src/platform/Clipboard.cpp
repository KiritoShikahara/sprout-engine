#include "platform/Clipboard.hpp"

#include <SFML/System/String.hpp>
#include <SFML/Window/Clipboard.hpp>

namespace {

std::string fromSfString(const sf::String& value)
{
    const sf::U8String utf8 = value.toUtf8();
    return {reinterpret_cast<const char*>(utf8.data()), utf8.size()};
}

sf::String toSfString(std::string_view text)
{
    return sf::String::fromUtf8(text.begin(), text.end());
}

class SystemClipboardBackend final : public sprout::detail::ClipboardBackend {
public:
    [[nodiscard]] std::string read() const override
    {
        return fromSfString(sf::Clipboard::getString());
    }

    void write(std::string_view text) override
    {
        sf::Clipboard::setString(toSfString(text));
    }
};

}  // namespace

namespace sprout::detail {

ClipboardBackend& systemClipboardBackend()
{
    static SystemClipboardBackend backend;
    return backend;
}

Clipboard::Clipboard() noexcept
    : m_backend{&systemClipboardBackend()}
{
}

Clipboard::Clipboard(ClipboardBackend& backend) noexcept
    : m_backend{&backend}
{
}

std::string Clipboard::text() const
{
    return m_backend->read();
}

void Clipboard::setText(std::string_view text)
{
    m_backend->write(text);
}

bool Clipboard::hasText() const
{
    return !text().empty();
}

void Clipboard::clear()
{
    setText({});
}

}  // namespace sprout::detail
