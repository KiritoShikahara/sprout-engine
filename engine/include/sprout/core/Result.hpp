#pragma once

#include <cassert>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

namespace sprout {

/// @brief 失敗の種類。
///
/// 失敗の大分類を表す。具体的な説明は Error::message() が持つ。
enum class ErrorCode {
    Unknown = 0,
    InvalidArgument,
    InvalidState,
    NotFound,
    AlreadyExists,
    PermissionDenied,
    OutOfRange,
    NotSupported,
    Io,
    Parse,
    OutOfMemory,
    Timeout,
};

/// @brief 失敗の理由（コードとメッセージ）。
///
/// @code
/// const sprout::Error error{sprout::ErrorCode::NotFound, "Assets/player.png が見つかりません"};
/// const sprout::ErrorCode code = error.code();
/// @endcode
class Error {
public:
    /// 未知のエラーを生成する。
    constexpr Error() noexcept = default;

    /// コードのみを指定して生成する。メッセージは空になる。
    /// @param code 失敗の種類。
    constexpr explicit Error(ErrorCode code) noexcept
        : m_code{code}
    {
    }

    /// コードとメッセージを指定して生成する。
    /// @param code 失敗の種類。
    /// @param message 失敗の説明。
    Error(ErrorCode code, std::string message)
        : m_code{code}
        , m_message{std::move(message)}
    {
    }

    /// 失敗の種類を返す。
    /// @return エラーコード。
    [[nodiscard]] constexpr ErrorCode code() const noexcept
    {
        return m_code;
    }

    /// 失敗の説明を返す。
    /// @return メッセージへの参照。
    [[nodiscard]] const std::string& message() const noexcept
    {
        return m_message;
    }

private:
    ErrorCode m_code{ErrorCode::Unknown};
    std::string m_message;
};

/// @brief 成功値または Error を持つ結果型。
///
/// 公開 API は例外を投げないため、失敗はこの型で返す。
/// `value()` / `error()` は isOk() が期待する状態であることを前提とする。
///
/// @code
/// sprout::Result<int> parse(bool valid)
/// {
///     if (!valid) {
///         return sprout::Error{sprout::ErrorCode::Parse, "数値ではありません"};
///     }
///     return 42;
/// }
///
/// const sprout::Result<int> result = parse(true);
/// if (result.isOk()) {
///     const int value = result.value();
/// }
/// @endcode
/// @tparam T 成功時の値の型。void は専用の特殊化を使う。
template <typename T>
class Result {
    static_assert(!std::is_void_v<T>, "Result<void> は専用の特殊化を使用してください");
    static_assert(!std::is_reference_v<T>, "Result<T> は参照型を保持できません");

public:
    /// 成功値を保持して生成する。
    /// @param value 成功時の値。
    Result(T value)
        : m_value{std::move(value)}
    {
    }

    /// 失敗を保持して生成する。
    /// @param error 失敗の理由。
    Result(Error error)
        : m_error{std::move(error)}
    {
    }

    /// 成功しているか判定する。
    /// @return 成功値を持っていれば true。
    [[nodiscard]] bool isOk() const noexcept
    {
        return m_value.has_value();
    }

    /// 成功値を返す。
    /// @return 成功値への参照。isOk() が false の場合は未定義動作。
    [[nodiscard]] const T& value() const& noexcept
    {
        assert(isOk());
        return *m_value;
    }

    /// 成功値を返す。
    /// @return 成功値への参照。isOk() が false の場合は未定義動作。
    [[nodiscard]] T& value() & noexcept
    {
        assert(isOk());
        return *m_value;
    }

    /// 成功値を移動して返す。
    /// @return 成功値。isOk() が false の場合は未定義動作。
    [[nodiscard]] T&& value() && noexcept
    {
        assert(isOk());
        return std::move(*m_value);
    }

    /// 失敗の理由を返す。
    /// @return 失敗の理由への参照。isOk() が true の場合は未定義動作。
    [[nodiscard]] const Error& error() const noexcept
    {
        assert(!isOk());
        return m_error;
    }

private:
    std::optional<T> m_value;
    Error m_error;
};

/// @brief 成功値を持たない Result の特殊化。
template <>
class Result<void> {
public:
    /// 成功を生成する。
    Result() noexcept = default;

    /// 失敗を保持して生成する。
    /// @param error 失敗の理由。
    Result(Error error)
        : m_error{std::move(error)}
        , m_ok{false}
    {
    }

    /// 成功しているか判定する。
    /// @return 失敗を保持していなければ true。
    [[nodiscard]] bool isOk() const noexcept
    {
        return m_ok;
    }

    /// 失敗の理由を返す。
    /// @return 失敗の理由への参照。isOk() が true の場合は未定義動作。
    [[nodiscard]] const Error& error() const noexcept
    {
        assert(!m_ok);
        return m_error;
    }

private:
    Error m_error;
    bool m_ok{true};
};

}  // namespace sprout
