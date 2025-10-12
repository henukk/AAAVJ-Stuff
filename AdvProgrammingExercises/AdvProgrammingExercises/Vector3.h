#pragma once
#include <cmath>

template <class T>
class Vector3
{
private:
	T x;
	T y;
	T z;

public:
#pragma region CONSTRUCTOR - DESTRUCTOR
	constexpr Vector3() : x(0), y(0), z(0) {}
	constexpr Vector3(T x, T y, T z) : x(x), y(y), z(z) {}
	constexpr Vector3(const Vector3& v) = default;
#pragma endregion

#pragma region GETTERS
	constexpr inline T getX() const {
		return x;
	}
	constexpr inline T getY() const {
		return y;
	}
	constexpr inline T getZ() const {
		return z;
	}
#pragma endregion

#pragma region SETTERS
	constexpr inline void setX(const T& newX) {
		x = newX;
	}
	constexpr inline void setY(const T& newY) {
		y = newY;
	}
	constexpr inline void setZ(const T& newZ) {
		z = newZ;
	}
#pragma endregion

#pragma region OPERATORS
	constexpr Vector3<T> operator+(const Vector3<T>& other) const {
		return { x + other.x, y + other.y, z + other.z };
	}
	constexpr Vector3<T> operator-(const Vector3<T>& other) const {
		return { x - other.x, y - other.y, z - other.z };
	}
#pragma endregion

#pragma region FUNCTIONS
	constexpr inline T Magnitude() const {
		return std::sqrt(x * x + y * y + z * z);
	}

	constexpr inline T SquaredMagnitude() const {
		return x * x + y * y + z * z;
	}

	// quizás en vez de T pasar a float o a double para ints 4/5->0 | 4/5->0.8
	// int y float -> float | double -> double???
	constexpr inline Vector3<T> Normalize() const {
		T mag = Magnitude(); 
		if (mag == 0) return Vector3<T>(0, 0, 0);
		return { x / mag, y / mag, z / mag };
	}

	constexpr inline T distance_to(const Vector3& b) const {
		return (*this - b).Magnitude();
	}

	constexpr inline T dot_product(const Vector3& b) const {
		return x * b.x + y * b.y + z * b.z;
	}

	constexpr inline Vector3<T> cross_product(const Vector3<T>& b) const {
		return {
			y * b.z - z * b.y,
			z * b.x - x * b.z,
			x * b.y - y * b.x
		};
	}

	constexpr inline float angle_between(const Vector3<T>& b) const {
		T dot = dot_product(b);
		T magA2 = SquaredMagnitude();
		T magB2 = b.SquaredMagnitude();
		if (magA2 == 0 || magB2 == 0) return 0.0f;

		return std::acos(dot / std::sqrt(magA2 * magB2));
	}
#pragma endregion

#pragma region UTILS
	//con int hará cosas raras
	constexpr inline void print() const {
		std::printf("%.3f, %.3f, %.3f\n", x, y, z);
	}
#pragma endregion
};

