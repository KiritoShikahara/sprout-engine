#pragma once

#include "sprout/core/Result.hpp"

#include <cstdint>
#include <string>
#include <string_view>

namespace sprout::detail {

/// @brief ユーザーコード実行時の障害情報。
struct UserCodeFault {
    /// 障害の種類。
    enum class Type {
        /// C++ 例外（std::exception 派生）。
        CppException,
        /// 構造化例外（SEH：アクセス違反、スタックオーバーフロー等）。
        StructuredException,
        /// 不明な障害。
        Unknown,
    };

    /// 障害の種類。
    Type type{Type::Unknown};

    /// 例外メッセージ（C++ 例外の what()、または SEH 例外コードの説明）。
    std::string message;

    /// SEH 例外コード（EXCEPTION_ACCESS_VIOLATION 等）。C++ 例外の場合は 0。
    uint32_t exceptionCode{0};

    /// 障害が発生したアドレス。取得できない場合は nullptr。
    void* exceptionAddress{nullptr};
};

/// @brief ユーザーコードを保護して実行する。
///
/// Windows では SEH（__try/__except）で構造化例外を捕捉し、
/// 全環境で std::exception を捕捉する。障害時は UserCodeFault を含む Error を返す。
///
/// @tparam Fn 実行する呼び出し可能オブジェクトの型。
/// @param fn 実行する関数。引数なし、戻り値なしを想定。
/// @return 成功なら空の結果、失敗なら UserCodeFault を含む Error。
template <typename Fn>
[[nodiscard]] Result<void> runUserCode(Fn&& fn);

/// @brief SEH 例外コードから説明文字列を取得する（Windows のみ）。
/// @param code 例外コード。
/// @return 説明文字列。未知のコードなら "Unknown SEH exception"。
[[nodiscard]] std::string sehExceptionDescription(uint32_t code) noexcept;

}  // namespace sprout::detail

// 実装（テンプレートのためヘッダに記述）
#include "platform/UserCodeGuard.inl"