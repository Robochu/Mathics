/* A vector class used for math.
 * Essentially just a thin wrapper around std::array with the required functionality.
 */

export module math:Vector;

import :forward;

import <array>;
import <numeric>;
import <algorithm>;
import <iostream>;
import <type_traits>;
import <cstddef>;
import <cmath>;

export namespace math
{
	template<std::size_t N>
	class Vector
	{
		// The vector is neither a row vector, nor a column vector; it depends on the context.
		std::array<float, N> components;

	public:
		friend class Vector;

		constexpr Vector() = default;
		explicit constexpr Vector(const float component)
		{
			components.fill(component);
		}
		constexpr Vector(const std::array<float, N>& components) : components(components) {}
		template<typename... T, std::enable_if_t<
			std::conjunction_v<std::is_same<T, float>...> && sizeof...(T) == N
		>* = nullptr>
		constexpr Vector(const T... components) : components({components...}) {}
		template<std::size_t M, std::enable_if_t<M < N>* = nullptr>
		explicit constexpr Vector(const Vector<M>& vector)
		{
			std::copy(vector.components.begin(), vector.components.end(), components.begin());
			std::fill(components.begin() + M, components.end(), 0.0f);
		}
		template<std::size_t M, std::enable_if_t<(M > N)>* = nullptr>
		explicit constexpr Vector(const Vector<M>& vector) :
			components(vector.subvector<N>().components) {}
		constexpr Vector(const Matrix<1, N>& matrix) : Vector(matrix[0]) {}
		template<std::size_t D = N, std::enable_if_t<D != 1>* = nullptr>
		constexpr Vector(const Matrix<N, 1>& matrix) : Vector(matrix.getColumn(0)) {}

		constexpr float& operator[](const std::size_t i)
		{
			return components[i];
		}
		constexpr const float& operator[](const std::size_t i) const
		{
			return components[i];
		}

		template<std::size_t I, std::size_t J, std::enable_if_t<I <= J && J <= N>* = nullptr>
		constexpr Vector<J - I> subvector() const
		{
			Vector<J - I> result;
			std::copy(components.begin() + I, components.begin() + J, result.components.begin());
			return result;
		}
		template<std::size_t I, std::enable_if_t<I <= N>* = nullptr>
		constexpr Vector<I> subvector() const
		{
			return subvector<0, I>();
		}

		constexpr Vector<N>& operator+=(const Vector<N>& rhs)
		{
			for (std::size_t i = 0; i < N; i++)
			{
				components[i] += rhs[i];
			}
			return *this;
		}
		constexpr Vector<N>& operator-=(const Vector<N>& rhs)
		{
			for (std::size_t i = 0; i < N; i++)
			{
				components[i] -= rhs[i];
			}
			return *this;
		}
		constexpr Vector<N>& operator*=(const float rhs)
		{
			for (float& component : components)
			{
				component *= rhs;
			}
			return *this;
		}
		constexpr Vector<N>& operator/=(const float rhs)
		{
			for (float& component : components)
			{
				component /= rhs;
			}
			return *this;
		}

		constexpr float dot(const Vector<N>& rhs) const
		{
			float sum = 0.0f;
			for (std::size_t i = 0; i < N; i++)
			{
				sum += components[i] * rhs[i];
			}
			return sum;
		}
		template<std::size_t D = N, std::enable_if_t<D == 2>* = nullptr>
		constexpr float cross(const Vec2& rhs) const
		{
			// This isn't really the cross product since the cross product is not defined for 2D
			// vectors. However, I couldn't find a better name. The best alternative I could find
			// is 'the wedge product of two one-forms followed by the Hodge dual operation', found
			// at https://www.shorturl.at/huV18.
			return x() * rhs.y() - y() * rhs.x();
		}
		template<std::size_t D = N, std::enable_if_t<D == 3>* = nullptr>
		constexpr Vec3 cross(const Vec3& rhs) const
		{
			return {
				y() * rhs.z() - z() * rhs.y(),
				z() * rhs.x() - x() * rhs.z(),
				x() * rhs.y() - y() * rhs.x()
			};
		}
		template<std::size_t M>
		constexpr Matrix<N, M> outerProduct(const Vector<M>& rhs) const
		{
			return Matrix<N, 1>(components) * Matrix<1, M>(rhs);
		}
		constexpr float angleTo(const Vector<N>& rhs) const
		{
			return std::acos(dot(rhs) / (norm() * rhs.norm()));
		}
		constexpr Vector<N> projectOnto(const Vector<N>& rhs) const
		{
			return dot(rhs) / rhs.norm() * rhs;
		}

