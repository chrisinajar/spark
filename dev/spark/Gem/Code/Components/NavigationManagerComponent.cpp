
#include "spark_precompiled.h"

#include <ISystem.h>
#include <CryAction.h>

#include <AzFramework/Entity/EntityContextBus.h>
#include <AzFramework/Entity/EntityContext.h>
#include <AzCore/Component/TransformBus.h>

#include "NavigationManagerComponent.h"

#include "Components/UnitComponent.h"
#include "Components/UnitNavigationComponent.h"


#include <AzFramework/Network/NetBindingComponent.h>
#include <LmbrCentral/Shape/ShapeComponentBus.h>

#include <AzCore/Jobs/JobFunction.h>

#include "Utils/NavigationUtils.h"
#include "Utils/SetUtils.h"
#include <LmbrCentral/Scripting/TriggerAreaComponentBus.h>
#include <IProximityTriggerSystem.h>
#include <SFunctor.h>

#include "Busses/CameraBus.h"
#include "Busses/UnitBus.h"

namespace spark
{

	

	class MapVisibilityNotificationBusHandler
		: public MapVisibilityNotificationBus::Handler
		, public AZ::BehaviorEBusHandler
	{
	public:
		AZ_EBUS_BEHAVIOR_BINDER(MapVisibilityNotificationBusHandler, "{60DB4AAA-EDF8-4093-89A9-8486B81CA026}", AZ::SystemAllocator, OnUnitVisibilityChange);

		void OnUnitVisibilityChange(UnitId unit, VisibilityEnum visibility)
		{
			Call(FN_OnUnitVisibilityChange, unit, visibility);
		}
	};

	void NavigationManagerComponent::Reflect(AZ::ReflectContext* context)
	{
		if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
		{
			serialize->Class<NavigationManagerComponent, AZ::Component>()
				->Version(0)
				;

			if (AZ::EditContext* ec = serialize->GetEditContext())
			{
				ec->Class<NavigationManagerComponent>("NavigationManagerComponent", "Manage pathfinding")
					->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					->Attribute(AZ::Edit::Attributes::Category, "spark")
					->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"))
					;
			}
		}

		if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
		{

			behaviorContext->Enum<(int)VisibilityEnum::NOT_VISIBLE>("VISIBILITY_NOT_VISIBLE");
			behaviorContext->Enum<(int)VisibilityEnum::VISIBLE>("VISIBILITY_VISIBLE");
			behaviorContext->Enum<(int)VisibilityEnum::TRUE_VISION>("VISIBILITY_TRUE_VISION");
			behaviorContext->Enum<(int)VisibilityEnum::TOTAL_VISION>("VISIBILITY_TOTAL_VISION");

			behaviorContext->EBus<MapRequestBus>("MapRequestBus")
				->Event("GetTerrainHeightAt", &MapRequestBus::Events::GetTerrainHeightAt)
				->Event("GetUnitAt", &MapRequestBus::Events::GetUnitAt)
				->Event("GetNearestFreePosition", &MapRequestBus::Events::GetNearestFreePosition)
				->Event("IsPointVisibleByTeam", &MapRequestBus::Events::IsPointVisibleByTeam)
				->Event("IsPointVisibleByUnit", &MapRequestBus::Events::IsPointVisibleByUnit)
				->Event("IsUnitVisibleByUnit", &MapRequestBus::Events::IsUnitVisibleByUnit)
				->Event("IsUnitVisibleByTeam", &MapRequestBus::Events::IsUnitVisibleByTeam)
				->Event("ShowMapForTeam", &MapRequestBus::Events::ShowMapForTeam)
				//->Event("ShowMapForTeams", &MapRequestBus::Events::ShowMapForTeams)
				->Event("IsUnitVisible", &MapRequestBus::Events::IsUnitVisible)
				->Event("IsPointVisible", &MapRequestBus::Events::IsPointVisible)
				;

			behaviorContext->EBus<MapVisibilityNotificationBus>("MapVisibilityNotificationBus")
				->Handler< MapVisibilityNotificationBusHandler>();
		
		}
	}

	void NavigationManagerComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
	{
		provided.push_back(AZ_CRC("NavigationManagerService"));
	}

