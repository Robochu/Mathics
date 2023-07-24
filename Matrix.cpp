/* A matrix class used for math.
 * Similar to Vector<N>, this is just an std::array of vectors with the needed functionality.
 */

export module math:Matrix;

import :forward;
import :Vector;

import <array>;
import <iostream>;
import <type_traits>;
import <cstddef>;
import <cmath>;

export namespace math
{
	template<std::size_t M, std::size_t N>
	class Matrix
	{
		// Vector<N> is treated as a row vector here.
		std::array<Vector<N>, M> rows;

	public:
		constexpr Matrix() = default;
		explicit constexpr Matrix(const float entry)
		{
			rows.fill(Vector<N>(entry));
		}
		constexpr Matrix(const std::array<Vector<N>, M>& rows) : rows(rows) {}
		constexpr Matrix(const std::array<std::array<float, N>, M>& rows)
		{
			for (std::size_t i = 0; i < M; i++)
			{
				this->rows[i] = rows[i];
			}
		}
		constexpr Matrix(const std::array<float, M * N>& entries)
		{
			for (std::size_t i = 0; i < M; i++)
			{
				for (std::size_t j = 0; j < N; j++)
				{
					rows[i][j] = entries[i * N + j];
				}
			}
		}
		template<typename... T, std::enable_if_t<
			std::conjunction_v<std::is_same<T, Vector<N>>...> && sizeof...(T) == M
		>* = nullptr>
		constexpr Matrix(const T&... rows) : rows({rows...}) {}
		template<typename... T, std::enable_if_t<
			std::conjunction_v<std::is_same<T, std::array<float, N>>...> && sizeof...(T) == M
		>* = nullptr>
		constexpr Matrix(const T&... rows) :
			Matrix(std::array<std::array<float, N>, M>({rows...})) {}
		template<typename... T, std::enable_if_t<
			std::conjunction_v<std::is_same<T, float>...> && sizeof...(T) == M * N
		>* = nullptr>
		constexpr Matrix(const T... entries) : Matrix(std::array<float, M * N>({entries...})) {}

		constexpr Vector<N>& operator[](const std::size_t i)
		{
			return rows[i];
		}
		constexpr const Vector<N>& operator[](const std::size_t i) const
		{
			return rows[i];
		}
		constexpr Vector<M> getColumn(const std::size_t i) const
		{
			Vector<M> column;
			for (std::size_t j = 0; j < M; j++)
			{
				column[j] = rows[j][i];
			}
			return column;
		}

		constexpr Matrix<M, N>& operator+=(const Matrix<M, N>& rhs)
		{
			for (std::size_t i = 0; i < M; i++)
			{
				rows[i] += rhs[i];
			}
			return *this;
		}
		constexpr Matrix<M, N>& operator-=(const Matrix<M, N>& rhs)
		{
			for (std::size_t i = 0; i < M; i++)
			{
				rows[i] -= rhs[i];
			}
			return *this;
		}
		constexpr Matrix<M, N>& operator*= (const float rhs)
		{
			for (Vector<N>& row : rows)
			{
				row *= rhs;
			}
			return *this;
		}
		constexpr Matrix<M, N>& operator/= (const float rhs)
		{
			for (Vector<N>& row : rows)
			{
				row /= rhs;
			}
			return *this;
		}

		constexpr Matrix<N, M> transpose() const
		{
			Matrix<N, M> transpose;
			for (std::size_t i = 0; i < N; i++)
			{
				transpose[i] = getColumn(i);
			}
			return transpose;
		}

		template<std::size_t D = N, std::enable_if_t<D == 1 && M == D>* = nullptr>
		constexpr float determinant() const
		{
			return rows[0][0];
		}
		template<std::size_t D = N, std::enable_if_t<D == 2 && M == D>* = nullptr>
		constexpr float determinant() const
		{
			return rows[0][0] * rows[1][1] - rows[0][1] * rows[1][0];
		}
		template<std::size_t D = N, std::enable_if_t<D == 3 && M == D>* = nullptr>
		constexpr float determinant() const
		{
			return rows[0][0] * (rows[1][1] * rows[2][2] - rows[1][2] * rows[2][1]) +
				rows[0][1] * (rows[1][2] * rows[2][0] - rows[1][0] * rows[2][2]) +
				rows[0][2] * (rows[1][0] * rows[2][1] - rows[1][1] * rows[2][0]);
		}
		// TODO determinant in the general case

