#pragma once


namespace pathfinder
{
	using uint32 = unsigned int;

	template<typename T>
	struct vec2_base {
		using CoordinateType = T;
		CoordinateType x, y;

		vec2_base() :x(0), y(0) {}
		vec2_base(CoordinateType x, CoordinateType y) :x(x), y(y) {}

		template<typename T>
		vec2_base(T x, T y) : x(static_cast<CoordinateType>(x)), y(static_cast<CoordinateType>(y)) {}

		template<typename T, template <typename> class other_vec>
		vec2_base(const other_vec<T> &other) : x(static_cast<CoordinateType>(other.x)), y(static_cast<CoordinateType>(other.y)) {}

		static vec2_base Invalid() {
			return vec2_base{ std::numeric_limits<CoordinateType>::infinity(),std::numeric_limits<CoordinateType>::infinity() };
		}
		bool IsValid() const {
			return *this != Invalid();
		}

		vec2_base operator+(const vec2_base& other) const
		{
			return vec2_base{ x + other.x,y + other.y };
		}

		vec2_base operator-(const vec2_base& other) const
		{
			return vec2_base{ x - other.x,y - other.y };
		}

		vec2_base operator*(const vec2_base& other) const
		{
			return vec2_base{ x*other.x,y*other.y };
		}
		vec2_base operator/(const vec2_base& other) const
		{
			return vec2_base{ x / other.x,y / other.y };
		}

		vec2_base operator*(const CoordinateType scalar) const
		{
			return vec2_base{ x*scalar,y*scalar };
		}

		vec2_base operator/(const CoordinateType scalar) const
		{
			return vec2_base{ x / scalar,y / scalar };
		}

		vec2_base& operator+=(const vec2_base& other)
		{
			x += other.x;
			y += other.y;
			return *this;
		}

		vec2_base operator-=(const vec2_base& other)
		{
			x -= other.x;
			y -= other.y;
			return *this;
		}

		vec2_base operator*=(const vec2_base& other)
		{
			x *= other.x;
			y *= other.y;
			return *this;
		}


		vec2_base operator*=(const CoordinateType scalar)
		{
			x *= scalar;
			y *= scalar;
			return *this;
		}

		vec2_base operator/=(const vec2_base& other)
		{
			x /= other.x;
			y /= other.y;
			return *this;
		}


		vec2_base operator/=(const CoordinateType scalar)
		{
			x /= scalar;
			y /= scalar;
			return *this;
		}

		bool operator==(const vec2_base& other) const
		{
			return x == other.x && y == other.y;
		}

		bool operator!=(const vec2_base& other) const
		{
			return !(*this == other);
		}

		CoordinateType distance_squared(const vec2_base& other) const
		{
			return (x - other.x)*(x - other.x) + (y - other.y)*(y - other.y);
		}
		float distance(const vec2_base& other) const
		{
			return std::sqrtf(static_cast<float>(distance_squared(other)));
		}
	};


	using uvec2 = vec2_base<uint32>;
	using ivec2 = vec2_base<int>;
	using vec2 = vec2_base<float>;

	using coordinates = vec2;

}
