find_package(SFML 3 COMPONENTS Window REQUIRED)

sprout_add_test(sprout_unit_platform_event
    SOURCES
        PlatformEventTests.cpp
        ${SPROUT_ROOT_DIR}/engine/src/platform/PlatformEventTranslator.cpp
    DEPS
        SFML::Window
    LABELS unit
)

target_include_directories(sprout_unit_platform_event PRIVATE
    "${SPROUT_ROOT_DIR}/engine/src")

sprout_add_test(sprout_unit_platform_clipboard
    SOURCES
        ClipboardTests.cpp
        ${SPROUT_ROOT_DIR}/engine/src/platform/Clipboard.cpp
    DEPS
        SFML::Window
    LABELS unit
)

target_include_directories(sprout_unit_platform_clipboard PRIVATE
    "${SPROUT_ROOT_DIR}/engine/src")

sprout_add_test(sprout_unit_platform_window
    SOURCES
        WindowTests.cpp
        ${SPROUT_ROOT_DIR}/engine/src/platform/Window.cpp
    DEPS
        SFML::Window
    LABELS unit
)

target_include_directories(sprout_unit_platform_window PRIVATE
    "${SPROUT_ROOT_DIR}/engine/src")

sprout_add_test(sprout_unit_platform_clock
    SOURCES
        ClockTests.cpp
        ${SPROUT_ROOT_DIR}/engine/src/platform/Clock.cpp
    LABELS unit
)

target_include_directories(sprout_unit_platform_clock PRIVATE
    "${SPROUT_ROOT_DIR}/engine/src")

sprout_add_test(sprout_unit_platform_usercodeguard
    SOURCES
        UserCodeGuardTests.cpp
        ${SPROUT_ROOT_DIR}/engine/src/platform/UserCodeGuard.cpp
    LABELS unit
)

target_include_directories(sprout_unit_platform_usercodeguard PRIVATE
    "${SPROUT_ROOT_DIR}/engine/src")

sprout_add_test(sprout_unit_platform_display
    SOURCES
        DisplayTests.cpp
        ${SPROUT_ROOT_DIR}/engine/src/platform/Display.cpp
    DEPS
        SFML::Window
    LABELS unit
)

target_include_directories(sprout_unit_platform_display PRIVATE
    "${SPROUT_ROOT_DIR}/engine/src")

if(WIN32)
    sprout_add_test(sprout_unit_platform_crashhandler
        SOURCES
            CrashHandlerTests.cpp
            ${SPROUT_ROOT_DIR}/engine/src/platform/CrashHandler.cpp
        DEPS
            dbghelp
        LABELS unit
    )

    target_include_directories(sprout_unit_platform_crashhandler PRIVATE
        "${SPROUT_ROOT_DIR}/engine/src")
    target_link_libraries(sprout_unit_platform_crashhandler PRIVATE dbghelp)
endif()
