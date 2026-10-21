function(sprout_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE
            /W4
            /permissive-
            /WX
            /utf-8
            /Zc:__cplusplus)
    else()
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Werror)
    endif()
endfunction()
