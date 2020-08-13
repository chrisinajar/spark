#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/std/containers/bitset.h>

#include "Utils/PathfinderRequest.h"
#include "Utils/SparkNavPath.h"

namespace spark
{
	class SparkNavPath;
	
    class UnitNavigationRequests
		: public AZ::ComponentBus
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        //////////////////////////////////////////////////////////////////////////

        virtual void MoveToPosition(AZ::Vector3 position) = 0;
		virtual void Follow(AZ::EntityId, float distance) = 0;
		virtual void SendPathfindRequest(spark::PathfindRequest request) = 0;
		virtual bool IsMoving() = 0;
		virtual void Stop() = 0;

		virtual void SetPath(const SparkNavPath&) = 0;

		virtual AZ::Vector3 GetPosition() = 0;
		virtual void SetPosition(AZ::Vector3) = 0;

		virtual void SetRotationZ(float) = 0;
		virtual float GetRotationZ() = 0;

		virtual float GetRadius() = 0;
		virtual float GetMovementSpeed() = 0;
		virtual float GetTurnRate() = 0;


		virtual bool IsFacingDirection(AZ::Vector3 dir) = 0; //calls IsFacingDirectionWithTolerance with tolerance = 0.1 radians(~5.7 degrees)
		virtual bool IsFacingDirectionWithTolerance(AZ::Vector3 dir, float tolerance) = 0;
		virtual AZ::Vector3 GetDirection() = 0;
		virtual void SetDirection(AZ::Vector3) = 0;
    };
    using UnitNavigationRequestBus = AZ::EBus<UnitNavigationRequests>;


	class UnitNavigationNotifications
        : public AZ::ComponentBus
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
        //////////////////////////////////////////////////////////////////////////

        // Put your public methods here
		virtual void OnMovingDone() {}
		virtual void OnDestinationUnreachable() {}

		virtual void OnMovingStarted() {}
		virtual void OnPathChanged(const SparkNavPath &path) {}
    };
    using UnitNavigationNotificationBus = AZ::EBus<UnitNavigationNotifications>;
}