		constexpr float norm(const float p) const
		{
			float sum = 0.0f;
			for (const float component : components)
			{
				sum += std::pow(std::abs(component), p);
			}
			return std::pow(sum, 1.0f / p);
		}
		constexpr float norm() const
		{
			// Same as the 2-norm, but performed more efficiently than Vector<N>::norm(2.0f).
			return std::sqrt(dot(*this));
		}
		constexpr float magnitude() const
		{
			return norm();
		}
		constexpr float length() const
		{
			return norm();
		}

		constexpr float sum() const
		{
			// Same as the 1-norm.
			return std::accumulate(components.begin(), components.end(), 0.0f);
		}
		constexpr float max() const
		{
			// Same as the inf-norm.
			return *std::max_element(components.begin(), components.end());
		}

		constexpr Vector<N>& normalize()
		{
			return *this /= norm();
		}
		constexpr Vector<N> unit() const
		{
			Vector<N> result = *this;
			return result.normalize();
		}

		// TODO constexpr when MSVC becomes compliant with C++23
		template<std::size_t D = N, std::enable_if_t<D == 3>* = nullptr>
		Vec3 rotatedAboutAxis(const Vec3& origin, const Vec3& axis, const float theta)
		{
			const Mat3 axes = axesZ(axis.unit());
			// This uses the fact that the transpose of axes is equal to its inverse since axes is
			// orthonormal.
			return axes.transpose() * (rotationZ(theta) * (axes * (*this - origin))) + origin;
		}
		template<std::size_t D = N, std::enable_if_t<D == 3>* = nullptr>
		Vec3 rotatedAboutAxis(const Vec3& axis, const float theta)
		{
			return rotatedAboutAxis(Vec3(0.0f), axis, theta);
		}
		template<std::size_t D = N, std::enable_if_t<D == 3>* = nullptr>
		Vec3 rotatedAboutSegment(const Vec3& start, const Vec3& end, const float theta)
		{
			return rotatedAboutAxis(start, end - start, theta);
		}

		template<std::size_t D = N, std::enable_if_t<D == 3>* = nullptr>
		Vec3& rotateAboutAxis(const Vec3& origin, const Vec3& axis, const float theta)
		{
			*this = rotatedAboutAxis(origin, axis, theta);
			return *this;
		}
		template<std::size_t D = N, std::enable_if_t<D == 3>* = nullptr>
		Vec3& rotateAboutAxis(const Vec3& axis, const float theta)
		{
			*this = rotatedAboutAxis(axis, theta);
			return *this;
		}
		template<std::size_t D = N, std::enable_if_t<D == 3>* = nullptr>
		Vec3& rotateAboutSegment(const Vec3& start, const Vec3& end, const float theta)
		{
			*this = rotatedAboutSegment(start, end, theta);
			return *this;
		}

		constexpr Vector<N> clamped(const float lowerBound, const float upperBound) const
		{
			Vector<N> result;
			for (std::size_t i = 0; i < N; i++)
			{
				result[i] = std::clamp(components[i], lowerBound, upperBound);
			}
			return result;
		}
		constexpr Vector<N> clamped(const std::array<float, N>& lowerBounds,
			const std::array<float, N>& upperBounds) const
		{
			Vector<N> result;
			for (std::size_t i = 0; i < N; i++)
			{
				result[i] = std::clamp(components[i], lowerBounds[i], upperBounds[i]);
			}
			return result;
		}
		constexpr Vector<N> clamped(const Vector<N>& lowerBounds, const Vector<N>& upperBounds)
			const
		{
			return clamped(lowerBounds.components, upperBounds.components);
		}
		constexpr Vector<N>& clamp(const float lowerBound, const float upperBound)
		{
			*this = clamped(lowerBound, upperBound);
			return *this;
		}
		constexpr Vector<N>& clamp(const std::array<float, N>& lowerBounds,
			const std::array<float, N>& upperBounds)
		{
			*this = clamped(lowerBounds, upperBounds);
			return *this;
		}
		constexpr Vector<N>& clamp(const Vector<N>& lowerBounds, const Vector<N>& upperBounds)
		{
			*this = clamped(lowerBounds, upperBounds);
			return *this;
		}

		template<std::size_t D = N, std::enable_if_t<D >= 1>* = nullptr>
		constexpr float& x()
		{
			return components[0];
		}
		template<std::size_t D = N, std::enable_if_t<D >= 1>* = nullptr>
		constexpr const float& x() const
		{
			return components[0];
		}
		template<std::size_t D = N, std::enable_if_t<D >= 2>* = nullptr>
		constexpr float& y()
		{
			return components[1];
		}
		template<std::size_t D = N, std::enable_if_t<D >= 2>* = nullptr>
		constexpr const float& y() const
		{
			return components[1];
		}
		template<std::size_t D = N, std::enable_if_t<D >= 3>* = nullptr>
		constexpr float& z()
		{
			return components[2];
		}
		template<std::size_t D = N, std::enable_if_t<D >= 3>* = nullptr>
		constexpr const float& z() const
		{
			return components[2];
		}
		template<std::size_t D = N, std::enable_if_t<D >= 4>* = nullptr>
		constexpr float& w()
		{
			return components[3];
		}
		template<std::size_t D = N, std::enable_if_t<D >= 4>* = nullptr>
		constexpr const float& w() const
		{
			return components[3];
		}

