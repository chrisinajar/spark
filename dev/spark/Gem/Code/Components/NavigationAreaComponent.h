#pragma once

#include "Busses/NavigationAreaBus.h"
#include <AzCore/Component/TickBus.h>

#include <AzCore/std/containers/vector.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/Math/Aabb.h>

#include "Utils/SparkNavPath.h"
#include "Utils/Pathfinder.h"
#include "Busses/MapBus.h"
#include "Busses/NavigationManagerBus.h"
//#include "NavigationManagerComponent.h"

#include <ITexture.h>

namespace spark
{
	using NavPath = SparkNavPath;

	using VisibilityField = pathfinder::Field<int>; //VisibilityEnum
	using BlockingField = pathfinder::Field<short>;
	using HeightField = pathfinder::Field<float>;
	using UnitField = pathfinder::Field< AZStd::set<AZ::EntityId> >;// AZ::EntityId > ;

	const float InvalidHeight = -1024.0f;

	class NavigationAreaComponent
		: public AZ::Component
		, public NavigationEntitiesNotificationBus::Handler
		, protected NavigationAreaRequestBus::Handler
		//, protected MapRequestBus::Handler
		//, protected AZ::TickBus::Handler
	{
	public:
		AZ_COMPONENT(NavigationAreaComponent, "{40ABCEC1-D7FF-41DB-A272-E729278F3346}");

		static void Reflect(AZ::ReflectContext* context);

	protected:

		////////////////////////////////////////////////////////////////////////
		// AZ::Component interface implementation
		void Init() override;
		void Activate() override;
		void Deactivate() override;

		static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
		static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
		static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
		static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);
		////////////////////////////////////////////////////////////////////////

	public:
		//navigation area
		void UpdateHeightfield();

		void SetTilesResolution(AZ::Vector2);
		AZ::Vector2 GetTilesResolution();

		AZ::Vector2 GetTileSize();

		AZ::Vector3 GetAabbMin();
		AZ::Vector3 GetAabbMax();

		bool IsPointInside(AZ::Vector3);
		bool IsUnitInside(UnitId);

		//navigation entities
		void OnNavigationEntityPositionChanged(AZ::EntityId, AZ::Vector3 previous, AZ::Vector3 current);

		//map bus
		float GetTerrainHeightAt(AZ::Vector3); //z coordinate is ignored

		AZ::EntityId GetUnitAt(AZ::Vector3);
		AZStd::vector<AZ::EntityId> GetUnitsAt(AZ::Vector3);
		AZ::Vector3 GetNearestFreePosition(AZ::Vector3);

		VisibilityEnum IsPointVisibleByTeam(AZ::Vector3, TeamId);
		VisibilityEnum IsPointVisibleByUnit(AZ::Vector3, UnitId);
		bool IsUnitVisibleByUnit(UnitId, UnitId);
		bool IsUnitVisibleByTeam(UnitId, TeamId);


		//fog of war
		void BindMiniMapTexture();
		void BindVisibilityTexture();
		void UnbindVisibilityTexture();

		void UpdateMinimapTexture();
		void ClearVisibilityField();
		void UpdateVisibilityFieldForTeams(AZStd::set<TeamId> teams);
		void UpdateVisibilityTexture();

		//internal
		pathfinder::coordinates ConvertToGridCoordinates(AZ::Vector3 worldCoordinates);
		AZ::Vector3 ConvertToWorldCoordinates(pathfinder::coordinates gridCoordinates, AZ::Vector3 cellOffset=AZ::Vector3::CreateZero());
		
		void DrawTiles();
	
		AZ::Aabb	m_aabb;
		AZ::Vector3 m_resolution = AZ::Vector3{ 64,64,1};

		HeightField m_heightField;
		UnitField	m_unitField;
		VisibilityField m_visibilityField;
		BlockingField m_movementBlockingField;
		BlockingField m_visionBlockingField;

		float m_maxSlope = 0.5;

	private:

		//void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

		bool m_debug = false;
		ITexture* m_visibilityMap = nullptr;

		float m_maxHeight = 0;
		float m_minHeight = 0;

		friend class NavigationManagerComponent;
	};

}
