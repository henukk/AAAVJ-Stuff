#pragma once
#include <iostream>
#include <cmath>
#include <cassert>

template <class T>
class Vector3
{
private:
	T x;
	T y;
	T z;

public:
#pragma region CONSTRUCTORS & DESTRUCTORS
	constexpr inline Vector3() noexcept : x(0), y(0), z(0) {}
	constexpr inline Vector3(T x, T y, T z) noexcept  : x(x), y(y), z(z) {}
	constexpr inline Vector3(const Vector3& v) noexcept = default;

	//compilador debería hacer uno igual de eficiente
	//constexpr inline ~Vector3() = default;
#pragma endregion

#pragma region GETTERS
	constexpr inline T getX() const noexcept {
		return x;
	}
	constexpr inline T getY() const noexcept {
		return y;
	}
	constexpr inline T getZ() const noexcept {
		return z;
	}
#pragma endregion

#pragma region SETTERS
	constexpr inline void setX(const T& newX) noexcept {
		x = newX;
	}
	constexpr inline void setY(const T& newY) noexcept {
		y = newY;
	}
	constexpr inline void setZ(const T& newZ) noexcept {
		z = newZ;
	}
#pragma endregion

#pragma region OPERATORS
	constexpr inline Vector3<T> operator+(const Vector3<T>& other) const noexcept {
		return { x + other.x, y + other.y, z + other.z };
	}
	constexpr inline Vector3<T> operator-(const Vector3<T>& other) const noexcept {
		return { x - other.x, y - other.y, z - other.z };
	}
	friend std::ostream& operator<<(std::ostream& os, const Vector3<T>& v) noexcept {
		assert(!std::isnan(v.x) && !std::isnan(v.y) && !std::isnan(v.z) &&
			"Vector3::operator<< - vector contains NaN values");
		os << '(' << v.x << ", " << v.y << ", " << v.z << ')';
		return os;
	}
#pragma endregion

#pragma region FUNCTIONS
	constexpr inline T Magnitude() const noexcept {
		return std::sqrt(x * x + y * y + z * z);
	}

	constexpr inline T SquaredMagnitude() const noexcept {
		return x * x + y * y + z * z;
	}

	// quizás en vez de T pasar a float o a double para ints 4/5->0 | 4/5->0.8
	// int y float -> float | double -> double???
	constexpr inline Vector3<T> Normalize() const noexcept {
		T mag = Magnitude();
		assert(mag != 0 && "ASSERT FAIL: Vector3::Normalize() - cannot normalize a zero-length vector");
		if (mag == 0) return Vector3<T>(0, 0, 0);
		return { x / mag, y / mag, z / mag };
	}

	constexpr inline T distance_to(const Vector3& b) const noexcept {
		return (*this - b).Magnitude();
	}

	constexpr inline T dot_product(const Vector3& b) const noexcept {
		return x * b.x + y * b.y + z * b.z;
	}

	constexpr inline Vector3<T> cross_product(const Vector3<T>& b) const noexcept {
		return {
			y * b.z - z * b.y,
			z * b.x - x * b.z,
			x * b.y - y * b.x
		};
	}

	constexpr inline float angle_between(const Vector3<T>& b) const noexcept {
		T dot = dot_product(b);
		T magA2 = SquaredMagnitude();
		T magB2 = b.SquaredMagnitude();
		assert(magA2 > 0 && magB2 > 0 && "ASSERT FAIL: Vector3::angle_between() - one of the vectors is zero-length");
		if (magA2 == 0 || magB2 == 0) return 0.0f;

		return std::acos(dot / std::sqrt(magA2 * magB2));
	}
#pragma endregion

#pragma region UTILS/HELPERS
	constexpr inline void print() const noexcept {
		std::cout << *this << '\n';
	}
#pragma endregion
};


