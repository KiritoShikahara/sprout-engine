#pragma once

#include <gtest/gtest.h>

namespace sprout::test {

/// ウィンドウを生成せずにエンジンを動かすテスト用フィクスチャ。
class HeadlessEngineFixture : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

}  // namespace sprout::test
