#pragma once

#include "Busses/NavigationManagerBus.h"

#include <AzCore/Component/Component.h>
#include <AzCore/Component/ComponentBus.h>
#include <AzCore/Component/TickBus.h>
#include <LmbrCentral/Shape/ShapeComponentBus.h>

#include <AzCore/std/containers/map.h>
#include <AzCore/std/containers/vector.h>

#include "NavigationAreaComponent.h"

namespace spark
{
	

	class NavigationManagerComponent
	:   public AZ::Component,
		public AZ::TickBus::Handler,
		public NavigationAreaNotificationBus::Handler,
		public NavigationManagerRequestBus::Handler,
		public MapRequestBus::Handler,
		public NavigationEntityNotificationBus::MultiHandler,
		private LmbrCentral::ShapeComponentNotificationsBus::MultiHandler
	{
	public:
		AZ_COMPONENT(NavigationManagerComponent, "{4478615F-F6EC-4FBC-B407-73B5020BB5B4}");
        static void Reflect(AZ::ReflectContext* context);

	protected:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

		////////////////////////////////////////////////////////////////////////
		// AZ::Component interface implementation
		void Init() override;
		void Activate() override;
		void Deactivate() override;
		////////////////////////////////////////////////////////////////////////

    public:

		//#####// NavigationManagerRequestBus //#####//

		NavigationPathTicket CalculatePath(AZ::EntityId, spark::PathfindRequest);
		void UpdatePath(SparkNavPath);


		void AddNavigationEntity(NavigationEntity*);
		void RemoveNavigationEntity(NavigationEntity*);

		void SetEntityPath(AZ::EntityId id, SparkNavPath path);

		//#####// MapBus //#####//

		float GetTerrainHeightAt(AZ::Vector3); //z coordinate is ignored
		AZ::Vector3 GetNearestFreePosition(AZ::Vector3);

		//visibility (works the same for everyone)
		UnitId GetUnitAt(AZ::Vector3);
		AZStd::vector<UnitId> GetUnitsAt(AZ::Vector3);
		VisibilityEnum IsPointVisibleByTeam(AZ::Vector3, TeamId);
		VisibilityEnum IsPointVisibleByUnit(AZ::Vector3, UnitId);
		bool IsUnitVisibleByUnit(UnitId, UnitId);
		bool IsUnitVisibleByTeam(UnitId, TeamId);

		//visibility - but is based on the view ( like in the model-view-controller pattern, the view being the team of the player, or the teams that the spectator want to see)
		void ShowMapForTeam(TeamId team); //set the view
		void ShowMapForTeams(AZStd::set<TeamId> teams);

		bool IsUnitVisible(UnitId);
		bool IsPointVisible(AZ::Vector3);


	private:

		void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

		void OnNavigationAreaActivated(AZ::Entity* e);
		void OnNavigationAreaDeactivated(AZ::Entity* e);

		//NavigationEntityNotificationBus
		void OnPositionChanged(AZ::Vector3 previous, AZ::Vector3 current);
		void OnShapeChanged(LmbrCentral::ShapeComponentNotifications::ShapeChangeReasons changeReason);

		//internal
		enum NavigationEntityEvent{
			NavigationEntityAdded=1,
			NavigationEntityRemoved =2,
			NavigationEntityMoved = 3
		};
		void OnNavigationEntityEvent(AZ::EntityId id, AZ::Vector3 previous, AZ::Vector3 current, NavigationEntityEvent e);

		NavigationAreaComponent* GetContainingNavigationArea(UnitId unit);
		NavigationAreaComponent* GetContainingNavigationArea(AZ::Vector3 pos);

		//static void OnFogOfWarChanged(ICVar* cvar);

		NavigationPathTicket m_nextTicket = 0;

		NavigationAreaComponent* m_currentCameraNavigationArea = nullptr;

		AZStd::vector<NavigationAreaComponent*> m_navigationAreas;
		//AZStd::vector<NavigationEntity*> m_navigationEntities;
		AZStd::unordered_map < AZ::EntityId, NavigationEntity*> m_navigationEntities;

		AZStd::vector<SparkNavPath> m_navPaths;
		AZStd::mutex m_navPathsMutex;


		AZStd::set<TeamId> m_viewTeams;
		//AZStd::unordered_map < AZ::EntityId, NavigationEntity> m_navigationEntities;
		
		/*AZStd::vector<spark::NavigationEntity> m_navigationEntities;
		AZStd::unordered_map<AZ::EntityId, int> m_navigationEntitiesMap;*/
	};


}