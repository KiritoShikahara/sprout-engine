cmake_minimum_required(VERSION 3.25)

project(SproutEngine
    VERSION 0.1.0
    DESCRIPTION "Sprout Engine - 学生向け 2D ゲームエンジン"
    LANGUAGES CXX
)

set(SPROUT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "Sprout Engine のリポジトリルート")
list(APPEND CMAKE_MODULE_PATH "${SPROUT_ROOT_DIR}/cmake")

option(SPROUT_BUILD_EDITOR "エディタ (SproutEditor) をビルドする" ON)
option(SPROUT_BUILD_TESTS "テストをビルドする" ON)
option(SPROUT_BUILD_SAMPLES "サンプルをビルドする" ON)
option(SPROUT_BUILD_CLI "CLI (sprout_cli) をビルドする" ON)
option(SPROUT_MONOLITHIC "エンジンとゲームモジュールを単一実行ファイルに静的結合する" OFF)
option(SPROUT_WITH_TRACY "Tracy プロファイラ連携を有効にする" OFF)
option(SPROUT_ENABLE_ASAN "AddressSanitizer を有効にする" OFF)
option(SPROUT_ENABLE_COVERAGE "OpenCppCoverage 用のデバッグ情報を有効にする" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

if(SPROUT_MONOLITHIC)
    add_compile_definitions(SPROUT_MONOLITHIC=1)
endif()
if(SPROUT_WITH_TRACY)
    add_compile_definitions(SPROUT_WITH_TRACY=1)
endif()

include(SproutWarnings)
include(SproutVersion)
include(SproutSanitizers)
include(SproutCoverage)
include(SproutModule)

sprout_generate_version()

if(SPROUT_ENABLE_ASAN)
    sprout_global_enable_asan()
endif()
if(SPROUT_ENABLE_COVERAGE)
    sprout_global_enable_coverage()
endif()

foreach(module_dir IN ITEMS engine editor player cli samples)
    if(EXISTS "${SPROUT_ROOT_DIR}/${module_dir}/CMakeLists.txt")
        add_subdirectory("${SPROUT_ROOT_DIR}/${module_dir}")
    endif()
endforeach()

if(SPROUT_BUILD_TESTS)
    enable_testing()
    add_subdirectory("${SPROUT_ROOT_DIR}/tests")
endif()

include(SproutLint)
sprout_add_lint_target()
