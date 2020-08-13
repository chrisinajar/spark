#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/containers/set.h>
#include "Busses/UnitBus.h"

namespace spark 
{

	//VISIBILITY
	// each tile has a enum for visibility
	// each team use (for now) two bits, having 4 level of visibility: not visible (00), visible(01), true vision(10), total vision(11)
	// to access the bits for each flag do :  (flag>>(team_index*2)) & 3
	enum VisibilityEnum
	{
		NOT_VISIBLE,
		VISIBLE,
		TRUE_VISION,
		TOTAL_VISION,
	};

	class MapRequests
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;

		virtual float GetTerrainHeightAt(AZ::Vector3) = 0; //z coordinate is ignored
		virtual AZ::Vector3 GetNearestFreePosition(AZ::Vector3) = 0;

		//visibility (works the same for everyone)
		virtual UnitId GetUnitAt(AZ::Vector3) = 0;
		virtual AZStd::vector<UnitId> GetUnitsAt(AZ::Vector3) = 0;
		virtual VisibilityEnum IsPointVisibleByTeam(AZ::Vector3, TeamId) = 0;
		virtual VisibilityEnum IsPointVisibleByUnit(AZ::Vector3, UnitId) = 0;
		virtual bool IsUnitVisibleByUnit(UnitId, UnitId) = 0;
		virtual bool IsUnitVisibleByTeam(UnitId, TeamId) = 0;

		//visibility - but is based on the view ( like in the model-view-controller pattern, the view being the team of the player, or the teams that the spectator want to see)
		virtual void ShowMapForTeam(TeamId team) = 0; //set the view
		virtual void ShowMapForTeams(AZStd::set<TeamId> teams) = 0; 
		
		virtual bool IsUnitVisible(UnitId) = 0;
		virtual bool IsPointVisible(AZ::Vector3) = 0;
	};
	using MapRequestBus = AZ::EBus<MapRequests>;

	class MapVisibilityNotifications
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
		
		virtual void OnUnitVisibilityChange(UnitId, VisibilityEnum) {}
	};
	using MapVisibilityNotificationBus = AZ::EBus<MapVisibilityNotifications>;

	
}
