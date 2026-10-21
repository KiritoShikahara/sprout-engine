// UserCodeGuard.inl - runUserCode の実装（テンプレートのためヘッダからインクルード）

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <excpt.h>
#endif

#include <exception>
#include <string>

namespace sprout::detail {

namespace {

#if defined(_WIN32)

/// SEH 例外情報を UserCodeFault に詰める。
UserCodeFault makeSehFault(uint32_t code, EXCEPTION_POINTERS* pointers) noexcept
{
    UserCodeFault fault;
    fault.type = UserCodeFault::Type::StructuredException;
    fault.exceptionCode = code;
    fault.exceptionAddress = pointers ? pointers->ExceptionRecord->ExceptionAddress : nullptr;
    fault.message = sehExceptionDescription(code);
    return fault;
}

#endif  // _WIN32

/// C++ 例外から UserCodeFault を作る。
UserCodeFault makeCppFault(const std::exception& e) noexcept
{
    UserCodeFault fault;
    fault.type = UserCodeFault::Type::CppException;
    fault.exceptionCode = 0;
    fault.exceptionAddress = nullptr;
    fault.message = e.what();
    return fault;
}

/// 未知の例外から UserCodeFault を作る。
UserCodeFault makeUnknownFault() noexcept
{
    UserCodeFault fault;
    fault.type = UserCodeFault::Type::Unknown;
    fault.exceptionCode = 0;
    fault.exceptionAddress = nullptr;
    fault.message = "Unknown exception";
    return fault;
}

}  // namespace

template <typename Fn>
Result<void> runUserCode(Fn&& fn)
{
    try {
#if defined(_WIN32)
        __try {
            fn();
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            const uint32_t code = GetExceptionCode();
            EXCEPTION_POINTERS* pointers = GetExceptionInformation();
            UserCodeFault fault = makeSehFault(code, pointers);
            return Error{ErrorCode::Unknown, "User code fault: " + fault.message};
        }
#else
        fn();
#endif
    }
    catch (const std::exception& e) {
        UserCodeFault fault = makeCppFault(e);
        return Error{ErrorCode::Unknown, "User code fault: " + fault.message};
    }
    catch (...) {
        UserCodeFault fault = makeUnknownFault();
        return Error{ErrorCode::Unknown, "User code fault: " + fault.message};
    }
    return {};
}

}  // namespace sprout::detail