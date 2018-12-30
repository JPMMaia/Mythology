#ifndef MAIA_MYTHOLOGY_PASSDATA_H_INCLUDED
#define MAIA_MYTHOLOGY_PASSDATA_H_INCLUDED

#include <Eigen/Core>

namespace Maia::Mythology
{
	struct Pass_data
	{
		Eigen::Matrix4f view_matrix;
		Eigen::Matrix4f projection_matrix;
	};
}

#endif