cmake_minimum_required(VERSION 3.25)

project(SproutEngine_Engine LANGUAGES CXX)

find_package(SFML 3 COMPONENTS system window graphics audio REQUIRED)

sprout_add_module(core
    SOURCES
        src/core/Assert.hpp
        src/core/ErrorCatalog.hpp
        src/core/ErrorCatalog.cpp
        src/core/Guid.hpp
        src/core/Guid.cpp
        src/core/Hash.hpp
        src/core/StringId.cpp
        src/core/EventBus.cpp
        src/core/job/JobSystem.cpp
        src/core/job/JobHandle.hpp
        src/core/memory/FrameAllocator.cpp
        src/core/memory/PoolAllocator.hpp
        src/core/log/Log.cpp
        src/core/log/ConsoleSink.cpp
        src/core/log/FileSink.cpp
        src/core/log/CallbackSink.cpp
        src/core/math/Matrix3.cpp
        src/core/math/Easing.cpp
        src/core/math/Color.cpp
        src/core/fs/FileSystem.cpp
        src/core/fs/NativeMount.cpp
        src/core/Random.cpp
    DEPS
)

sprout_add_module(platform
    SOURCES
        src/platform/Window.cpp
        src/platform/Window.hpp
        src/platform/Clipboard.cpp
        src/platform/Clipboard.hpp
        src/platform/Clock.cpp
        src/platform/Clock.hpp
        src/platform/Display.cpp
        src/platform/Display.hpp
        src/platform/PlatformEvent.hpp
        src/platform/PlatformEventTranslator.cpp
        src/platform/UserCodeGuard.cpp
        src/platform/UserCodeGuard.hpp
        src/platform/CrashHandler.cpp
        src/platform/CrashHandler.hpp
    DEPS
        core
)

if(WIN32)
    target_link_libraries(sprout_platform PRIVATE dbghelp)
endif()

sprout_add_module(reflection
    SOURCES
        src/reflection/TypeInfo.hpp
        src/reflection/TypeRegistry.cpp
    DEPS
        core
)

# 以降のモジュールは未実装のためコメントアウト
# sprout_add_module(serialization ...)
# sprout_add_module(scene ...)
# sprout_add_module(render ...)
# sprout_add_module(physics ...)
# sprout_add_module(animation ...)
# sprout_add_module(audio ...)
# sprout_add_module(input ...)
# sprout_add_module(ui ...)
# sprout_add_module(diagnostics ...)
# sprout_add_module(asset ...)