	void NavigationManagerComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
	{
		incompatible.push_back(AZ_CRC("NavigationManagerService"));
	}

	void NavigationManagerComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
	{
		(void)required;
	}

	void NavigationManagerComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
	{
		(void)dependent;
	}

	void NavigationManagerComponent::Init()
	{
		//auto console = gEnv->pConsole;
		//ICVar* var = console->GetCVar("g_FogOfWar");

		//if (var)
		//{
		//	//SFunctor onFogOfWarChange;
		//	//onFogOfWarChange.Set(OnFogOfWarChanged, var);
		//	//var->AddOnChangeFunctor(onFogOfWarChange);
		//}
		//else
		//{ 
		//	sERROR("cvar \'g_FogOfWar\' not defined! cannot create the callback");
		//}
	}

	void OnFogOfWarChanged(ICVar * cvar)
	{
		if (!cvar)return;

		//sLOG("OnFogOfWarChanged");
		bool visible = !(cvar->GetIVal() > 0);


		AZStd::vector<UnitId> units;
		EBUS_EVENT(UnitsRequestBus, GetAllUnits, units);

		for (auto u : units)
		{
			EBUS_EVENT_ID(u, UnitRequestBus, SetVisible, visible);
		}
		/*if (cvar && cvar->GetIVal() > 0 && m_currentCameraNavigationArea)
		{
			m_currentCameraNavigationArea->BindVisibilityTexture();
		}*/
	}


	void NavigationManagerComponent::Activate()
	{
		NavigationManagerRequestBus::Handler::BusConnect();
		NavigationAreaNotificationBus::Handler::BusConnect();
		MapRequestBus::Handler::BusConnect();
		AZ::TickBus::Handler::BusConnect();

		auto console = gEnv->pConsole;
		ICVar* var = console->GetCVar("g_FogOfWar");

		if (var)
		{
			SFunctor onFogOfWarChange;
			onFogOfWarChange.Set(OnFogOfWarChanged, var);
			var->AddOnChangeFunctor(onFogOfWarChange);
		}
		else
		{ 
			sERROR("cvar \'g_FogOfWar\' not defined! cannot create the callback");
		}
	}

	void NavigationManagerComponent::Deactivate()
	{
		AZ::TickBus::Handler::BusDisconnect();
		MapRequestBus::Handler::BusDisconnect();
		NavigationManagerRequestBus::Handler::BusDisconnect();
		NavigationAreaNotificationBus::Handler::BusDisconnect();
	}


