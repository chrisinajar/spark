#pragma once

#include <AzCore/std/containers/set.h>

namespace SetUtils
{

	template<typename T>
	T set_union(const T &setA, const T &setB)
	{
		T u;
		for (auto t : setA)u.insert(t);
		for (auto t : setB)u.insert(t);
		return u;
	}

	template<typename T>
	T set_intersection(const T &setA, const T &setB)
	{
		T u;
		for (auto t : setA)if (setB.find(t) != setB.end())u.insert(t);
		return u;
	}

	template<typename T>
	T set_difference(const T &setA, const T &setB)
	{
		T u;
		for (auto t : setA)if (setB.find(t) == setB.end())u.insert(t);
		return u;
	}

}
