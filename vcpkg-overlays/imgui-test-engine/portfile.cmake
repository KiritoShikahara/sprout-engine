vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO ocornut/imgui_test_engine
    REF 16b6efbb393fdef0dcac9b0794a8ee476606a152
    SHA512 c6740899bab7395d6a4d3f0ddcbcf486eae9226b3ee1142cacdfda6e950862fd25565f59f593e2d6d8a69e952d7405c7133c67bccab115708dfce830ff5aa08e
    HEAD_REF master)

file(WRITE "${SOURCE_PATH}/CMakeLists.txt" [=[
cmake_minimum_required(VERSION 3.25)
project(imgui_test_engine LANGUAGES CXX)

find_package(imgui CONFIG REQUIRED)

file(GLOB IMGUI_TEST_ENGINE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/imgui_test_engine/*.cpp")
file(GLOB IMGUI_TEST_ENGINE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/imgui_test_engine/*.h")

add_library(imgui_test_engine STATIC ${IMGUI_TEST_ENGINE_SOURCES})
target_include_directories(imgui_test_engine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imgui_test_engine>
    $<INSTALL_INTERFACE:include/imgui_test_engine>)
target_link_libraries(imgui_test_engine PUBLIC imgui::imgui)
target_compile_features(imgui_test_engine PUBLIC cxx_std_17)

install(TARGETS imgui_test_engine ARCHIVE DESTINATION lib)
install(FILES ${IMGUI_TEST_ENGINE_HEADERS} DESTINATION include/imgui_test_engine)
]=])

vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}")
vcpkg_cmake_install()
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/imgui_test_engine/LICENSE.txt")
