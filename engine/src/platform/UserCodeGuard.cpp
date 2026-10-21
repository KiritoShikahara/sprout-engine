// UserCodeGuard.cpp - 非テンプレート実装

#include "platform/UserCodeGuard.hpp"

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <string>

namespace sprout::detail {

namespace {

#if defined(_WIN32)
std::string toHexString(uint32_t value) noexcept
{
    constexpr char kHexChars[] = "0123456789ABCDEF";
    std::string result(8, '0');
    for (int i = 7; i >= 0; --i) {
        result[i] = kHexChars[value & 0xF];
        value >>= 4;
    }
    return result;
}
#endif

}  // namespace

std::string sehExceptionDescription(uint32_t code) noexcept
{
#if defined(_WIN32)
    switch (code) {
    case EXCEPTION_ACCESS_VIOLATION:
        return "Access violation (EXCEPTION_ACCESS_VIOLATION)";
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
        return "Array bounds exceeded (EXCEPTION_ARRAY_BOUNDS_EXCEEDED)";
    case EXCEPTION_BREAKPOINT:
        return "Breakpoint (EXCEPTION_BREAKPOINT)";
    case EXCEPTION_DATATYPE_MISALIGNMENT:
        return "Data type misalignment (EXCEPTION_DATATYPE_MISALIGNMENT)";
    case EXCEPTION_FLT_DENORMAL_OPERAND:
        return "Float denormal operand (EXCEPTION_FLT_DENORMAL_OPERAND)";
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        return "Float divide by zero (EXCEPTION_FLT_DIVIDE_BY_ZERO)";
    case EXCEPTION_FLT_INEXACT_RESULT:
        return "Float inexact result (EXCEPTION_FLT_INEXACT_RESULT)";
    case EXCEPTION_FLT_INVALID_OPERATION:
        return "Float invalid operation (EXCEPTION_FLT_INVALID_OPERATION)";
    case EXCEPTION_FLT_OVERFLOW:
        return "Float overflow (EXCEPTION_FLT_OVERFLOW)";
    case EXCEPTION_FLT_STACK_CHECK:
        return "Float stack check (EXCEPTION_FLT_STACK_CHECK)";
    case EXCEPTION_FLT_UNDERFLOW:
        return "Float underflow (EXCEPTION_FLT_UNDERFLOW)";
    case EXCEPTION_ILLEGAL_INSTRUCTION:
        return "Illegal instruction (EXCEPTION_ILLEGAL_INSTRUCTION)";
    case EXCEPTION_IN_PAGE_ERROR:
        return "In-page error (EXCEPTION_IN_PAGE_ERROR)";
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
        return "Integer divide by zero (EXCEPTION_INT_DIVIDE_BY_ZERO)";
    case EXCEPTION_INT_OVERFLOW:
        return "Integer overflow (EXCEPTION_INT_OVERFLOW)";
    case EXCEPTION_INVALID_DISPOSITION:
        return "Invalid disposition (EXCEPTION_INVALID_DISPOSITION)";
    case EXCEPTION_NONCONTINUABLE_EXCEPTION:
        return "Non-continuable exception (EXCEPTION_NONCONTINUABLE_EXCEPTION)";
    case EXCEPTION_PRIV_INSTRUCTION:
        return "Privileged instruction (EXCEPTION_PRIV_INSTRUCTION)";
    case EXCEPTION_SINGLE_STEP:
        return "Single step (EXCEPTION_SINGLE_STEP)";
    case EXCEPTION_STACK_OVERFLOW:
        return "Stack overflow (EXCEPTION_STACK_OVERFLOW)";
    case CONTROL_C_EXIT:
        return "Control-C exit (CONTROL_C_EXIT)";
    default:
        return "Unknown SEH exception (0x" + toHexString(code) + ")";
    }
#else
    (void)code;
    return "SEH not supported on this platform";
#endif
}

}  // namespace sprout::detail