	void NavigationManagerComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time)
	{
		
		//if (m_navPaths.size())sLOG("m_navPaths size: " + (int)m_navPaths.size());

		for (auto it = m_navPaths.begin(); it != m_navPaths.end();)
		{
			if (NavigationPathNotificationBus::GetNumOfEventHandlers(it->m_ticket) == 0)
			{
				//sLOG("erasing path with ticket=" + it->m_ticket);
				it = m_navPaths.erase(it);
			}
			else
			{
				//check if it need to be updated
				SparkNavPath navPath = *it;

				AZ::Vector3 *current_target = navPath.GetDestinationPoint();
				AZ::Vector3 new_target;
				EBUS_EVENT_ID_RESULT(new_target, navPath.m_request.GetTargetEntityId(), AZ::TransformBus, GetWorldTranslation);

				//if (current_pos)sLOG("distance is = " + Distance2D(*current_pos, new_pos));

				if (current_target && Distance2D(*current_target, new_target) > 3.0f)
				{
					//sLOG("updating path");
					m_navPathsMutex.lock();
					it = m_navPaths.erase(it);
					m_navPathsMutex.unlock();

					UpdatePath(navPath);
				}
				else
				{
					++it;
				}
			}
		}


		//todo:
		//calc velocities based on paths

		//iterate all entities
			//if future collision is detected
				//avoid it ( controlling only the moving units? )


		//iterate all entities
			//apply velocity
			//update transform

		for (auto &it : m_navigationEntities)
		{
			auto &e = it.second;

			if (e->IsMoving()) {

				auto m = e->CalculateMovement(deltaTime);

				//todo:
				//-probably store the movements/velocities inside a container
				//-edit movements/velocities to avoid collisions 

				//N.B. the movement must follow this rule(defined inside the CheckMovement function):
				// (movement.delta_rotation_z/(GetTurnRate()*TURNRATE_SCALE) + movement.delta_movement_forward/(GetMovementSpeed()*MOVEMENT_SCALE)) <= deltaTime

				//N.B. MovementInfo can be converted to Velocity vector using e->ConvertToVelocity(m)


				if (!e->CheckMovement(m, deltaTime))sLOG("test not passed");

				//use convertovelocity and run collision checking here
				//that or modify calculatemovement


				e->ApplyMovement(m);
			}

			e->UpdateTransform();
		}


		AZ::EntityId cameraId;
		EBUS_EVENT_RESULT(cameraId, CameraRequestBus, GetCameraEntityId);

		if (cameraId.IsValid())
		{
			AZ::Vector3 pos;
			EBUS_EVENT_ID_RESULT(pos, cameraId, AZ::TransformBus, GetWorldTranslation);

			auto currentNavArea = GetContainingNavigationArea(pos);

			if (currentNavArea && m_currentCameraNavigationArea != currentNavArea)
			{
				m_currentCameraNavigationArea = currentNavArea;
				m_currentCameraNavigationArea->BindMiniMapTexture();
				m_currentCameraNavigationArea->BindVisibilityTexture();

				EBUS_EVENT(NavigationAreaNotificationBus, OnMainNavigationAreaChanged, m_currentCameraNavigationArea->GetEntityId());
			}

			if (m_currentCameraNavigationArea)
			{
				//update visibility texture
				auto console = gEnv->pConsole;
				if (console)
				{
					ICVar* var = console->GetCVar("g_FogOfWar");

					if (var && var->GetIVal() > 0)
					{
						m_currentCameraNavigationArea->ClearVisibilityField();
						m_currentCameraNavigationArea->UpdateVisibilityFieldForTeams(m_viewTeams);
						m_currentCameraNavigationArea->UpdateVisibilityTexture();
					}

					var = console->GetCVar("g_DebugTiles");
					
					if (var && var->GetIVal() > 0)
					{
						m_currentCameraNavigationArea->DrawTiles();
					}
				}
			}
		}
	}



	void NavigationManagerComponent::OnNavigationAreaActivated(AZ::Entity* e)
	{
		NavigationAreaComponent *navArea = e->FindComponent<NavigationAreaComponent>();
		if (!navArea)return;

		auto it = AZStd::find(m_navigationAreas.begin(), m_navigationAreas.end(), navArea);

		if (it == m_navigationAreas.end())
		{
			m_navigationAreas.push_back(navArea);

			//if (!m_currentCameraNavigationArea)m_currentCameraNavigationArea = navArea;
		}
	}

	void NavigationManagerComponent::OnNavigationAreaDeactivated(AZ::Entity* e)
	{
		NavigationAreaComponent *navArea = e->FindComponent<NavigationAreaComponent>();
		if (!navArea)return;

		auto it = AZStd::find(m_navigationAreas.begin(), m_navigationAreas.end(), navArea);

		if (it != m_navigationAreas.end())
		{
			m_navigationAreas.erase(it);
		}
	}

	void NavigationManagerComponent::OnPositionChanged(AZ::Vector3 previous, AZ::Vector3 current)
	{
		const AZ::EntityId id = *NavigationEntityNotificationBus::GetCurrentBusId();

		//auto navEntity = m_navigationEntities[id];
		//auto navArea = GetContainingNavigationArea(id);

		//if (navEntity->m_lastAabb.navigationArea != navArea->GetEntityId())
		//{
		//	OnNavigationEntityEvent(id, previous, current, NavigationEntityRemoved);
		//	OnNavigationEntityEvent(id, previous, current, NavigationEntityAdded);
		//	return;
		//}

		OnNavigationEntityEvent(id, previous, current, NavigationEntityMoved);
	}

	void NavigationManagerComponent::OnShapeChanged(LmbrCentral::ShapeComponentNotifications::ShapeChangeReasons changeReason)
	{
		if (changeReason == LmbrCentral::ShapeComponentNotifications::ShapeChangeReasons::ShapeChanged)
		{
			const AZ::EntityId id = *LmbrCentral::ShapeComponentNotificationsBus::GetCurrentBusId();

			AZ::Vector3 pos;
			EBUS_EVENT_ID_RESULT(pos, id, NavigationEntityRequestBus, GetPosition);

			OnNavigationEntityEvent(id, AZ::Vector3(), pos, NavigationEntityMoved);
		}
	}

	void NavigationManagerComponent::OnNavigationEntityEvent(AZ::EntityId id, AZ::Vector3 previous, AZ::Vector3 current, NavigationEntityEvent e)
	{
		using namespace SetUtils;

		auto navEntity = m_navigationEntities[id];
		auto &lastAabb = navEntity->m_lastAabb;
		auto navArea = GetContainingNavigationArea(id);
		bool isMovingTriggerArea = false;
		EBUS_EVENT_ID_RESULT(isMovingTriggerArea, id, SparkTriggerAreaEntityRequestBus, IsSparkTriggerArea);

		AZStd::set<AZ::EntityId> entered_triggerAreas;
		AZStd::set<AZ::EntityId> exited_triggerAreas;
		AZStd::set<AZ::EntityId> unchanged_triggerAreas;

		AZStd::set<AZ::EntityId> entities_exited;
		AZStd::set<AZ::EntityId> entities_entered;
		AZStd::set<AZ::EntityId> entities_unchanged;

		int blocking_flag = 0;
		EBUS_EVENT_ID_RESULT(blocking_flag, id, NavigationEntityRequestBus, IsBlocking);

		AZ::Aabb aabb;
		//bool aabb_requested = false;

		pathfinder::coordinates min, max;

		if (e & NavigationEntityRemoved)
		{
			NavigationAreaComponent* navArea = nullptr;
			for (auto n : m_navigationAreas)
			{
				if (n->GetEntityId() == lastAabb.navigationArea)
				{
					navArea = n;
					break;
				}
			}

			if (navArea)
			{
				//if (navArea->GetEntityId() == lastAabb.navigationArea) //old aabb is cached
				{
					min = lastAabb.min;
					max = lastAabb.max;
				}
				//else
				//{
				//	EBUS_EVENT_ID_RESULT(aabb, id, LmbrCentral::ShapeComponentRequestsBus, GetEncompassingAabb);
				//	aabb_requested = true;

				//	//clear the old position
				//	if (aabb.IsValid())
				//	{
				//		min = navArea->ConvertToGridCoordinates(aabb.GetMin() - current + previous);
				//		max = navArea->ConvertToGridCoordinates(aabb.GetMax() - current + previous);
				//	}
				//	else
				//	{
				//		min = max = navArea->ConvertToGridCoordinates(previous);
				//	}
				//}

				RasterizeAABB(min, max, [&](pathfinder::coordinates c)
				{

					auto* cell = navArea->m_unitField.GetCellSecure(c);
					if (cell)
					{
						bool isTriggerArea;
						

						for (auto e : *cell)
						{
							if (e == id)continue;

							if (isMovingTriggerArea)
							{
								entities_exited.insert(e);
							}

							isTriggerArea = false;
							EBUS_EVENT_ID_RESULT(isTriggerArea, e, SparkTriggerAreaEntityRequestBus, IsSparkTriggerArea);

							if (isTriggerArea)
							{
								exited_triggerAreas.insert(e);
							}
						}
						cell->erase(id);
					}

					if (blocking_flag & BlockingFlag::MOVEMENT_BLOCKING)
					{
						auto blocking_cell = navArea->m_movementBlockingField.GetCellSecure(c);
						if (blocking_cell)*blocking_cell = AZStd::max(0, *blocking_cell - 1);
					}

					if (blocking_flag & BlockingFlag::VISION_BLOCKING)
					{
						auto blocking_cell = navArea->m_visionBlockingField.GetCellSecure(c);
						if (blocking_cell)*blocking_cell = AZStd::max(0, *blocking_cell - 1);
					}
				});
			}
		}


		if (navArea && (e & NavigationEntityAdded))
		{
			//if (!aabb_requested)
			//{
			EBUS_EVENT_ID_RESULT(aabb, id, LmbrCentral::ShapeComponentRequestsBus, GetEncompassingAabb);
			//}

			//set the new position
			if (aabb.IsValid())
			{
				min = navArea->ConvertToGridCoordinates(aabb.GetMin());
				max = navArea->ConvertToGridCoordinates(aabb.GetMax());
			}
			else
			{
				min = max = navArea->ConvertToGridCoordinates(current);
			}

			lastAabb.min = min;
			lastAabb.max = max;
			lastAabb.navigationArea = navArea->GetEntityId();

			RasterizeAABB(min, max, [&](pathfinder::coordinates c)
			{
				auto* cell = navArea->m_unitField.GetCellSecure(c);
				if (cell)
				{
					bool isTriggerArea;

					cell->insert(id);

					for (auto e : *cell)
					{
						if (e == id)continue;

						if (isMovingTriggerArea)
						{
							entities_entered.insert(e);
						}

						isTriggerArea = false;
						EBUS_EVENT_ID_RESULT(isTriggerArea, e, SparkTriggerAreaEntityRequestBus, IsSparkTriggerArea);

						if (isTriggerArea)
						{
							entered_triggerAreas.insert(e);
						}
						
					}
					
				}
				if (blocking_flag & BlockingFlag::MOVEMENT_BLOCKING)
				{
					auto blocking_cell = navArea->m_movementBlockingField.GetCellSecure(c);
					if (blocking_cell)++*blocking_cell;
				}

				if (blocking_flag & BlockingFlag::VISION_BLOCKING)
				{
					auto blocking_cell = navArea->m_visionBlockingField.GetCellSecure(c);
					if (blocking_cell)++*blocking_cell;
				}
			});

		}

		AZStd::set<AZ::EntityId> already_inside_triggerAreas;
		AZStd::set<AZ::EntityId> colliding_triggerAreas;
		AZStd::set<AZ::EntityId> concerned_triggerAreas = set_union(exited_triggerAreas, entered_triggerAreas);

		for (auto t : exited_triggerAreas)
		{
			bool inside = false;
			EBUS_EVENT_ID_RESULT(inside, t, SparkTriggerAreaEntityRequestBus, IsEntityInside, id);
			if (inside)already_inside_triggerAreas.insert(t);
		}

		for (auto t : concerned_triggerAreas)
		{
			bool inside = false;
			EBUS_EVENT_ID_RESULT(inside, t, SparkTriggerAreaEntityRequestBus, NarrowPassCheck, current);
			if (inside)colliding_triggerAreas.insert(t);
		}

		for (auto t : set_difference(already_inside_triggerAreas, colliding_triggerAreas))
		{
			EBUS_EVENT_ID(t, ProximityTriggerEventBus, OnTriggerExit, id);
		}

		for (auto t : set_difference(colliding_triggerAreas, already_inside_triggerAreas))
		{
			EBUS_EVENT_ID(t, ProximityTriggerEventBus, OnTriggerEnter, id);
		}


		if (isMovingTriggerArea)
		{

			AZStd::set<AZ::EntityId> already_inside;
			AZStd::set<AZ::EntityId> colliding;
			AZStd::set<AZ::EntityId> concerned = entities_entered;


			EBUS_EVENT_ID_RESULT(already_inside, id, SparkTriggerAreaEntityRequestBus, GetAllEntitiesInside);

			for (auto t : concerned)
			{
				bool inside = true;// false;
				AZ::Vector3 pos;
				EBUS_EVENT_ID_RESULT(pos, t, NavigationEntityRequestBus, GetPosition);
				//EBUS_EVENT_ID_RESULT(inside, id, SparkTriggerAreaEntityRequestBus, NarrowPassCheck, pos);
				if (inside)colliding.insert(t);
			}

			for (auto t : set_difference(already_inside, colliding))
			{
				EBUS_EVENT_ID(id, ProximityTriggerEventBus, OnTriggerExit, t);
			}

			for (auto t : set_difference(colliding, already_inside))
			{
				EBUS_EVENT_ID(id, ProximityTriggerEventBus, OnTriggerEnter, t);
			}
		}



		//

		//if (isMovingTriggerArea)
		//{
		//	for (auto t : entities_entered)if (entities_exited.find(t) != entities_exited.end())entities_unchanged.insert(t);//set intersection

		//	for (auto t : entities_unchanged)
		//	{
		//		entities_entered.erase(t);
		//		entities_exited.erase(t);
		//	}

		//	for (auto t : entities_exited)
		//	{
		//		EBUS_EVENT_ID(id, ProximityTriggerEventBus, OnTriggerExit, t);
		//	}

		//	for (auto t : entities_entered)
		//	{
		//		EBUS_EVENT_ID(id, ProximityTriggerEventBus, OnTriggerEnter, t);
		//	}
		//}
	}

	NavigationPathTicket NavigationManagerComponent::CalculatePath(AZ::EntityId id, spark::PathfindRequest request)
	{
		if (!id.IsValid())
		{
			sLOG("NavigationManagerComponent::CalculatePath  invalid entityId");
			return -1;
		}

		if (!request.IsValid())
		{
			sLOG("NavigationManagerComponent::CalculatePath  invalid request");
			return -1;
		}

		if (m_navigationAreas.empty())
		{
			sLOG("NavigationManagerComponent::CalculatePath  navigationArea not found");
			return -1;
		}
		sLOG("NavigationManagerComponent::CalculatePath() calculating path to " + request.GetDestinationLocation());

		NavigationPathTicket ticket = m_nextTicket++;

		SparkNavPath path;
		path.m_request = request;
		path.m_navigationEntity = id;
		path.m_ticket = ticket;

		UpdatePath(path);

		return ticket;
	}

	void NavigationManagerComponent::UpdatePath(SparkNavPath path)
	{
		AZ::EntityId id = path.m_navigationEntity;
		auto request = path.m_request;
		auto ticket = path.m_ticket;

		AZ::Vector3 pos;
		EBUS_EVENT_ID_RESULT(pos, id, AZ::TransformBus, GetWorldTranslation);

		AZ::Vector3 target;
		AZ::EntityId targetEntity;
		if (request.HasTargetLocation())
		{
			target = request.GetDestinationLocation();
		}
		else
		{
			targetEntity = request.GetTargetEntityId();
			EBUS_EVENT_ID_RESULT(target, targetEntity, AZ::TransformBus, GetWorldTranslation);
		}

		Status status = 0;
		EBUS_EVENT_ID_RESULT(status, id, UnitRequestBus, GetStatus);

		NavigationAreaComponent* navArea = GetContainingNavigationArea(id);	//todo handle different navigation areas (check if current position and target are on the same area, no need for areas to intersect: e.g. one area for each arena)

		if (navArea)
		{
			AZ::CreateJobFunction([navArea, id, request, ticket, pos, target, status, targetEntity]() {
				//sLOG("NavigationManagerComponent::CalculatePath() job started");

				pathfinder::Path gridPath;

				pathfinder::coordinates from = navArea->ConvertToGridCoordinates(pos);
				pathfinder::coordinates to = navArea->ConvertToGridCoordinates(target);

				int path_found = pathfinder::CalculatePath(gridPath, from, to, { 0,0 }, navArea->m_heightField.GetSize(),
					[id, targetEntity, navArea, status](pathfinder::coordinates from, pathfinder::coordinates to)->bool {

					float cost = from.distance(to);

					if (status & FLYING_MOVEMENT)
					{
						return true;// cost;
					}

					//auto *fromHeight = navArea->m_heightField.GetCellSecure(from);
					auto *toHeight = navArea->m_heightField.GetCellSecure(to);
					if (!toHeight || *toHeight == InvalidHeight)return false;// pathfinder::ProhibitiveCost;
					//if (fromHeight && toHeight)return fabsf(*fromHeight - *toHeight) < navArea->m_maxSlope; //max slope between two adjacent tiles

					if (!(status & PHASED)) {

						auto blocking = navArea->m_movementBlockingField.GetCellSecure(to);
						if (!blocking || *blocking>0)return false;
						
						//auto *entities = navArea->m_unitField.GetCellSecure(to);

						////for (auto u : *unit)//check if the cell is obstructed by another unit
						////{
						////	if (u != id && u != targetEntity)return false;
						////}
						//if (entities)
						//{
						//	for (auto e : *entities)
						//	{
						//		if (e == id || e == targetEntity)continue;

						//		int blocking = 0;
						//		EBUS_EVENT_ID_RESULT(blocking, e, NavigationEntityRequestBus, IsBlocking);
						//		if (blocking & BlockingFlag::MOVEMENT_BLOCKING)return false;
						//		/*if (occupied == 1)return pathfinder::ProhibitiveCost;
						//		if (occupied == 2)cost = pathfinder::ProhibitiveCost - 1;*/
						//	}

						//}
					}

					return true;// cost;
				});


				//sLOG("NavigationManagerComponent::CalculatePath() path found :" + path_found);

				if (path_found)
				{
					SparkNavPath navPath;
					for (auto c : gridPath)
					{
						navPath.m_pathPoints.push_back(navArea->ConvertToWorldCoordinates(c, AZ::Vector3(0.5, 0.5, 0)));
					}

					if (path_found == 1) {
						*navPath.m_pathPoints.begin() = pos;
						*navPath.m_pathPoints.rbegin() = target;
					}
					else
					{
						sLOG("path not found, using the path to the closest point");
					}

					navPath.m_navigationEntity = id;
					navPath.m_ticket = ticket;
					navPath.m_request = request;
					navPath.m_valid = true;

					EBUS_EVENT_ID(ticket, NavigationPathNotificationBus, OnPathFound, request, navPath);

					/*if (request.HasTargetEntity())
					{
						m_navPathsMutex.lock();
						m_navPaths.push_back(navPath);
						m_navPathsMutex.unlock();
					}*/
				}
				else
				{
					EBUS_EVENT_ID(ticket, NavigationPathNotificationBus, OnPathNotFound, request);
				}

			}, true)->Start();
		}
		else
		{
			EBUS_EVENT_ID(ticket, NavigationPathNotificationBus, OnPathNotFound, request);
		}
	}

	void NavigationManagerComponent::AddNavigationEntity(NavigationEntity *e)
	{
		//auto it = AZStd::find(m_navigationEntities.begin(), m_navigationEntities.end(), e);
		auto id = e->GetNavigationEntityId();
		auto it = m_navigationEntities.find(id);

		if (it == m_navigationEntities.end()) {
			//m_navigationEntities.push_back(e);
			e->SetCurrentNavigationArea(m_navigationAreas[0]->GetEntityId());

			m_navigationEntities[id] = e;
			NavigationEntityNotificationBus::MultiHandler::BusConnect(id);
			LmbrCentral::ShapeComponentNotificationsBus::MultiHandler::BusConnect(id);

			AZ::Vector3 pos;
			EBUS_EVENT_ID_RESULT(pos, id, NavigationEntityRequestBus, GetPosition);

			OnNavigationEntityEvent(id, AZ::Vector3(), pos, NavigationEntityAdded);
		}
	}

	void NavigationManagerComponent::RemoveNavigationEntity(NavigationEntity *e)
	{
		//auto it = AZStd::find(m_navigationEntities.begin(), m_navigationEntities.end(), e);
		auto id = e->GetNavigationEntityId();
		auto it = m_navigationEntities.find(id);

		if (it != m_navigationEntities.end()) {
			AZ::Vector3 pos;
			EBUS_EVENT_ID_RESULT(pos, id, NavigationEntityRequestBus, GetPosition);

			OnNavigationEntityEvent(id, pos, pos, NavigationEntityRemoved);

			e->m_lastAabb.navigationArea.SetInvalid();
			m_navigationEntities.erase(it);
			NavigationEntityNotificationBus::MultiHandler::BusDisconnect(id);
			LmbrCentral::ShapeComponentNotificationsBus::MultiHandler::BusDisconnect(id);
		}
	}

	void NavigationManagerComponent::SetEntityPath(AZ::EntityId id, SparkNavPath path)
	{
		if (path.m_request.IsValid() && path.m_request.HasTargetEntity())
		{
			m_navPathsMutex.lock();
			m_navPaths.push_back(path);
			m_navPathsMutex.unlock();
		}
	}


	NavigationAreaComponent* NavigationManagerComponent::GetContainingNavigationArea(UnitId unit)
	{
		for (auto &a : m_navigationAreas)
		{
			if (a->IsUnitInside(unit))return a;
		}
		return nullptr;
	}

	NavigationAreaComponent* NavigationManagerComponent::GetContainingNavigationArea(AZ::Vector3 pos)
	{
		for (auto &a : m_navigationAreas)
		{
			if (a->IsPointInside(pos))return a;
		}
		return nullptr;
	}

	float NavigationManagerComponent::GetTerrainHeightAt(AZ::Vector3 pos)//z coordinate is ignored
	{
		NavigationAreaComponent* navArea = GetContainingNavigationArea(pos);
		if (navArea)
		{
			return navArea->GetTerrainHeightAt(pos);
		}
		return 0;
	}

	AZ::Vector3 NavigationManagerComponent::GetNearestFreePosition(AZ::Vector3 pos)
	{
		NavigationAreaComponent* navArea = GetContainingNavigationArea(pos);
		if (navArea)
		{
			return navArea->GetNearestFreePosition(pos);
		}
		return pos;
	}



	UnitId NavigationManagerComponent::GetUnitAt(AZ::Vector3 pos)
	{
		NavigationAreaComponent* navArea = GetContainingNavigationArea(pos);
		if (navArea)
		{
			return navArea->GetUnitAt(pos);
		}
		return UnitId();
	}

	AZStd::vector<UnitId> NavigationManagerComponent::GetUnitsAt(AZ::Vector3 pos)
	{
		NavigationAreaComponent* navArea = GetContainingNavigationArea(pos);
		if (navArea)
		{
			return navArea->GetUnitsAt(pos);
		}
		return AZStd::vector<UnitId>();
	}

	VisibilityEnum NavigationManagerComponent::IsPointVisibleByTeam(AZ::Vector3 pos, TeamId teamId)
	{
		NavigationAreaComponent* navArea = GetContainingNavigationArea(pos);
		if (navArea)
		{
			return navArea->IsPointVisibleByTeam(pos, teamId);
		}
		return (VisibilityEnum)0;
	}

	VisibilityEnum NavigationManagerComponent::IsPointVisibleByUnit(AZ::Vector3 pos, UnitId u)
	{
		NavigationAreaComponent* navArea = GetContainingNavigationArea(pos);
		if (navArea)
		{
			return navArea->IsPointVisibleByUnit(pos, u);
		}
		return (VisibilityEnum)0;
	}

	bool NavigationManagerComponent::IsUnitVisibleByUnit(UnitId target, UnitId u)
	{
		NavigationAreaComponent* navArea = GetContainingNavigationArea(target);
		if (navArea)
		{
			return navArea->IsUnitVisibleByUnit(target, u);
		}
		return false;
	}

	bool NavigationManagerComponent::IsUnitVisibleByTeam(UnitId target, TeamId teamId)
	{
		NavigationAreaComponent* navArea = GetContainingNavigationArea(target);
		if (navArea)
		{
			return navArea->IsUnitVisibleByTeam(target, teamId);
		}
		return false;
	}


	void NavigationManagerComponent::ShowMapForTeam(TeamId team)
	{
		m_viewTeams.clear();
		m_viewTeams.insert(team);
	}
	void NavigationManagerComponent::ShowMapForTeams(AZStd::set<TeamId> teams)
	{
		m_viewTeams = teams;
	}


	bool NavigationManagerComponent::IsUnitVisible(UnitId u)
	{
		NavigationAreaComponent* navArea = GetContainingNavigationArea(u);
		if (navArea)
		{
			for (auto t : m_viewTeams)
			{
				if (navArea->IsUnitVisibleByTeam(u, t))return true;
			}
		}
		return false;
	}

	bool NavigationManagerComponent::IsPointVisible(AZ::Vector3 pos)
	{
		NavigationAreaComponent* navArea = GetContainingNavigationArea(pos);
		if (navArea)
		{
			for (auto t : m_viewTeams)
			{
				if (navArea->IsPointVisibleByTeam(pos, t))return true;
			}
		}
		return false;
	}

}
