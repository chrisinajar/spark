#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/ComponentBus.h>

#include "NavigationAreaBus.h"
#include "Utils/SparkNavPath.h"
#include "Utils/PathfinderRequest.h"
#include "Utils/PathfinderVec2.h"
#include "Utils/SetUtils.h"

namespace spark
{
	using NavPath = SparkNavPath;

	struct MovementInfo
	{
		float delta_rotation_z=0.0f;
		float delta_movement_forward=0.0f;
	};


	struct Aabb2DSnapshot
	{
		pathfinder::coordinates min, max;
		AZ::EntityId navigationArea;
	};

	enum BlockingFlag
	{
		MOVEMENT_BLOCKING = BIT(0),
		VISION_BLOCKING = BIT(1),
		SPAWN_BLOCKING = BIT(2),
	};

	class NavigationEntityRequests
		: public AZ::ComponentBus
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;

		using MutexType = AZStd::mutex;

		//orders
		virtual void SetPath(const NavPath&) = 0;
		virtual bool IsMoving() = 0;
		virtual void Stop() = 0;

		//movement related
		virtual MovementInfo CalculateMovement(float deltaTime) = 0;
		virtual void ApplyMovement(MovementInfo) = 0;
		virtual bool CheckMovement(MovementInfo, float deltaTime) = 0;
		virtual AZ::Vector3 ConvertToVelocity(MovementInfo) = 0;

		virtual void UpdateTransform() = 0; 

		//getter/setter
		virtual AZ::Vector3 GetPosition() = 0;
		virtual void SetPosition(AZ::Vector3) = 0;

		virtual void SetRotationZ(float) = 0;
		virtual float GetRotationZ() = 0;

		virtual float GetRadius() = 0;
		virtual float GetMovementSpeed() = 0;
		virtual float GetTurnRate() = 0;

		virtual int IsBlocking() = 0;
		virtual void SetBlocking(int) = 0;

		virtual void SetCurrentNavigationArea(AZ::EntityId) = 0; //c++ only
		virtual AZ::EntityId GetCurrentNavigationArea() = 0;

		//checks
		virtual bool IsFacingDirection(AZ::Vector3 dir) = 0; //calls IsFacingDirectionWithTolerance with tolerance = 0.1 radians(~5.7 degrees)
		virtual bool IsFacingDirectionWithTolerance(AZ::Vector3 dir, float tolerance) = 0;
		virtual AZ::Vector3 GetDirection() = 0;
		virtual void SetDirection(AZ::Vector3) = 0;

		//enable/disable
		virtual void AddToNavigationManager() = 0;
		virtual void RemoveFromNavigationManager() = 0;

		//internal 
		virtual AZ::EntityId GetNavigationEntityId() = 0;

		//todo collision for multiple shapes
		bool m_addOnActivate = true;
		Aabb2DSnapshot m_lastAabb;

		virtual bool CollideWith(AZ::EntityId) = 0;
	};
	using NavigationEntityRequestBus = AZ::EBus<NavigationEntityRequests>;

	using NavigationEntity = NavigationEntityRequestBus::Handler;

	class SparkTriggerAreaEntityRequests
		: public AZ::ComponentBus
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;

		virtual bool IsSparkTriggerArea() { return true; }
		virtual bool NarrowPassCheck(const AZ::Vector3& position) = 0;
		virtual AZStd::vector<AZ::EntityId> GetEntitiesInside() = 0;
		virtual AZStd::set<AZ::EntityId> GetAllEntitiesInside() = 0;
		virtual bool IsEntityInside(AZ::EntityId) = 0;
	};
	using SparkTriggerAreaEntityRequestBus = AZ::EBus<SparkTriggerAreaEntityRequests>;


	class NavigationEntityNotifications
		: public AZ::ComponentBus
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;

		virtual void OnPositionChangedFilter(AZ::Vector3 current,AZ::Vector3 proposed) {}
		virtual void OnPositionChanged(AZ::Vector3 previous, AZ::Vector3 current) {}

		virtual void OnRotationZChangedFilter(float current, float proposed) {}
		virtual void OnRotationZChanged(float previous, float current) {}
	};
	using NavigationEntityNotificationBus = AZ::EBus<NavigationEntityNotifications>;

	class NavigationEntitiesNotifications
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;

		virtual void OnNavigationEntityPositionChangedFilter(AZ::EntityId, AZ::Vector3 current, AZ::Vector3 proposed) {}
		virtual void OnNavigationEntityPositionChanged(AZ::EntityId, AZ::Vector3 previous, AZ::Vector3 current) {}

		virtual void OnNavigationEntityRotationZChangedFilter(AZ::EntityId, float current, float proposed) {}
		virtual void OnNavigationEntityRotationZChanged(AZ::EntityId, float previous, float current) {}
	};
	using NavigationEntitiesNotificationBus = AZ::EBus<NavigationEntitiesNotifications>;


	class NavigationManagerRequests
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
		
		using MutexType = AZStd::mutex;

		virtual NavigationPathTicket CalculatePath(AZ::EntityId, spark::PathfindRequest) = 0;

		virtual void AddNavigationEntity(NavigationEntity*) = 0;
		virtual void RemoveNavigationEntity(NavigationEntity*) = 0;

		virtual void SetEntityPath(AZ::EntityId, SparkNavPath) = 0;
	};
	using NavigationManagerRequestBus = AZ::EBus<NavigationManagerRequests>;


	class NavigationPathNotifications 
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
		using BusIdType = NavigationPathTicket;

		using MutexType = AZStd::recursive_mutex;

		virtual void OnPathFound(const spark::PathfindRequest &request,const NavPath &path) {}
		virtual void OnPathNotFound(const spark::PathfindRequest &request){}
	};
	using NavigationPathNotificationBus = AZ::EBus<NavigationPathNotifications>;


}
