#include "sprout/math/Matrix3.hpp"
#include "sprout/math/Vector2.hpp"

#include <cmath>
#include <gtest/gtest.h>
#include <numbers>
#include <optional>

namespace {

constexpr float kTolerance = 1.0e-5F;

void expectVectorNear(const sprout::Vector2& actual, const sprout::Vector2& expected, float tolerance = kTolerance)
{
    EXPECT_NEAR(actual.x, expected.x, tolerance);
    EXPECT_NEAR(actual.y, expected.y, tolerance);
}

void expectMatrixNear(const sprout::Matrix3& actual, const sprout::Matrix3& expected, float tolerance = kTolerance)
{
    const sprout::Vector2 probePoints[] = {{0.0F, 0.0F}, {1.0F, 0.0F}, {0.0F, 1.0F}, {2.0F, -3.0F}};
    for (const sprout::Vector2& point : probePoints) {
        expectVectorNear(actual.transformPoint(point), expected.transformPoint(point), tolerance);
    }
}

}  // namespace

TEST(Matrix3Tests, Identity_TransformPoint_ReturnsPoint)
{
    const sprout::Matrix3 matrix = sprout::Matrix3::identity();

    expectVectorNear(matrix.transformPoint({3.0F, -4.0F}), {3.0F, -4.0F});
}

TEST(Matrix3Tests, Identity_TransformDirection_ReturnsDirection)
{
    const sprout::Matrix3 matrix = sprout::Matrix3::identity();

    expectVectorNear(matrix.transformDirection({3.0F, -4.0F}), {3.0F, -4.0F});
}

TEST(Matrix3Tests, Translation_TransformPoint_AddsOffset)
{
    const sprout::Matrix3 matrix = sprout::Matrix3::translation({10.0F, -2.0F});

    expectVectorNear(matrix.transformPoint({3.0F, 4.0F}), {13.0F, 2.0F});
}

TEST(Matrix3Tests, Translation_TransformDirection_IgnoresOffset)
{
    const sprout::Matrix3 matrix = sprout::Matrix3::translation({10.0F, -2.0F});

    expectVectorNear(matrix.transformDirection({3.0F, 4.0F}), {3.0F, 4.0F});
}

TEST(Matrix3Tests, Scale_TransformPoint_ScalesComponents)
{
    const sprout::Matrix3 matrix = sprout::Matrix3::scale({2.0F, 3.0F});

    expectVectorNear(matrix.transformPoint({4.0F, -5.0F}), {8.0F, -15.0F});
}

TEST(Matrix3Tests, Scale_UniformOverload_ScalesBothComponents)
{
    const sprout::Matrix3 matrix = sprout::Matrix3::scale(2.5F);

    expectVectorNear(matrix.transformPoint({4.0F, -5.0F}), {10.0F, -12.5F});
}

TEST(Matrix3Tests, Rotation_QuarterTurn_TransformsRightToUp)
{
    const sprout::Matrix3 matrix = sprout::Matrix3::rotation(std::numbers::pi_v<float> / 2.0F);

    expectVectorNear(matrix.transformPoint(sprout::Vector2::right()), sprout::Vector2::up());
}

TEST(Matrix3Tests, Multiply_AppliesRightHandFirst)
{
    const sprout::Matrix3 translation = sprout::Matrix3::translation({10.0F, 0.0F});
    const sprout::Matrix3 scale = sprout::Matrix3::scale(2.0F);

    expectVectorNear((translation * scale).transformPoint({1.0F, 1.0F}), {12.0F, 2.0F});
    expectVectorNear((scale * translation).transformPoint({1.0F, 1.0F}), {22.0F, 2.0F});
}

TEST(Matrix3Tests, Multiply_Associative_ProducesSameTransform)
{
    const sprout::Matrix3 translation = sprout::Matrix3::translation({5.0F, -2.0F});
    const sprout::Matrix3 rotation = sprout::Matrix3::rotation(0.75F);
    const sprout::Matrix3 scale = sprout::Matrix3::scale({2.0F, 3.0F});

    expectMatrixNear((translation * rotation) * scale, translation * (rotation * scale));
}