		template<std::size_t D = N, std::enable_if_t<D >= 2 && M == D>* = nullptr>
		constexpr float cofactor(const std::size_t row, const std::size_t col) const
		{
			Matrix<N - 1, N - 1> submatrix;
			for (std::size_t i = 0; i < N - 1; i++)
			{
				for (std::size_t j = 0; j < N - 1; j++)
				{
					submatrix[i][j] = rows[i + static_cast<std::size_t>(i >= row)]
						[j + static_cast<std::size_t>(j >= col)];
				}
			}
			return static_cast<bool>((row + col) % 2) ?
				-submatrix.determinant() : submatrix.determinant();
		}
		template<std::size_t D = N, std::enable_if_t<D >= 2 && M == D>* = nullptr>
		constexpr Matrix<N, N> comatrix() const
		{
			Matrix<N, N> comatrix;
			for (std::size_t i = 0; i < N; i++)
			{
				for (std::size_t j = 0; j < N; j++)
				{
					comatrix[i][j] = cofactor(i, j);
				}
			}
			return comatrix;
		}
		template<std::size_t D = N, std::enable_if_t<D >= 2 && M == D>* = nullptr>
		constexpr Matrix<N, N> adjugate() const
		{
			return comatrix().transpose();
		}

		template<std::size_t D = N, std::enable_if_t<D == 1 && M == D>* = nullptr>
		constexpr Matrix<1, 1> inverse() const
		{
			return Matrix<1, 1>(1.0f / determinant());
		}
		template<std::size_t D = N, std::enable_if_t<D == 2 && M == D>* = nullptr>
		constexpr Mat2 inverse() const
		{
			return Mat2(rows[1][1], -rows[0][1], -rows[1][0], rows[0][0]) / determinant();
		}
		template<std::size_t D = N, std::enable_if_t<D == 3 && M == D>* = nullptr>
		constexpr Mat3 inverse() const
		{
			// https://stackoverflow.com/questions/983999/simple-3x3-matrix-inverse-code-c
			return Mat3(
				rows[1][1] * rows[2][2] - rows[2][1] * rows[1][2],
				rows[0][2] * rows[2][1] - rows[0][1] * rows[2][2],
				rows[0][1] * rows[1][2] - rows[0][2] * rows[1][1],
				rows[1][2] * rows[2][0] - rows[1][0] * rows[2][2],
				rows[0][0] * rows[2][2] - rows[0][2] * rows[2][0],
				rows[1][0] * rows[0][2] - rows[0][0] * rows[1][2],
				rows[1][0] * rows[2][1] - rows[2][0] * rows[1][1],
				rows[2][0] * rows[0][1] - rows[0][0] * rows[2][1],
				rows[0][0] * rows[1][1] - rows[1][0] * rows[0][1]
			) / determinant();
		} // TODO inverse in the general case
		template<std::size_t D = N, std::enable_if_t<D >= 1 && M == D>* = nullptr>
		constexpr Matrix<N, N>& invert()
		{
			*this = inverse();
			return *this;
		}
	};

	template<std::size_t M, std::size_t N>
	constexpr Matrix<M, N> operator+(Matrix<M, N> lhs, const Matrix<M, N>& rhs)
	{
		lhs += rhs;
		return lhs;
	}
	template<std::size_t M, std::size_t N>
	constexpr Matrix<M, N> operator-(Matrix<M, N> lhs, const Matrix<M, N>& rhs)
	{
		lhs -= rhs;
		return lhs;
	}
	template<std::size_t M, std::size_t N>
	constexpr Matrix<M, N> operator-(Matrix<M, N> rhs)
	{
		for (std::size_t i = 0; i < M; i++)
		{
			rhs[i] = -rhs[i];
		}
		return rhs;
	}
	template<std::size_t M, std::size_t N>
	constexpr Matrix<M, N> operator*(Matrix<M, N> lhs, const float rhs)
	{
		lhs *= rhs;
		return lhs;
	}
	template<std::size_t M, std::size_t N>
	constexpr Matrix<M, N> operator*(const float lhs, const Matrix<M, N>& rhs)
	{
		return rhs * lhs;
	}
	template<std::size_t M, std::size_t N>
	constexpr Matrix<M, N> operator/(Matrix<M, N> lhs, const float rhs)
	{
		lhs /= rhs;
		return lhs;
	}

