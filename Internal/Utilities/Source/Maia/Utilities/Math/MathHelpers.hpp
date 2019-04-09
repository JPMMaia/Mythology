namespace Maia::Utilities::Math
{
	template <class Vector, class Scalar>
	inline Vector linear_interpolate(const Vector& a, const Vector& b, Scalar percentage)
	{
		return (Scalar(1) - percentage) * a + percentage * b;
	}
}
