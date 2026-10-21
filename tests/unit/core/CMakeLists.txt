find_package(xxHash CONFIG REQUIRED)

sprout_add_test(sprout_unit_core_result
    SOURCES
        ResultTests.cpp
    LABELS unit
)

sprout_add_test(sprout_unit_core_log
    SOURCES
        LogTests.cpp
        ${SPROUT_ROOT_DIR}/engine/src/core/log/Log.cpp
        ${SPROUT_ROOT_DIR}/engine/src/core/log/ConsoleSink.cpp
        ${SPROUT_ROOT_DIR}/engine/src/core/log/FileSink.cpp
        ${SPROUT_ROOT_DIR}/engine/src/core/log/CallbackSink.cpp
    LABELS unit
)

sprout_add_test(sprout_unit_core_errorcatalog
    SOURCES
        ErrorCatalogTests.cpp
        ${SPROUT_ROOT_DIR}/engine/src/core/ErrorCatalog.cpp
    LABELS unit
)

target_include_directories(sprout_unit_core_errorcatalog PRIVATE
    "${SPROUT_ROOT_DIR}/engine/src")

sprout_add_test(sprout_unit_core_guid
    SOURCES
        GuidTests.cpp
        ${SPROUT_ROOT_DIR}/engine/src/core/Guid.cpp
    LABELS unit
)

target_include_directories(sprout_unit_core_guid PRIVATE
    "${SPROUT_ROOT_DIR}/engine/src")

sprout_add_test(sprout_unit_core_hash
    SOURCES
        HashTests.cpp
        ${SPROUT_ROOT_DIR}/engine/src/core/StringId.cpp
    DEPS
        xxHash::xxhash
    LABELS unit
)

target_include_directories(sprout_unit_core_hash PRIVATE
    "${SPROUT_ROOT_DIR}/engine/src")

sprout_add_test(sprout_unit_core_eventbus
    SOURCES
        EventBusTests.cpp
        ${SPROUT_ROOT_DIR}/engine/src/core/EventBus.cpp
    LABELS unit
)

sprout_add_test(sprout_unit_core_jobsystem
    SOURCES
        JobSystemTests.cpp
        ${SPROUT_ROOT_DIR}/engine/src/core/job/JobSystem.cpp
    LABELS unit
)

target_include_directories(sprout_unit_core_jobsystem PRIVATE
    "${SPROUT_ROOT_DIR}/engine/src")

sprout_add_test(sprout_unit_core_random
    SOURCES
        RandomTests.cpp
        ${SPROUT_ROOT_DIR}/engine/src/core/Random.cpp
    LABELS unit
)

sprout_add_test(sprout_unit_core_allocator
    SOURCES
        AllocatorTests.cpp
        ${SPROUT_ROOT_DIR}/engine/src/core/memory/FrameAllocator.cpp
    LABELS unit
)

target_include_directories(sprout_unit_core_allocator PRIVATE
    "${SPROUT_ROOT_DIR}/engine/src")

sprout_add_test(sprout_unit_core_filesystem
    SOURCES
        FileSystemTests.cpp
        ${SPROUT_ROOT_DIR}/engine/src/core/fs/FileSystem.cpp
        ${SPROUT_ROOT_DIR}/engine/src/core/fs/NativeMount.cpp
    LABELS unit
)

target_include_directories(sprout_unit_core_filesystem PRIVATE
    "${SPROUT_ROOT_DIR}/engine/src")
