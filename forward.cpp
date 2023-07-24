export module math:forward;

import <cstddef>;

export namespace math
{
	// Maximum distance between floats for them to be considered equal. Applies to vectors and
	// matrices.
	constexpr float epsilon = 0.001f;

	template<std::size_t N>
	class Vector;
	using Vec2 = Vector<2>;
	using Vec3 = Vector<3>;
	using Vec4 = Vector<4>;

	template<std::size_t N>
	constexpr Vector<N> operator-(Vector<N> lhs, const Vector<N>& rhs);

	template<std::size_t M, std::size_t N>
	class Matrix;
	using Mat22 = Matrix<2, 2>;
	using Mat2 = Mat22;
	using Mat33 = Matrix<3, 3>;
	using Mat3 = Mat33;
	using Mat44 = Matrix<4, 4>;
	using Mat4 = Mat44;

	constexpr Mat3 rotationZ(const float theta);
	inline Mat3 axesZ(const Vec3& axis);
}

export namespace graphics
{
	class Framebuffer;
}
