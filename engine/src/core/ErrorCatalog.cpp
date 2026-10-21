#include "core/ErrorCatalog.hpp"

#include <iterator>
#include <string>

namespace {

using sprout::ErrorCode;
using sprout::ErrorInfo;

constexpr ErrorInfo kErrorInfos[] = {
    {ErrorCode::Unknown, "原因が特定できないエラーが発生しました", "エンジン内部",
        "ログを保存して開発者に報告してください"},
    {ErrorCode::InvalidArgument, "引数の値が正しくありません", "API の呼び出し",
        "渡した値の範囲と型を確認してください"},
    {ErrorCode::InvalidState, "現在の状態ではこの操作を行えません", "エンジンの状態遷移",
        "初期化や再生の状態を確認してください"},
    {ErrorCode::NotFound, "必要なファイルまたはアセットが見つかりません", "アセットやファイルの読み込み",
        "名前・拡張子・置き場所を確認し、必要なら再インポートしてください"},
    {ErrorCode::AlreadyExists, "同じ名前または ID がすでに存在します", "オブジェクトやアセットの作成",
        "別の名前を付けるか既存のものを使用してください"},
    {ErrorCode::PermissionDenied, "ファイルまたはフォルダにアクセスする権限がありません", "ファイルの読み書き",
        "アクセス権を確認し、書き込み可能な場所を使ってください"},
    {ErrorCode::OutOfRange, "値が許可された範囲を超えています", "数値プロパティやインデックスの設定",
        "範囲内の値に修正してください"},
    {ErrorCode::NotSupported, "この環境またはファイル形式には対応していません", "機能の呼び出しやファイルの読み込み",
        "対応している形式や機能に変更してください"},
    {ErrorCode::Io, "ファイルの読み書きに失敗しました", "ファイル入出力",
        "空き容量・パス・他のアプリによる使用を確認してください"},
    {ErrorCode::Parse, "ファイルの内容を解釈できませんでした", "シーン・設定・セーブデータの読み込み",
        "ファイルが壊れていないか確認し、バックアップから復元してください"},
    {ErrorCode::OutOfMemory, "メモリを確保できませんでした", "エンジンのメモリ確保",
        "大きなアセットを減らすかアプリを再起動してください"},
    {ErrorCode::Timeout, "処理が制限時間内に終わりませんでした", "非同期処理や読み込み",
        "処理量を減らすか時間制限を延ばしてください"},
};

constexpr std::string_view kErrorNames[] = {
    "E_UNKNOWN",
    "E_INVALID_ARGUMENT",
    "E_INVALID_STATE",
    "E_NOT_FOUND",
    "E_ALREADY_EXISTS",
    "E_PERMISSION_DENIED",
    "E_OUT_OF_RANGE",
    "E_NOT_SUPPORTED",
    "E_IO",
    "E_PARSE",
    "E_OUT_OF_MEMORY",
    "E_TIMEOUT",
};

constexpr std::size_t kErrorInfoCount = std::size(kErrorInfos);

static_assert(std::size(kErrorInfos) == static_cast<std::size_t>(ErrorCode::Timeout) + 1,
    "ErrorCode を追加したら ErrorCatalog の表も更新してください");
static_assert(std::size(kErrorNames) == std::size(kErrorInfos), "エラー名の数が表と一致しません");

}  // namespace

namespace sprout {

const ErrorInfo& errorInfo(ErrorCode code) noexcept
{
    const auto index = static_cast<std::size_t>(code);
    if (index >= kErrorInfoCount) {
        return kErrorInfos[0];
    }
    return kErrorInfos[index];
}

std::string_view errorCodeName(ErrorCode code) noexcept
{
    const auto index = static_cast<std::size_t>(code);
    if (index >= kErrorInfoCount) {
        return kErrorNames[0];
    }
    return kErrorNames[index];
}

bool isKnownErrorCode(ErrorCode code) noexcept
{
    return static_cast<std::size_t>(code) < kErrorInfoCount;
}

std::string formatErrorMessage(ErrorCode code, std::string_view location)
{
    return formatErrorMessage(errorInfo(code), location);
}

std::string formatErrorMessage(const ErrorInfo& info, std::string_view location)
{
    std::string text;
    text.reserve(192);
    text += '[';
    text += errorCodeName(info.code);
    text += "] 何が: ";
    text += info.what;
    text += " / どこで: ";
    text += info.where;
    if (!location.empty()) {
        text += "（";
        text += location;
        text += "）";
    }
    text += " / どうすれば: ";
    text += info.howToFix;
    return text;
}

}  // namespace sprout
