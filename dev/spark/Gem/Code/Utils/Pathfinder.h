#pragma once


#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <climits>
#include <functional>

#include "PathfinderVec2.h"

namespace pathfinder
{

	using std::vector;


	template<typename CellType>
	struct Field
	{
		Field() {}

		Field(uvec2 size)
		{
			SetSize(size);
		}

		bool IsInside(ivec2 c) const
		{
			return c.x >= 0 && c.y >= 0 && c.x < GetSize().x && c.y < GetSize().y;
		}
		CellType* GetCellSecure(ivec2 c)
		{
			return IsInside(c) ? &data[c.y][c.x] : nullptr;
		}
		CellType& GetCell(uvec2 c)
		{
			return  data[c.y][c.x];
		}
		uvec2 GetSize() const
		{
			return uvec2{ data.size(),(data.empty() ? 0 : data[0].size()) };
		}
		void SetSize(ivec2 size)
		{
			data.resize((size_t)size.y);

			for (auto &row : data)
			{
				row.resize((size_t)size.x);
			}
		}

		vector<vector<CellType>> data;
	};





	struct PathfinderCell
	{
		coordinates parent = coordinates::Invalid();
		uint32 G = std::numeric_limits<uint32>::max(),
			H = std::numeric_limits<uint32>::max(),
			F = std::numeric_limits<uint32>::max();
	};

	struct PathfinderProperties
	{
		uint32 max_slope = 0;
		PathfinderProperties() {}
	};

	using HeightField = Field<uint32>;
	using PathfinderField = Field<PathfinderCell>;
	using Path = vector<coordinates>;


	extern bool CalculatePath(Path &path, HeightField heightField, const coordinates from, const coordinates to, PathfinderProperties properties = PathfinderProperties());

	typedef bool(*IsTraversable)(coordinates,coordinates);


	//0  -> destination unreachable
	//1  -> destination reachable
	//2  -> destination unreachable, but the path to the closeset reachable position is returned
	extern int CalculatePath(Path &path, coordinates from, coordinates to, const coordinates min, const coordinates max, std::function< bool(coordinates, coordinates)> isTraversable);


	template<typename Foo>
	void RasterizeAABB(coordinates min, coordinates max, Foo foo)
	{
		AZ_Assert(max.x - min.x < 1000000, "X values for rasterize are insane");
		AZ_Assert(max.y - min.y < 1000000, "Y values for rasterize are insane");
		for (int x = (int)min.x; x < max.x + 0.01f; ++x) //add 0.01 so it always rasterize all tiles containing the aabb, also when min==max (the aabb becomes a point)
		{
			for (int y = (int)min.y; y < max.y + 0.01f; ++y)
			{
				foo({ x,y });
			}
		}
	}
	
	//ispired solution of kmillen at https://stackoverflow.com/questions/1201200/fast-algorithm-for-drawing-filled-circles
	template<typename Foo>
	void RasterizeFilledCircle(coordinates center, int radius, Foo foo)
	{
		int r2 = radius * radius;
		int area = r2 << 2;
		int rr = radius << 1;

		for (int i = 0; i < area; i++)
		{
			int tx = (i % rr) - radius;
			int ty = (i / rr) - radius;

			if (tx * tx + ty * ty <= r2)
				foo({center.x + tx, center.y + ty});
		}
	}

	//DDA Function for line generation 
	template<typename Foo>
	void DDA(coordinates from,coordinates to,Foo foo)
	{
		// calculate dx & dy 
		ivec2 d = to - from;

		// calculate steps required for generating pixels 
		int steps = abs(d.x) > abs(d.y) ? abs(d.x) : abs(d.y);

		// calculate increment in x & y for each steps 
		float Xinc = d.x / (float)steps;
		float Yinc = d.y / (float)steps;

		// Put pixel for each step 
		float X = (float)from.x;
		float Y = (float)from.y;
		for (int i = 0; i <= steps; i++)
		{
			foo({ X, Y });		 // put pixel at (X,Y) 
			X += Xinc;           // increment in x at each step 
			Y += Yinc;           // increment in y at each step 
		}
	}

	template<typename Foo>
	int raytrace(vec2 origin, vec2 end,Foo visit)
	{
		float dx = fabs(end.x - origin.x);
		float dy = fabs(end.y - origin.y);

		int x = int(floor(origin.x));
		int y = int(floor(origin.y));

		int n = 1;
		int x_inc, y_inc;
		float error;

		if (dx == 0)
		{
			x_inc = 0;
			error = std::numeric_limits<float>::infinity();
		}
		else if (end.x > origin.x)
		{
			x_inc = 1;
			n += int(floor(end.x)) - x;
			error = (floor(origin.x) + 1 - origin.x) * dy;
		}
		else
		{
			x_inc = -1;
			n += x - int(floor(end.x));
			error = (origin.x - floor(origin.x)) * dy;
		}

		if (dy == 0)
		{
			y_inc = 0;
			error -= std::numeric_limits<float>::infinity();
		}
		else if (end.y > origin.y)
		{
			y_inc = 1;
			n += int(floor(end.y)) - y;
			error -= (floor(origin.y) + 1 - origin.y) * dx;
		}
		else
		{
			y_inc = -1;
			n += y - int(floor(end.y));
			error -= (origin.y - floor(origin.y)) * dx;
		}

		int i = 0;
		for (; n > 0; --n)
		{
			if (!visit({ x, y }))return i;
			++i;

			if (error > 0)
			{
				y += y_inc;
				error -= dx;
			}
			else
			{
				x += x_inc;
				error += dy;
			}
		}

		return i;
	}

	template<typename Foo>
	int grid_raycast(vec2 origin, vec2 end, Foo foo)
	{
		vec2 dir = end - origin;

		vec2 side;
		vec2 delta = { abs(1 / dir.x), abs(1 / dir.y) };
		ivec2 step;

		ivec2 pos = origin;

		if (dir.x < 0)
		{
			step.x = -1;
			side.x = (origin.x - pos.x) * delta.x;
		}
		else
		{
			step.x = 1;
			side.x = (pos.x  + 1 - origin.x ) * delta.x;
		}

		if (dir.y < 0)
		{
			step.y = -1;
			side.y = (origin.y - pos.y) * delta.y;
		}
		else
		{
			step.y = 1;
			side.y = (pos.y + 1 - origin.y) * delta.y;
		}
		int i = 0;
		//perform DDA
		while (pos !=(ivec2)end)
		{
			//jump to next map square, OR in x-direction, OR in y-direction
			if (side.x < side.y)
			{
				side.x += delta.x;
				pos.x += step.x;
			}
			else
			{
				side.y += delta.y;
				pos.y += step.y;
			}
			//Check if ray has hit a wall
			if (!foo(pos))return i;
			++i;
		}
		return i;
	}
}
