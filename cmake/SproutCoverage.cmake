find_program(SPROUT_OPENCPPCOVERAGE_EXECUTABLE
    NAMES OpenCppCoverage OpenCppCoverage.exe
    DOC "OpenCppCoverage の実行ファイル")
mark_as_advanced(SPROUT_OPENCPPCOVERAGE_EXECUTABLE)

function(sprout_enable_coverage target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /Z7)
        target_link_options(${target} PRIVATE /DEBUG /OPT:REF /OPT:ICF)
    else()
        target_compile_options(${target} PRIVATE -g --coverage)
        target_link_options(${target} PRIVATE --coverage)
    endif()
endfunction()

function(sprout_global_enable_coverage)
    if(MSVC)
        add_compile_options(/Z7)
        add_link_options(/DEBUG)
    else()
        add_compile_options(-g --coverage)
        add_link_options(--coverage)
    endif()
endfunction()
