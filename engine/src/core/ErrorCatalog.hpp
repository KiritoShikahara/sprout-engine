#pragma once

#include "sprout/core/Result.hpp"

#include <cstddef>
#include <string>
#include <string_view>

namespace sprout {

struct ErrorInfo {
    ErrorCode code{ErrorCode::Unknown};
    std::string_view what{};
    std::string_view where{};
    std::string_view howToFix{};
};

const ErrorInfo& errorInfo(ErrorCode code) noexcept;
std::string_view errorCodeName(ErrorCode code) noexcept;
bool isKnownErrorCode(ErrorCode code) noexcept;
std::string formatErrorMessage(ErrorCode code, std::string_view location = {});
std::string formatErrorMessage(const ErrorInfo& info, std::string_view location = {});

}  // namespace sprout
