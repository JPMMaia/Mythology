#include <gtest/gtest.h>

#include <Maia/Utilities/Math/MathHelpers.hpp>

namespace Maia::Utilities::Test
{
	class Math_helpers_test : public ::testing::Test
	{
	};

	TEST_F(Math_helpers_test, LinearInterpolate_ShouldReturnFirstArgument_GivenPercentageZero)
	{
		const auto value = Math::linear_interpolate(0.0f, 2.0f, 0.0f);
		
		EXPECT_EQ(0.0f, value);
	}
	
	TEST_F(Math_helpers_test, LinearInterpolate_ShouldReturnSecondArgument_GivenPercentageOne)
	{
		const auto value = Math::linear_interpolate(0.0f, 2.0f, 1.0f);

		EXPECT_EQ(2.0f, value);
	}
	
	TEST_F(Math_helpers_test, LinearInterpolate_ShouldReturnHalf_GivenPercentageHalf)
	{
		const auto value = Math::linear_interpolate(0.0f, 2.0f, 0.5f);

		EXPECT_EQ(1.0f, value);
	}
}
