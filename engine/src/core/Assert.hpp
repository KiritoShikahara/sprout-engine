#pragma once

#include "core/ErrorCatalog.hpp"

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>

namespace sprout {

struct AssertFailure {
    const char* expression{};
    const char* file{};
    int line{0};
    std::string_view message{};
    ErrorCode code{ErrorCode::InvalidState};
};

using AssertHandler = void (*)(const AssertFailure&);

namespace detail {
inline std::atomic<AssertHandler> gAssertHandler{nullptr};
}  // namespace detail

inline void setAssertHandler(AssertHandler handler) noexcept
{
    detail::gAssertHandler.store(handler, std::memory_order_release);
}

inline AssertHandler assertHandler() noexcept
{
    return detail::gAssertHandler.load(std::memory_order_acquire);
}

inline std::string formatAssertFailure(const AssertFailure& failure)
{
    const std::string location =
        std::string{failure.file == nullptr ? "" : failure.file} + ":" + std::to_string(failure.line);
    std::string text = formatErrorMessage(failure.code, location);
    text += "\n条件: ";
    text += failure.expression == nullptr ? "" : failure.expression;
    if (!failure.message.empty()) {
        text += "\n詳細: ";
        text += failure.message;
    }
    return text;
}

inline void reportAssertFailure(const AssertFailure& failure)
{
    if (const AssertHandler handler = assertHandler(); handler != nullptr) {
        handler(failure);
        return;
    }

    const std::string text = formatAssertFailure(failure);
    std::fputs(text.c_str(), stderr);
    std::fputc('\n', stderr);
    std::fflush(stderr);
    std::abort();
}

}  // namespace sprout

#define SPROUT_ASSERT(condition)                                                                                      \
    do {                                                                                                              \
        if (!(condition)) {                                                                                           \
            ::sprout::reportAssertFailure(                                                                            \
                {#condition, __FILE__, __LINE__, "条件が満たされませんでした", ::sprout::ErrorCode::InvalidState});   \
        }                                                                                                             \
    } while (false)

#define SPROUT_ASSERT_MSG(condition, message)                                                                         \
    do {                                                                                                              \
        if (!(condition)) {                                                                                           \
            ::sprout::reportAssertFailure(                                                                            \
                {#condition, __FILE__, __LINE__, (message), ::sprout::ErrorCode::InvalidState});                      \
        }                                                                                                             \
    } while (false)
