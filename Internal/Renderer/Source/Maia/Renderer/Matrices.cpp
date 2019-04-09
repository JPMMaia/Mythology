#include <cassert>

#include "Matrices.hpp"

namespace Maia::Renderer
{
	Eigen::Matrix4f create_view_matrix(Eigen::Vector3f const& position, Eigen::Quaternionf const& rotation)
	{
		assert(rotation.isApprox(rotation.normalized()));

		Eigen::Matrix4f value;
		value.topLeftCorner<3, 3>() = rotation.conjugate().toRotationMatrix();
		value.topRightCorner<3, 1>() = value.topLeftCorner<3, 3>() * (-position);
		value.bottomLeftCorner<1, 4>() = Eigen::Vector4f{ 0.0f, 0.0f, 0.0f, 1.0f };
		return value;
	}

	Eigen::Matrix4f create_orthographic_projection_matrix(float const horizontal_magnification, float const vertical_magnification, float const near_z, float const far_z)
	{
		float const range_z = far_z - near_z;

		Eigen::Matrix4f value;
		value << 
			1.0f / horizontal_magnification, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f / vertical_magnification, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f / range_z, -near_z / range_z,
			0.0f, 0.0f, 0.0f, 1.0f;

		return value;
	}

	Eigen::Matrix4f create_infinite_perspective_projection_matrix(float const aspect_ratio, float const vertical_field_of_view, float const near_z)
	{
		float const half_height = std::tan(0.5f * vertical_field_of_view);
		
		// TODO check
		Eigen::Matrix4f value;
		value <<
			1.0f / (aspect_ratio * half_height), 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f / half_height, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, -near_z,
			0.0f, 0.0f, 1.0f, 0.0f;

		return value;
	}

	Eigen::Matrix4f create_finite_perspective_projection_matrix(float const aspect_ratio, float const vertical_field_of_view, float const near_z, float const far_z)
	{
		float const half_height = std::tan(0.5f * vertical_field_of_view);

		Eigen::Matrix4f value;
		value <<
			1.0f / (aspect_ratio * half_height), 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f / half_height, 0.0f, 0.0f,
			0.0f, 0.0f, far_z / (far_z - near_z), - far_z * near_z / (far_z - near_z),
			0.0f, 0.0f, 1.0f, 0.0f;

		return value;
	}
}
