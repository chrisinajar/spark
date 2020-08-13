#pragma once

#include <AzCore/Component/EntityId.h>

#include "Utils/CircularVector.h"

namespace spark
{

	struct UnitsGroup 
			: protected CircularVector<AZ::EntityId>
	{

		AZStd::vector<AZ::EntityId> GetVector() const;

		AZ::EntityId GetMainUnit() const; //the one shown in the HUD
		int	         GetMainUnitIndex() const;
		void	     SetMainUnit(AZ::EntityId);
		void	     SetMainUnitByIndex(int index);
		AZ::EntityId NextUnit();
		AZ::EntityId PreviousUnit();

		void Add(AZ::EntityId id, bool set_main_unit = false);
		void Set(AZ::EntityId unit);

		void Clear();

		bool Contains(AZ::EntityId unit) const;
		void Remove(AZ::EntityId unit);
			
		using CircularVector<AZ::EntityId>::size;
		using CircularVector<AZ::EntityId>::empty;
		using CircularVector<AZ::EntityId>::iterator;
		using CircularVector<AZ::EntityId>::begin;
		using CircularVector<AZ::EntityId>::end;
	};
}
