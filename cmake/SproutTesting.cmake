include(CTest)
include(GoogleTest)

function(sprout_add_test name)
    cmake_parse_arguments(ARG "" "" "SOURCES;LABELS;DEPS" ${ARGN})

    if(NOT ARG_SOURCES)
        message(FATAL_ERROR "sprout_add_test(${name}): SOURCES が指定されていません")
    endif()

    add_executable(${name} ${ARG_SOURCES})
    target_link_libraries(${name} PRIVATE GTest::gtest_main ${ARG_DEPS})
    target_include_directories(${name} PRIVATE
        "${SPROUT_ROOT_DIR}/tests/support"
        "${SPROUT_ROOT_DIR}/engine/include"
        "${SPROUT_GENERATED_DIR}")

    sprout_set_warnings(${name})
    if(SPROUT_ENABLE_ASAN)
        sprout_enable_asan(${name})
    endif()
    if(SPROUT_ENABLE_COVERAGE)
        sprout_enable_coverage(${name})
    endif()

    if(ARG_LABELS)
        set(labels ${ARG_LABELS})
    else()
        set(labels unit)
    endif()

    gtest_discover_tests(${name}
        PROPERTIES LABELS "${labels}"
        DISCOVERY_TIMEOUT 60)
endfunction()
