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

sprout_add_test(sprout_unit_math_color
    SOURCES
        ColorTests.cpp
        ${SPROUT_ROOT_DIR}/engine/src/core/math/Color.cpp
    LABELS unit
)

sprout_add_test(sprout_unit_math_easing
    SOURCES
        EasingTests.cpp
        ${SPROUT_ROOT_DIR}/engine/src/core/math/Easing.cpp
    LABELS unit
)

sprout_add_test(sprout_unit_math_mathutil
    SOURCES
        MathUtilTests.cpp
    LABELS unit
)

sprout_add_test(sprout_unit_math_angle
    SOURCES
        AngleTests.cpp
    LABELS unit
)
