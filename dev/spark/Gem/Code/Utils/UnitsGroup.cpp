#include "spark_precompiled.h"

#include "UnitsGroup.h"

using namespace spark;


AZStd::vector<AZ::EntityId> UnitsGroup::GetVector() const 
{
	return *this;
}

AZ::EntityId UnitsGroup::GetMainUnit() const//the one shown in the HUD
{
	return GetCurrent();
}

int	UnitsGroup::GetMainUnitIndex() const
{
	return GetIndex();
}

void UnitsGroup::SetMainUnit(AZ::EntityId unit) 
{
	int i = 0;
	for (AZ::EntityId u : GetVector()) 
	{
		if (u == unit) 
		{
			SetIndex(i);
			return;
		}
		++i;
	}
}

void UnitsGroup::SetMainUnitByIndex(int index) 
{
	SetIndex(index);
}

AZ::EntityId UnitsGroup::NextUnit() 
{
	return GetNext();
}

AZ::EntityId UnitsGroup::PreviousUnit() 
{
	return GetPrevious();
}


void UnitsGroup::Add(AZ::EntityId id, bool set_main_unit)
{
	push_back(id);
	if (set_main_unit)SetIndex(-1);
}

void UnitsGroup::Set(AZ::EntityId unit)
{
	clear();
	push_back(unit);
}

void UnitsGroup::Clear()
{
	clear();
}

bool UnitsGroup::Contains(AZ::EntityId unit) const
{
	return AZStd::find(begin(), end(), unit)!=end();
}

void UnitsGroup::Remove(AZ::EntityId unit)
{
	auto it=AZStd::find(begin(), end(), unit);
	if (it != end())
	{
		erase(it);
	}
}
