#include "spark_precompiled.h"


#include "pathfinder.h"

namespace pathfinder
{



	bool inside(ivec2 min, ivec2 max, ivec2 current)
	{
		return current.x >= min.x && current.x < max.x && current.y >= min.y && current.y < max.y;
	}

	vector<ivec2> GetNeighbours(ivec2 min, ivec2 max, ivec2 current)
	{
		vector<ivec2> n;

		//no diagonals
		//coordinates c = current + coordinates{ 0,1};
		//if (inside(min, max, c))n.push_back(c);
		//c = current + coordinates{ 0,-1 };
		//if (inside(min, max, c))n.push_back(c);
		//c = current + coordinates{ 1,0 };
		//if (inside(min, max, c))n.push_back(c);
		//c = current + coordinates{ -1,0 };
		//if (inside(min, max, c))n.push_back(c);

		//with diagonals
		for (int x = -1; x < 2; x++)
		{
			for (int y = -1; y < 2; y++)
			{
				ivec2 c = current + coordinates{ x,y };

				if (c == current)continue;
				if (inside(min, max, c))n.push_back(c);
			}
		}
		return n;
	}


	bool CalculatePath(Path &path, HeightField heightField, const coordinates from, const coordinates to, PathfinderProperties properties)
	{
		auto size = heightField.GetSize();
		bool found = false;
		vector<coordinates> open; //the set of nodes to be evaluated
		vector<coordinates> closed; //the set of nodes already evaluated

		PathfinderField field(size);


		//std::cout<<"\n\n";

		open.push_back(from);

		{
			auto &cell = field.GetCell(from);
			cell.G = 0;
			cell.H = static_cast<uint32>(from.distance(to)*1.0f);
			cell.F = cell.G + cell.H;
		}
		{
			auto &cell = field.GetCell(to);
			cell.G = static_cast<uint32>(from.distance(to)*1.0f);
			cell.H = 0;
			cell.F = cell.G + cell.H;
		}

		// print(field);

		while (!open.empty()) {
			auto current_it = std::min_element(open.begin(), open.end(), [&field](auto a, auto b)->bool {return field.GetCell(a).F < field.GetCell(b).F; });
			auto current = *current_it;

			open.erase(current_it);
			closed.push_back(current);

			if (current == to) {
				found = true;
				break;
			}

			auto current_height = heightField.GetCell(current);
			auto &current_cell = field.GetCell(current);

			//std::cout<<"current is "<<current.x<<","<<current.y<<"\t\t\t\t\tF-cost:"<<current_cell.F<<"\tG-cost:"<<current_cell.G<<"\tH-cost:"<<current_cell.H<<"\n";

			auto neighbours = GetNeighbours({ 0,0 }, size, current);
			for (auto n : neighbours)
			{


				if (properties.max_slope < static_cast<uint32>(std::abs(static_cast<int>(current_height - heightField.GetCell(n)))))continue;
				if (std::find(closed.begin(), closed.end(), n) != closed.end())continue;

				uint32 distance = static_cast<uint32>(n.distance(current)*1.0f);

				auto &cell = field.GetCell(n);
				if (std::find(open.begin(), open.end(), n) == open.end() || cell.G > current_cell.G + distance)
				{
					cell.G = current_cell.G + distance;
					cell.H = static_cast<uint32>(n.distance(to)*1.0f);
					cell.F = cell.G + cell.H;
					cell.parent = current;
					if (std::find(open.begin(), open.end(), n) == open.end())
					{
						open.push_back(n);
					}
				}
				//std::cout<<"\tneighbour is "<<n.x<<","<<n.y<<"\tdistance="<<distance<<"\tF-cost:"<<cell.F<<"\tG-cost:"<<cell.G<<"\tH-cost:"<<current_cell.H<<"\n";
			}


		};

		if (!found)return false;

		coordinates current = to;
		while (current != from)
		{
			path.push_back(current);

			auto &current_cell = field.GetCell(current);
			current = current_cell.parent;
		}
		path.push_back(current);

		std::reverse(path.begin(), path.end());

		// print(field,path);
		 //std::cout<<"\n\n";

		return true;
	}

	uint32 Distance(coordinates from, coordinates to)
	{
		return static_cast<uint32>(from.distance(to)*10.0f);
	}


