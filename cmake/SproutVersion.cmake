set(SPROUT_VERSION "${PROJECT_VERSION}")
set(SPROUT_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(SPROUT_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(SPROUT_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

execute_process(
    COMMAND git rev-parse --short=12 HEAD
    WORKING_DIRECTORY "${SPROUT_ROOT_DIR}"
    OUTPUT_VARIABLE SPROUT_GIT_HASH
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE SPROUT_GIT_RESULT)

if(NOT SPROUT_GIT_RESULT EQUAL 0 OR NOT SPROUT_GIT_HASH)
    set(SPROUT_GIT_HASH "unknown")
else()
    execute_process(
        COMMAND git status --porcelain
        WORKING_DIRECTORY "${SPROUT_ROOT_DIR}"
        OUTPUT_VARIABLE SPROUT_GIT_STATUS
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(SPROUT_GIT_STATUS)
        string(APPEND SPROUT_GIT_HASH "-dirty")
    endif()
endif()

set(SPROUT_GENERATED_DIR "${CMAKE_BINARY_DIR}/generated" CACHE INTERNAL "生成ヘッダの出力先")

function(sprout_generate_version)
    set(output_dir "${SPROUT_GENERATED_DIR}/sprout/core")
    file(MAKE_DIRECTORY "${output_dir}")
    configure_file(
        "${SPROUT_ROOT_DIR}/engine/include/sprout/core/Version.hpp.in"
        "${output_dir}/Version.hpp"
        @ONLY)
endfunction()
