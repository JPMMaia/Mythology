#include <cmath>

#include <catch2/catch.hpp>

#include <Maia/Renderer/Matrices.hpp>

namespace Maia::Renderer::Test
{
	SCENARIO("Create a view matrix")
	{
		GIVEN("Position = { 1.0f, 0.0f, 0.0f } and a rotation of 90 degrees around the Y axis")
		{
			Eigen::Vector3f const position{ 1.0f, 0.0f, 0.0f };

			float const half_angle = EIGEN_PI / 4.0f;
			Eigen::Quaternionf const rotation{ std::cos(half_angle), 0.0f, std::sin(half_angle), 0.0f };

			WHEN("The view matrix is calculated")
			{
				Eigen::Matrix4f const view_matrix = create_view_matrix(position, rotation);

				THEN("A Point { 2.0f, 0.0f, 0.0f, 1.0f } should be transformed to { 0.0f, 0.0f, 1.0f, 1.0f }")
				{
					Eigen::Vector4f const point{ 2.0f, 0.0f, 0.0f, 1.0f };

					Eigen::Vector4f const transformed_point = view_matrix * point;
					CHECK(Eigen::Vector4f{ 0.0f, 0.0f, 1.0f, 1.0f }.isApprox(transformed_point));
				}
			}
		}
	}

	SCENARIO("Create an orthographic projection matrix, assuming a right-handed coordinate system in which X goes right and Y goes down and the camera is looking at +Z")
	{
		GIVEN("Dimensions = { 4.0f, 4.0f, 10.0f }")
		{
			Eigen::Vector3f const dimensions{ 4.0f, 4.0f, 10.0f };

			WHEN("The orthographic projection matrix is calculated")
			{
				Eigen::Matrix4f const projection_matrix = create_orthographic_projection_matrix(dimensions);

				THEN("Point { 2.0f, 2.0f, 10.0f } should be transformed to { 1.0f, 1.0f, 1.0f }")
				{
					Eigen::Vector4f const point { 2.0f, 2.0f, 10.0f, 1.0f };
					
					Eigen::Vector4f const transformed_point = projection_matrix * point;
					CHECK(Eigen::Vector4f{ 1.0f, 1.0f, 1.0f, 1.0f } == transformed_point);
				}

				THEN("Point { -2.0f, -2.0f, 5.0f } should be transformed to { -1.0f, -1.0f, 0.5f }")
				{
					Eigen::Vector4f point{ -2.0f, -2.0f, 5.0f, 1.0f };

					Eigen::Vector4f const transformed_point = projection_matrix * point;
					CHECK(Eigen::Vector4f{ -1.0f, -1.0f, 0.5f, 1.0f } == transformed_point);
				}
			}
		}
	}

	SCENARIO("Create a perspective projection matrix, assuming a right-handed coordinate system in which X goes right and Y goes down")
	{
		GIVEN("Dimensions = { 16.0f, 9.0f } and Z-Range = { 1.0f, 21.0f }")
		{
			Eigen::Vector2f const dimensions{ 8.0f, 4.5f };
			Eigen::Vector2f const zRange{ 1.0f, 21.0f };

			WHEN("The perspective projection matrix is calculated")
			{
				Eigen::Matrix4f const projection_matrix = create_perspective_projection_matrix(dimensions, zRange);

				THEN("Point { 168.0f, 94.5f, 21.0f, 1.0f } should be transformed to { 21.0f, 21.0f, 21.0f, 21.0f }")
				{
					Eigen::Vector4f const point{ 168.0f, 94.5f, 21.0f, 1.0f };

					Eigen::Vector4f const transformed_point = projection_matrix * point;
					CHECK(Eigen::Vector4f{ 21.0f, 21.0f, 21.0f, 21.0f } == transformed_point);
				}

				THEN("Point { -8.0f, -4.5f, 1.0f, 1.0f } should be transformed to { -1.0f, -1.0f, 0.0f, 1.0f }")
				{
					Eigen::Vector4f const point{ -8.0f, -4.5f, 1.0f, 1.0f };

					Eigen::Vector4f const transformed_point = projection_matrix * point;
					CHECK(Eigen::Vector4f{ -1.0f, -1.0f, 0.0f, 1.0f } == transformed_point);
				}
			}
		}

		GIVEN("Vertical half angle of view = pi / 4, width by height ratio = 2.0f, Z-Range = { 1.0f, 21.0f }")
		{
			float const vertical_half_angle_of_view = EIGEN_PI / 4.0f;
			float const width_by_height_ratio = 2.0f;
			Eigen::Vector2f const zRange{ 1.0f, 21.0f };

			WHEN("The perspective projection matrix is calculated using these parameters")
			{
				Eigen::Matrix4f const projection_matrix = 
					create_perspective_projection_matrix(vertical_half_angle_of_view, width_by_height_ratio, zRange);

				THEN("The matrix should be the same as if created with Dimensions = { 2.0, 1.0f } and Z-Range = { 1.0f, 21.0f }")
				{
					Eigen::Matrix4f const expected_projection_matrix =
						create_perspective_projection_matrix({ 2.0f, 1.0f }, { 1.0f, 21.0f });

					CHECK(expected_projection_matrix == projection_matrix);
				}
			}
		}
	}
}