	int CalculatePath(Path &path, coordinates from, coordinates to, const coordinates min, const coordinates max, std::function<bool(coordinates, coordinates)> isTraversable)
	{
		auto size = max - min;
		int path_found = 0;

		if (size.x <= 0 || size.y <= 0)return path_found; //min must be less than max
		if (!inside(min, max, from) || !inside(min, max, to))return path_found; //both starting and ending position must stay inside the area

		//convert to 0, size coordinate system
		from -= min;
		to -= min;

		bool found = false;
		vector<uvec2> open; //the set of nodes to be evaluated
		vector<uvec2> closed; //the set of nodes already evaluated
		uvec2 current;

		int ttl = 1000;

		PathfinderField field(size); 

		open.push_back(from); //insert the starting point

		{
			auto &cell = field.GetCell(from);
			cell.G = 0;
			cell.H = Distance(from, to);
			cell.F = cell.G + cell.H;
		}
		{
			auto &cell = field.GetCell(to);
			cell.G = Distance(from, to);
			cell.H = 0;
			cell.F = cell.G + cell.H;
		}



		while (!open.empty() && ttl--) {
			auto current_it = std::min_element(open.begin(), open.end(), [&field](auto a, auto b)->bool {return field.GetCell(a).F < field.GetCell(b).F; });
			current = *current_it;

			open.erase(current_it);
			closed.push_back(current);

			if (current == (ivec2)to) {
				found = true;
				break;
			}

			auto &current_cell = field.GetCell(current);

			//std::cout<<"current is "<<current.x<<","<<current.y<<"\t\t\t\t\tF-cost:"<<current_cell.F<<"\tG-cost:"<<current_cell.G<<"\tH-cost:"<<current_cell.H<<"\n";

			auto neighbours = GetNeighbours({ 0,0 }, size, current);
			for (auto n : neighbours)
			{
				if (!isTraversable(current+min,n+min))continue;
				if (std::find(closed.begin(), closed.end(), n) != closed.end())continue;

				//uint32 distance = 1;// static_cast<uint32>(n.distance(current));	//calculating just the 4 adjacent neighbours, the distance is always 1
				uint32 distance = Distance(n, current);

				auto &cell = field.GetCell(n);
				if (std::find(open.begin(), open.end(), n) == open.end() || cell.G > current_cell.G + distance)
				{
					cell.G = current_cell.G + distance;
					cell.H = Distance(n, to);
					cell.F = cell.G + cell.H;
					cell.parent = current;
					if (std::find(open.begin(), open.end(), n) == open.end())
					{
						open.push_back(n);
					}
				}
				//std::cout<<"\tneighbour is "<<n.x<<","<<n.y<<"\tdistance="<<distance<<"\tF-cost:"<<cell.F<<"\tG-cost:"<<cell.G<<"\tH-cost:"<<current_cell.H<<"\n";
			}

		};

		if (!found)
		{
			auto current_it = std::min_element(closed.begin(), closed.end(), [&field](auto a, auto b)->bool {return field.GetCell(a).H < field.GetCell(b).H; });
			current = *current_it;

			path_found = 2;
			//return false;
		
			//return anyway the path to the closest position

		}
		else
		{
			path_found = 1;
		}

		//current = to;
		while ((ivec2)current != (ivec2)from)
		{
			path.push_back(current);

			auto &current_cell = field.GetCell(current);
			current = current_cell.parent;
		}
		path.push_back(current);

		if(path.size()<2)path.push_back(current);//make sure that the path has at least 2 points

		std::reverse(path.begin(), path.end());

		//re-convert to the initial coordinate system
		for (auto &p : path)
		{
			p += min;
		}

		//optimize path erasing  unnecessary mid points
		for (int current_index = 0; current_index < path.size()-2; current_index++)
		{
			bool ok = true;
			do
			{
				ok = true;
				coordinates pre = vec2(0.5, 0.5) + ivec2(path[current_index]);
				//pre += vec2(0.5, 0.5);
				//grid_raycast(pre, path[current_index+2], [&pre, &ok, &isTraversable](coordinates c) {
				//	if (!isTraversable(pre, c))
				//	{
				//		ok = false;
				//		return false;
				//	}
				//	pre = c;
				//	return true;
				//});

				//DDA(pre, path[current_index + 2], [&pre, &ok, &isTraversable](coordinates c) {
				//	if (!isTraversable(pre, c))
				//	{
				//		ok = false;
				//		return false;
				//	}
				//	pre = c;
				//	return true;
				//});
				auto current = vec2(0.5, 0.5) + ivec2(path[current_index + 2]);
				raytrace(pre, current, [&pre, &ok, &isTraversable](coordinates c) {
					if (!isTraversable(pre, c))
					{
						ok = false;
						return false;
					}
					pre = c;
					return true;
				});

				if (ok)path.erase(path.begin() + current_index + 1);
			} while (ok && current_index < path.size() - 2);
		}

		//what about twice
		for (int current_index = 0; current_index < path.size() - 2; current_index++)
		{
			bool ok = true;
			do
			{
				ok = true;
				coordinates pre = path[current_index];

				DDA(pre, path[current_index + 2], [&pre, &ok, &isTraversable](coordinates c) {
					if (!isTraversable(pre, c))
					{
						ok = false;
						return false;
					}
					pre = c;
					return true;
				});

				if (ok)path.erase(path.begin() + current_index + 1);
			} while (ok && current_index < path.size() - 2);
		}

		return path_found;
	}
}