		template<std::size_t D = N, std::enable_if_t<D >= 1>* = nullptr>
		constexpr float& r()
		{
			return components[0];
		}
		template<std::size_t D = N, std::enable_if_t<D >= 1>* = nullptr>
		constexpr const float& r() const
		{
			return components[0];
		}
		template<std::size_t D = N, std::enable_if_t<D >= 2>* = nullptr>
		constexpr float& g()
		{
			return components[1];
		}
		template<std::size_t D = N, std::enable_if_t<D >= 2>* = nullptr>
		constexpr const float& g() const
		{
			return components[1];
		}
		template<std::size_t D = N, std::enable_if_t<D >= 3>* = nullptr>
		constexpr float& b()
		{
			return components[2];
		}
		template<std::size_t D = N, std::enable_if_t<D >= 3>* = nullptr>
		constexpr const float& b() const
		{
			return components[2];
		}
		template<std::size_t D = N, std::enable_if_t<D >= 4>* = nullptr>
		constexpr float& a()
		{
			return components[3];
		}
		template<std::size_t D = N, std::enable_if_t<D >= 4>* = nullptr>
		constexpr const float& a() const
		{
			return components[3];
		}
	};

	template<std::size_t N>
	constexpr Vector<N> operator+(Vector<N> lhs, const Vector<N>& rhs)
	{
		lhs += rhs;
		return lhs;
	}
	template<std::size_t N>
	constexpr Vector<N> operator-(Vector<N> lhs, const Vector<N>& rhs)
	{
		lhs -= rhs;
		return lhs;
	}
	template<std::size_t N>
	constexpr Vector<N> operator-(Vector<N> rhs)
	{
		for (std::size_t i = 0; i < N; i++)
		{
			rhs[i] = -rhs[i];
		}
		return rhs;
	}
	template<std::size_t N>
	constexpr Vector<N> operator*(Vector<N> lhs, const float rhs)
	{
		lhs *= rhs;
		return lhs;
	}
	template<std::size_t N>
	constexpr Vector<N> operator*(const float lhs, const Vector<N>& rhs)
	{
		return rhs * lhs;
	}
	template<std::size_t N>
	constexpr Vector<N> operator/(Vector<N> lhs, const float rhs)
	{
		lhs /= rhs;
		return lhs;
	}

	template<std::size_t N>
	constexpr bool operator==(const Vector<N>& lhs, const Vector<N>& rhs)
	{
		for (std::size_t i = 0; i < N; i++)
		{
			if (std::abs(lhs[i] - rhs[i]) > epsilon)
			{
				return false;
			}
		}
		return true;
	}
	template<std::size_t N>
	constexpr bool operator!=(const Vector<N>& lhs, const Vector<N>& rhs)
	{
		return !(lhs == rhs);
	}

	template<std::size_t N>
	std::ostream& operator<<(std::ostream& lhs, const Vector<N>& rhs)
	{
		lhs << '(';
		for (std::size_t i = 0; i + 1 < N; i++)
		{
			lhs << rhs[i] << ' ';
		}
		if (N)
		{
			lhs << rhs[N - 1];
		}
		lhs << ')';
		return lhs;
	}
	template<std::size_t N>
	std::istream& operator>>(std::istream& lhs, Vector<N>& rhs)
	{
		for (std::size_t i = 0; i < N; i++)
		{
			lhs >> rhs[i];
		}
		return lhs;
	}

	template<std::size_t N>
	constexpr float dot(const Vector<N>& lhs, const Vector<N>& rhs)
	{
		return lhs.dot(rhs);
	}
	constexpr float cross(const Vec2& lhs, const Vec2& rhs)
	{
		return lhs.cross(rhs);
	}
	constexpr Vec3 cross(const Vec3& lhs, const Vec3& rhs)
	{
		return lhs.cross(rhs);
	}
	template<std::size_t N, std::size_t M>
	constexpr Matrix<N, M> outerProduct(const Vector<N>& lhs, const Vector<M>& rhs)
	{
		return lhs.outerProduct(rhs);
	}
	template<std::size_t N>
	constexpr float angleBetween(const Vector<N>& lhs, const Vector<N>& rhs)
	{
		return lhs.angleTo(rhs);
	}
	template<std::size_t N>
	constexpr Vector<N> project(const Vector<N>& lhs, const Vector<N>& rhs)
	{
		return lhs.projectOnto(rhs);
	}
}
