#pragma once

#include <AzCore/Component/Component.h>

#include <AzCore/Component/TickBus.h>
#include <AzFramework/Entity/GameEntityContextBus.h>
#include <LmbrCentral/Ai/NavigationComponentBus.h>
#include "Busses/UnitNavigationBus.h"
#include "Utils/SparkNavPath.h"
#include "Busses/NavigationManagerBus.h"
#include <AzCore/Component/TransformBus.h>


#include "Utils/NavigationUtils.h"

namespace spark
{


    class UnitNavigationComponent
        : public AZ::Component
        //, public AZ::TickBus::Handler
		//, public AZ::TransformNotificationBus::Handler
        , public UnitNavigationRequestBus::Handler
		, public NavigationEntityRequestBus::Handler
		, public NavigationPathNotificationBus::Handler
        //, protected LmbrCentral::NavigationComponentNotificationBus::Handler
    {
    public:
        AZ_COMPONENT(UnitNavigationComponent, "{54F4C316-6E75-45A5-98F3-DD38DFBBFF57}")

        ~UnitNavigationComponent() override {};

        static void Reflect(AZ::ReflectContext* reflection);

		static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
		static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
		static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
		static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        void Init() override;
        void Activate() override;
        void Deactivate() override;

		//UnitNavigationRequestBus
        void MoveToPosition(AZ::Vector3 position) override;
		void Follow(AZ::EntityId, float distance) override;
		
		void SendPathfindRequest(PathfindRequest request);

		//NavigationEntityRequestBus
		void SetPath(const SparkNavPath& path);
		bool IsMoving();
        void Stop() override;

		MovementInfo CalculateMovement(float deltaTime);
		void ApplyMovement(MovementInfo);
		bool CheckMovement(MovementInfo, float deltaTime);
		AZ::Vector3 ConvertToVelocity(MovementInfo);
		void UpdateTransform();
		

		bool IsFacingDirection(AZ::Vector3 dir); //calls IsFacingDirectionWithTolerance with tolerance = 0.1 radians(~5.7 degrees)
		bool IsFacingDirectionWithTolerance(AZ::Vector3 dir, float tolerance);
		AZ::Vector3 GetDirection();
		void SetDirection(AZ::Vector3);

		AZ::Vector3 GetPosition();
		void SetPosition(AZ::Vector3);

		void SetRotationZ(float);
		float GetRotationZ();

		float GetRadius();
		float GetMovementSpeed();
		float GetTurnRate();

		int IsBlocking();
		void SetBlocking(int);

		void AddToNavigationManager();
		void RemoveFromNavigationManager();

		void SetCurrentNavigationArea(AZ::EntityId);
		AZ::EntityId GetCurrentNavigationArea();

		AZ::EntityId GetNavigationEntityId() { return GetEntityId(); }
		bool CollideWith(AZ::EntityId);
    protected:
        //void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

		//void OnTransformChanged(const AZ::Transform& local, const AZ::Transform& world) override;
	protected:
		void OnPathFound(const spark::PathfindRequest &request, const SparkNavPath &path);
		void OnPathNotFound(const spark::PathfindRequest &request);
    private:
		void Done();

		bool HasArrivedToDestination();

		float GetMaxTurnMovement(float &deltaTime, float &currentAngle, float targetAngle);
		float GetMaxForwardMovement(float &deltaTime, AZ::Vector3 &currentPosition, AZ::Vector3 targetPosition);


        static bool ShouldTurnRight(float a1, float a2);

        //AZStd::shared_ptr<INavPath> 
		SparkNavPath m_currentPath;
		AZStd::mutex m_pathMutex;
		float m_currentRotation = 0;

		bool m_blocking = 0;
		int m_flag = 0;

        float m_movementSpeed = 300;
        float m_turnRate = .5f;
		bool m_ignoreTransformEvent = false;	

		MovementInfo m_lastMovement;
		AZ::EntityId m_currentNavigationArea;
    };

	using NavigationEntityComponent = UnitNavigationComponent;
};
