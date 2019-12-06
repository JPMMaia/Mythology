#ifndef MAIA_RENDERER_MATRICES_H_INCLUDED
#define MAIA_RENDERER_MATRICES_H_INCLUDED

#include <Eigen/Geometry>

namespace Maia::Renderer
{
	Eigen::Matrix4f create_view_matrix(Eigen::Vector3f const& position, Eigen::Quaternionf const& rotation);

	Eigen::Matrix4f create_orthographic_projection_matrix(float horizontal_magnification, float vertical_magnification, float near_z, float far_z);

	Eigen::Matrix4f create_infinite_perspective_projection_matrix(float aspect_ratio, float vertical_field_of_view, float near_z);
	Eigen::Matrix4f create_finite_perspective_projection_matrix(float aspect_ratio, float vertical_field_of_view, float near_z, float far_z);
}

#endif
