find_program(SPROUT_CLANG_FORMAT_EXECUTABLE
    NAMES clang-format clang-format-19 clang-format-18 clang-format-17
    DOC "clang-format の実行ファイル")
find_program(SPROUT_CLANG_TIDY_EXECUTABLE
    NAMES clang-tidy clang-tidy-19 clang-tidy-18 clang-tidy-17
    DOC "clang-tidy の実行ファイル")
mark_as_advanced(SPROUT_CLANG_FORMAT_EXECUTABLE SPROUT_CLANG_TIDY_EXECUTABLE)

function(_sprout_collect_lint_sources out_var)
    set(files "")
    foreach(root IN ITEMS engine editor player cli tests samples)
        if(EXISTS "${SPROUT_ROOT_DIR}/${root}")
            file(GLOB_RECURSE found
                "${SPROUT_ROOT_DIR}/${root}/*.cpp"
                "${SPROUT_ROOT_DIR}/${root}/*.hpp"
                "${SPROUT_ROOT_DIR}/${root}/*.h")
            list(APPEND files ${found})
        endif()
    endforeach()
    set(${out_var} "${files}" PARENT_SCOPE)
endfunction()

function(sprout_add_lint_target)
    _sprout_collect_lint_sources(sources)
    set(script "${CMAKE_BINARY_DIR}/sprout_lint.cmake")

    file(WRITE "${script}" "set(SPROUT_LINT_SOURCES)\n")
    foreach(source IN LISTS sources)
        file(APPEND "${script}" "list(APPEND SPROUT_LINT_SOURCES \"${source}\")\n")
    endforeach()
    file(APPEND "${script}" "set(SPROUT_INCLUDE_DIR \"${SPROUT_ROOT_DIR}/engine/include\")\n")
    file(APPEND "${script}" "set(SPROUT_GENERATED_DIR \"${SPROUT_GENERATED_DIR}\")\n")
    file(APPEND "${script}" "set(SPROUT_CLANG_FORMAT \"${SPROUT_CLANG_FORMAT_EXECUTABLE}\")\n")
    file(APPEND "${script}" "set(SPROUT_CLANG_TIDY \"${SPROUT_CLANG_TIDY_EXECUTABLE}\")\n")

    file(APPEND "${script}" [=[
if(NOT SPROUT_LINT_SOURCES)
    message(STATUS "sprout_lint: 対象ソースがまだありません")
    return()
endif()

if(NOT SPROUT_CLANG_FORMAT AND NOT SPROUT_CLANG_TIDY)
    message(WARNING "sprout_lint: clang-format と clang-tidy が見つかりません")
    return()
endif()

foreach(source IN LISTS SPROUT_LINT_SOURCES)
    if(SPROUT_CLANG_FORMAT)
        execute_process(
            COMMAND "${SPROUT_CLANG_FORMAT}" --dry-run --Werror "${source}"
            RESULT_VARIABLE format_result)
        if(NOT format_result EQUAL 0)
            message(FATAL_ERROR "clang-format 違反: ${source}")
        endif()
    endif()
    if(SPROUT_CLANG_TIDY)
        execute_process(
            COMMAND "${SPROUT_CLANG_TIDY}" --quiet "${source}" --
                -std=c++20 "-I${SPROUT_INCLUDE_DIR}" "-I${SPROUT_GENERATED_DIR}"
            RESULT_VARIABLE tidy_result)
        if(NOT tidy_result EQUAL 0)
            message(FATAL_ERROR "clang-tidy 違反: ${source}")
        endif()
    endif()
endforeach()
]=])

    add_custom_target(sprout_lint
        COMMAND ${CMAKE_COMMAND} -P "${script}"
        COMMENT "sprout_lint: clang-format と clang-tidy を実行"
        VERBATIM)
endfunction()
