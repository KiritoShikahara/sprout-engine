sprout_add_test(sprout_unit_math_vector2
    SOURCES
        Vector2Tests.cpp
    LABELS unit
)

sprout_add_test(sprout_unit_math_matrix3
    SOURCES
        Matrix3Tests.cpp
        ${SPROUT_ROOT_DIR}/engine/src/core/math/Matrix3.cpp
    LABELS unit
)

sprout_add_test(sprout_unit_math_shapes
    SOURCES
        ShapeTests.cpp
    LABELS unit
)
