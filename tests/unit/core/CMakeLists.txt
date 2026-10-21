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