TEST(Matrix3Tests, TransformPoint_CombinedTransform_AppliesScaleThenRotationThenTranslation)
{
    const sprout::Matrix3 matrix = sprout::Matrix3::translation({5.0F, -2.0F}) *
                                   sprout::Matrix3::rotation(std::numbers::pi_v<float> / 2.0F) *
                                   sprout::Matrix3::scale({2.0F, 3.0F});

    expectVectorNear(matrix.transformPoint({1.0F, 1.0F}), {2.0F, 0.0F});
}

TEST(Matrix3Tests, TransformDirection_CombinedTransform_IgnoresTranslation)
{
    const sprout::Matrix3 matrix = sprout::Matrix3::translation({5.0F, -2.0F}) *
                                   sprout::Matrix3::rotation(std::numbers::pi_v<float> / 2.0F) *
                                   sprout::Matrix3::scale({2.0F, 3.0F});

    expectVectorNear(matrix.transformDirection(sprout::Vector2::right()), {0.0F, 2.0F});
}

TEST(Matrix3Tests, Determinant_CombinedTransform_ReturnsLinearScaleProduct)
{
    const sprout::Matrix3 matrix = sprout::Matrix3::translation({5.0F, -2.0F}) *
                                   sprout::Matrix3::rotation(std::numbers::pi_v<float> / 2.0F) *
                                   sprout::Matrix3::scale({2.0F, 3.0F});

    EXPECT_NEAR(matrix.determinant(), 6.0F, kTolerance);
}

TEST(Matrix3Tests, Inverse_CombinedTransform_RestoresPoint)
{
    const sprout::Matrix3 matrix = sprout::Matrix3::translation({5.0F, -2.0F}) *
                                   sprout::Matrix3::rotation(0.75F) * sprout::Matrix3::scale({2.0F, 3.0F});
    const std::optional<sprout::Matrix3> inverse = matrix.inverse();

    ASSERT_TRUE(inverse.has_value());
    expectVectorNear(inverse->transformPoint(matrix.transformPoint({1.0F, 1.0F})), {1.0F, 1.0F});
}

TEST(Matrix3Tests, Inverse_MultipliedByOriginal_IsIdentity)
{
    const sprout::Matrix3 matrix = sprout::Matrix3::translation({5.0F, -2.0F}) *
                                   sprout::Matrix3::rotation(0.75F) * sprout::Matrix3::scale({2.0F, 3.0F});
    const std::optional<sprout::Matrix3> inverse = matrix.inverse();

    ASSERT_TRUE(inverse.has_value());
    expectMatrixNear(matrix * *inverse, sprout::Matrix3::identity());
    expectMatrixNear(*inverse * matrix, sprout::Matrix3::identity());
}

TEST(Matrix3Tests, Inverse_Identity_ReturnsIdentity)
{
    const std::optional<sprout::Matrix3> inverse = sprout::Matrix3::identity().inverse();

    ASSERT_TRUE(inverse.has_value());
    expectMatrixNear(*inverse, sprout::Matrix3::identity());
}

TEST(Matrix3Tests, Inverse_SingularScale_ReturnsNullopt)
{
    EXPECT_FALSE(sprout::Matrix3::scale({0.0F, 2.0F}).inverse().has_value());
    EXPECT_FALSE(sprout::Matrix3::scale(0.0F).inverse().has_value());
}

TEST(Matrix3Tests, Equality_SameTransform_IsTrue)
{
    const sprout::Matrix3 lhs = sprout::Matrix3::translation({1.0F, 2.0F});
    const sprout::Matrix3 rhs = sprout::Matrix3::translation({1.0F, 2.0F});

    EXPECT_TRUE(lhs == rhs);
}

TEST(Matrix3Tests, Equality_DifferentTransform_IsFalse)
{
    const sprout::Matrix3 lhs = sprout::Matrix3::translation({1.0F, 2.0F});
    const sprout::Matrix3 rhs = sprout::Matrix3::translation({2.0F, 1.0F});

    EXPECT_FALSE(lhs == rhs);
}
