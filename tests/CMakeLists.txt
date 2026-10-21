enable_testing()

find_package(GTest CONFIG REQUIRED)
include("${SPROUT_ROOT_DIR}/cmake/SproutTesting.cmake")

foreach(test_dir IN ITEMS unit integration determinism editor_e2e perf)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${test_dir}/CMakeLists.txt")
        add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/${test_dir}")
    endif()
endforeach()