	template<std::size_t M, std::size_t N, std::size_t O>
	constexpr Matrix<M, O> operator*(const Matrix<M, N>& lhs, const Matrix<N, O>& rhs)
	{
		Matrix<M, O> product;
		for (std::size_t i = 0; i < M; i++)
		{
			for (std::size_t j = 0; j < O; j++)
			{
				product[i][j] = lhs[i].dot(rhs.getColumn(j));
			}
		}
		return product;
	}
	template<std::size_t M, std::size_t N>
	constexpr Vector<M> operator*(const Matrix<M, N>& lhs, const Vector<N>& rhs)
	{
		return Vector<M>(lhs * Matrix<1, N>(rhs).transpose());
	}
	template<std::size_t M, std::size_t N>
	constexpr Vector<N> operator*(const Vector<M>& lhs, const Matrix<M, N>& rhs)
	{
		return Vector<N>(Matrix<1, M>(lhs) * rhs);
	}

	template<std::size_t M, std::size_t N>
	constexpr bool operator==(const Matrix<M, N>& lhs, const Matrix<M, N>& rhs)
	{
		for (std::size_t i = 0; i < M; i++)
		{
			if (lhs[i] != rhs[i])
			{
				return false;
			}
		}
		return true;
	}
	template<std::size_t M, std::size_t N>
	constexpr bool operator!=(const Matrix<M, N>& lhs, const Matrix<M, N>& rhs)
	{
		return !(lhs == rhs);
	}

	template<std::size_t M, std::size_t N>
	std::ostream& operator<<(std::ostream& lhs, const Matrix<M, N>& rhs)
	{
		lhs << '[';
		for (std::size_t i = 0; i + 1 < M; i++)
		{
			lhs << rhs[i] << ", ";
		}
		if (M)
		{
			lhs << rhs[M - 1];
		}
		lhs << ']';
		return lhs;
	}
	template<std::size_t M, std::size_t N>
	std::istream& operator>>(std::istream& lhs, Matrix<M, N>& rhs)
	{
		for (std::size_t i = 0; i < M; i++)
		{
			lhs >> rhs[i];
		}
		return lhs;
	}

	// Prefer to use I<N>.
	template<std::size_t N>
	constexpr Matrix<N, N> identity()
	{
		Matrix<N, N> identity = Matrix<N, N>(0.0f);
		for (std::size_t i = 0; i < N; i++)
		{
			identity[i][i] = 1.0f;
		}
		return identity;
	}
	template<std::size_t N>
	constexpr Matrix<N, N> I = identity<N>();
	constexpr Mat2 I2 = I<2>;
	constexpr Mat3 I3 = I<3>;
	constexpr Mat4 I4 = I<4>;

	// Rotation matrices about the 3D axes. Because there is a need to rotate by an arbitrary
	// angle, Rx<A>, Ry<A>, and Rz<A> are not necessarily preferred.
	constexpr Mat3 rotationX(const float theta)
	{
		return {
			1.0f, 0.0f, 0.0f,
			0.0f, std::cos(theta), -std::sin(theta),
			0.0f, std::sin(theta), std::cos(theta)
		};
	}
	template<float A>
	constexpr Mat3 Rx = rotationX(A);
	constexpr Mat3 rotationY(const float theta)
	{
		return {
			std::cos(theta), 0.0f, std::sin(theta),
			0.0f, 1.0f, 0.0f,
			-std::sin(theta), 0.0f, std::cos(theta)
		};
	}
	template<float A>
	constexpr Mat3 Ry = rotationY(A);
	constexpr Mat3 rotationZ(const float theta)
	{
		return {
			std::cos(theta), -std::sin(theta), 0.0f,
			std::sin(theta), std::cos(theta), 0.0f,
			0.0f, 0.0f, 1.0f
		};
	}
	template<float A>
	constexpr Mat3 Rz = rotationZ(A);

	// Construct the remaining axes from a single z-axis. The axis is assumed to be normalized.
	inline Mat3 axesZ(const Vec3& axis) // TODO constexpr when MSVC becomes compliant with C++23
	{
		// https://backend.orbit.dtu.dk/ws/portalfiles/portal/126824972/onb_frisvad_jgt2012_v2.pdf
		// https://graphics.pixar.com/library/OrthonormalB/paper.pdf
		const float sign = std::copysign(1.0f, axis.z());
		const float a = -1.0f / (sign + axis.z());
		const float b = axis.x() * axis.y() * a;
		return {
			Vec3(1.0f + sign * axis.x() * axis.x() * a, sign * b, -sign * axis.x()),
			Vec3(b, sign + axis.y() * axis.y() * a, -axis.y()),
			axis
		};
	}
}
