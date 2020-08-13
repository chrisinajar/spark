
#include "spark_precompiled.h"

#include "UnitNavigationComponent.h"

#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Math/Transform.h>


#include "Busses/UnitBus.h"
#include "Busses/VariableBus.h"
#include "Busses/NetSyncBusses.h"
#include "Busses/MapBus.h"

#include "Utils/SparkNavPath.h"
#include "Utils/Marshaler.h"

#include <AzCore/Math/Uuid.h>
//#include <Integration/AnimationBus.h>
#include <Integration/AnimGraphComponentBus.h>

#include "Busses/GameManagerBus.h"
#include "Busses/NavigationManagerBus.h"

#include <ISystem.h>
#include <CryAction.h>

#include "Utils/NavigationUtils.h"
#include "Utils/Log.h"
#include "Utils/Filter.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>


using namespace spark;
//using namespace AzFramework;

class NavigationEntityNotificationBusHandler
	: public NavigationEntityNotificationBus::Handler
	, public AZ::BehaviorEBusHandler
{
public:
	AZ_EBUS_BEHAVIOR_BINDER(NavigationEntityNotificationBusHandler, "{4D16647C-8E41-4F46-B459-02050414105F}", AZ::SystemAllocator, OnPositionChangedFilter, OnPositionChanged, OnRotationZChangedFilter, OnRotationZChanged);

	void OnPositionChangedFilter(AZ::Vector3 current, AZ::Vector3 proposed) override
	{
		Call(FN_OnPositionChangedFilter, current,proposed);
	}

	void OnPositionChanged(AZ::Vector3 previous, AZ::Vector3 current) override
	{
		Call(FN_OnPositionChanged, previous, current);
	}

	void OnRotationZChangedFilter(float current, float proposed) override
	{
		Call(FN_OnRotationZChangedFilter, current, proposed);
	}

	void OnRotationZChanged(float previous, float current)  override
	{
		Call(FN_OnRotationZChanged, previous, current);
	}
};

class NavigationEntitiesNotificationBusHandler
	: public NavigationEntitiesNotificationBus::Handler
	, public AZ::BehaviorEBusHandler
{
public:
	AZ_EBUS_BEHAVIOR_BINDER(NavigationEntitiesNotificationBusHandler, "{5A1E7354-CBA4-4FCF-8747-D8F22EA6C918}", AZ::SystemAllocator, OnNavigationEntityPositionChangedFilter, OnNavigationEntityPositionChanged, OnNavigationEntityRotationZChangedFilter, OnNavigationEntityRotationZChanged);

	void OnNavigationEntityPositionChangedFilter(AZ::EntityId id, AZ::Vector3 current, AZ::Vector3 proposed) override
	{
		Call(FN_OnNavigationEntityPositionChangedFilter, id, current, proposed);
	}

	void OnNavigationEntityPositionChanged(AZ::EntityId id, AZ::Vector3 previous, AZ::Vector3 current) override
	{
		Call(FN_OnNavigationEntityPositionChanged, id, previous, current);
	}

	void OnNavigationEntityRotationZChangedFilter(AZ::EntityId id, float current, float proposed) override
	{
		Call(FN_OnNavigationEntityRotationZChangedFilter, id, current, proposed);
	}

	void OnNavigationEntityRotationZChanged(AZ::EntityId id, float previous, float current)  override
	{
		Call(FN_OnNavigationEntityRotationZChanged, id, previous, current);
	}
};


void UnitNavigationComponent::Reflect(AZ::ReflectContext* reflection)
{
	if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
	{
		serializationContext->Class<UnitNavigationComponent, AZ::Component>()
			->Version(1)
			->Field("blocking", &UnitNavigationComponent::m_blocking)
			->Field("Turn Rate", &UnitNavigationComponent::m_turnRate)
			->Field("Movement Speed", &UnitNavigationComponent::m_movementSpeed);

		if (auto editContext = serializationContext->GetEditContext())
		{
			editContext->Class<UnitNavigationComponent>("UnitNavigationComponent", "Navigation component for units")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
				->Attribute(AZ::Edit::Attributes::Category, "spark")
				->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"))
				->DataElement(nullptr, &UnitNavigationComponent::m_blocking, "Is blocking", "Can units traverse this object?")
				->DataElement(nullptr, &UnitNavigationComponent::m_turnRate, "Turn Rate", "How fast the unit can turn around")
				->DataElement(nullptr, &UnitNavigationComponent::m_movementSpeed, "Movement Speed", "Speed of movement?");
		}
	}

	if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
	{
		behaviorContext->EBus<UnitNavigationRequestBus>("UnitNavigationRequestBus")
			->Event("Follow", &UnitNavigationRequestBus::Events::Follow)
			->Event("MoveToPosition", &UnitNavigationRequestBus::Events::MoveToPosition);

		behaviorContext->EBus<NavigationEntityRequestBus>("NavigationEntityRequestBus")
			->Event("Stop", &NavigationEntityRequestBus::Events::Stop)
			->Event("GetPosition", &NavigationEntityRequestBus::Events::GetPosition)
			->Event("SetPosition", &NavigationEntityRequestBus::Events::SetPosition)
			->Event("SetRotationZ", &NavigationEntityRequestBus::Events::SetRotationZ)
			->Event("GetRotationZ", &NavigationEntityRequestBus::Events::GetRotationZ)
			->Event("GetRadius", &NavigationEntityRequestBus::Events::GetRadius)
			->Event("GetMovementSpeed", &NavigationEntityRequestBus::Events::GetMovementSpeed)
			->Event("GetTurnRate", &NavigationEntityRequestBus::Events::GetTurnRate)
			->Event("IsFacingDirection", &NavigationEntityRequestBus::Events::IsFacingDirection)
			->Event("IsFacingDirectionWithTolerance", &NavigationEntityRequestBus::Events::IsFacingDirectionWithTolerance)
			->Event("GetDirection", &NavigationEntityRequestBus::Events::GetDirection)
			->Event("SetDirection", &NavigationEntityRequestBus::Events::SetDirection)
			->Event("IsBlocking", &NavigationEntityRequestBus::Events::IsBlocking)
			->Event("SetBlocking", &NavigationEntityRequestBus::Events::SetBlocking)
			->Event("GetCurrentNavigationArea", &NavigationEntityRequestBus::Events::GetCurrentNavigationArea)
			->Event("AddToNavigationManager", &NavigationEntityRequestBus::Events::AddToNavigationManager)
			->Event("RemoveFromNavigationManager", &NavigationEntityRequestBus::Events::RemoveFromNavigationManager);
		
		behaviorContext->EBus<NavigationEntityNotificationBus>("NavigationEntityNotificationBus")
			->Handler<NavigationEntityNotificationBusHandler>();

		behaviorContext->EBus<NavigationEntitiesNotificationBus>("NavigationEntitiesNotificationBus")
			->Handler<NavigationEntitiesNotificationBusHandler>();

		behaviorContext->Method("Distance2D", &Distance2D);
	}
}

void UnitNavigationComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
{
    provided.push_back(AZ_CRC("NavigationEntityService"));
}

void UnitNavigationComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
{
    incompatible.push_back(AZ_CRC("NavigationEntityService"));
}

void UnitNavigationComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
{
    required.push_back(AZ_CRC("TransformService", 0x8ee22c50));
}

void UnitNavigationComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
{
	dependent.push_back(AZ_CRC("VariableHolderService"));
	dependent.push_back(AZ_CRC("ShapeService", 0xe86aa5fe));
}

void UnitNavigationComponent::Init()
{
	m_flag = m_blocking ? BlockingFlag::MOVEMENT_BLOCKING | BlockingFlag::VISION_BLOCKING | SPAWN_BLOCKING : 0;
}

void UnitNavigationComponent::Activate()
{
	//AZ_Printf(0, "[%s] UnitNavigationComponent::Activate()", GetEntityId().ToString().c_str());
	//AZ::TickBus::Handler::BusConnect();
	NavigationEntityRequestBus::Handler::BusConnect(GetEntityId());
	UnitNavigationRequestBus::Handler::BusConnect(GetEntityId());
	//LmbrCentral::NavigationComponentNotificationBus::Handler::BusConnect(GetEntityId());

	//AZ::TransformNotificationBus::Handler::BusConnect(GetEntityId());  //unfortunately it's not useful since the physic component modify the tranform each step

	EBUS_EVENT(VariableManagerRequestBus, RegisterVariableAndInizialize, VariableId(GetEntityId(), "movement_speed"), m_movementSpeed);
	EBUS_EVENT(VariableManagerRequestBus, RegisterVariableAndInizialize, VariableId(GetEntityId(), "turn_rate"), m_turnRate);

	AZ::Vector3 rotation;
	EBUS_EVENT_ID_RESULT(rotation, GetEntityId(), AZ::TransformBus, GetLocalRotation);
	m_currentRotation = (float)rotation.GetZ() + M_PI_2;

	if (m_addOnActivate) {
		EBUS_EVENT(NavigationManagerRequestBus, AddNavigationEntity, this);
	}
}

void UnitNavigationComponent::Deactivate()
{
	EBUS_EVENT(NavigationManagerRequestBus, RemoveNavigationEntity, this);

	// AZ_Printf(0, "[%s] UnitNavigationComponent::Deactivate()", GetEntityId().ToString().c_str());

	//AZ::TransformNotificationBus::Handler::BusDisconnect();
	//AZ::TickBus::Handler::BusDisconnect();

	UnitNavigationRequestBus::Handler::BusDisconnect();
	NavigationEntityRequestBus::Handler::BusDisconnect();
	//LmbrCentral::NavigationComponentNotificationBus::Handler::BusDisconnect();
}

void UnitNavigationComponent::UpdateTransform()
{
	EBUS_EVENT_ID(GetEntityId(), UnitNetSyncRequestBus, SetNamedParameterFloat, "MovementSpeed", (m_currentPath && (m_lastMovement.delta_rotation_z != 0.0f || m_lastMovement.delta_movement_forward != 0.0f)) ? GetMovementSpeed() * MOVEMENT_SCALE : 0.0f);

	AZ::Vector3 currentRotation;
	EBUS_EVENT_ID_RESULT(currentRotation, GetEntityId(), AZ::TransformBus, GetLocalRotation);
	currentRotation.SetZ(m_currentRotation - M_PI_2);
	EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus, SetLocalRotation, currentRotation);
}

//void UnitNavigationComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time)
//{
//	if (m_currentPath) {
//		auto m = CalculateMovement(deltaTime);
//		if (!CheckMovement(m, deltaTime))sLOG("test not passed");
//
//		ApplyMovement(m);
//	}
//
//	UpdateTransform();
//}

float degrees(float rad)
{
	return rad * 180 * M_1_PI;
}


float UnitNavigationComponent::GetMaxTurnMovement(float &deltaTime, float &currentAngle, float targetAngle)
{
	currentAngle = NormalizeAngle(currentAngle);
	targetAngle = NormalizeAngle(targetAngle);

	float distance = GetMinAngleDistance(currentAngle, targetAngle);
	bool clockwise = distance > 0;
	distance = fabsf(distance);

	//limit the movement by the turnRate
	float turnRate = GetTurnRate() * TURNRATE_SCALE;
	distance = min(distance, turnRate*deltaTime);

	//apply the movement
	float turn_movement = (clockwise ? 1.0f : -1.0f) * distance;;
	currentAngle += turn_movement;

	//re-normalize the angle
	currentAngle = NormalizeAngle(currentAngle);

	//return the time still available
	deltaTime -= (distance / turnRate);// max(0.0f, deltaTime - (distance / turnRate) - 0.001f); //we prefer 0 ( so the walk animation don't start ), over some infinitesimal value caused by float calculations
	return turn_movement;
}

float UnitNavigationComponent::GetMaxForwardMovement(float &deltaTime, AZ::Vector3 &currentPosition, AZ::Vector3 targetPosition)
{
	auto movement = targetPosition - currentPosition;
	auto distance = movement.GetLength();
	auto direction = movement.GetNormalized();
	auto movementSpeed = GetMovementSpeed() * MOVEMENT_SCALE;

	auto maxDistance = deltaTime * movementSpeed;
	if (maxDistance > distance)
	{
		deltaTime -= distance / movementSpeed;
	}
	else
	{
		deltaTime = 0;
		distance = maxDistance;
	}

	currentPosition = currentPosition + (direction * distance);

	return distance;
}





void UnitNavigationComponent::MoveToPosition(AZ::Vector3 position)
{
	spark::PathfindRequest request;
	request.SetDestinationLocation(position);

	SendPathfindRequest(request);
}

void UnitNavigationComponent::Follow(AZ::EntityId target, float distance)
{
	spark::PathfindRequest request;
	request = PathfindRequest();
	request.SetTargetEntityId(target);
	request.SetArrivalDistanceThreshold(distance);

	SendPathfindRequest(request);
}

void UnitNavigationComponent::SendPathfindRequest(spark::PathfindRequest request)
{
	if (request.HasTargetEntity() && request.GetTargetEntityId() == GetEntityId())return;


	NavigationPathTicket ticket=-1;
	EBUS_EVENT_RESULT(ticket, NavigationManagerRequestBus, CalculatePath, GetEntityId(), request);
	if (ticket == -1)
	{
		AZ_Printf(0, "UnitNavigationComponent failed to calculate the path");
		return;
	} 
	
	if (NavigationPathNotificationBus::Handler::BusIsConnected())
	{
		NavigationPathNotificationBus::Handler::BusDisconnect(); //disconnect from previous request, it is forbidden to connect to the same bus with multiple adresses(when not using MultiHandler)
	}

	NavigationPathNotificationBus::Handler::BusConnect(ticket);

	

	AZStd::string name;
	EBUS_EVENT_ID_RESULT(name, GetEntityId(), UnitRequestBus, GetName);
	AZ_Printf(0, "%s - %s] sent (move) navigation request : %d", GetEntityId().ToString().c_str(), name.c_str(), ticket);
}


float spark::UnitNavigationComponent::GetMovementSpeed()
{
	Value value = 100;
	EBUS_EVENT_ID_RESULT(value, GetEntityId(), VariableHolderRequestBus, GetValue, VariableId(GetEntityId(), "movement_speed"));
	return value;
}

float UnitNavigationComponent::GetTurnRate()
{
	Value value = 0.5;
	EBUS_EVENT_ID_RESULT(value, GetEntityId(), VariableHolderRequestBus, GetValue, VariableId(GetEntityId(), "turn_rate"));
	return value;
}

int UnitNavigationComponent::IsBlocking()
{
	return m_flag;
}

void UnitNavigationComponent::SetBlocking(int blocking)
{
	m_flag = blocking;
}

void UnitNavigationComponent::AddToNavigationManager()
{
	EBUS_EVENT(NavigationManagerRequestBus, AddNavigationEntity, this);
	SetPosition(GetPosition());
}

void UnitNavigationComponent::RemoveFromNavigationManager()
{
	EBUS_EVENT(NavigationManagerRequestBus, RemoveNavigationEntity, this);
}

void UnitNavigationComponent::SetCurrentNavigationArea(AZ::EntityId id)
{
	m_currentNavigationArea = id;
}

AZ::EntityId UnitNavigationComponent::GetCurrentNavigationArea()
{
	return m_currentNavigationArea;
}

bool UnitNavigationComponent::CollideWith(AZ::EntityId other)
{
	return false;
}


bool UnitNavigationComponent::IsFacingDirection(AZ::Vector3 dir) //calls IsFacingDirectionWithTolerance with tolerance = 0.1 radians(~5.7 degrees)
{
	return IsFacingDirectionWithTolerance(dir, 0.1);
}

bool UnitNavigationComponent::IsFacingDirectionWithTolerance(AZ::Vector3 direction, float tolerance)
{
	direction.SetZ(0);
	return  GetMinAngleDistance(direction, GetDirection()) < tolerance;
}

AZ::Vector3 UnitNavigationComponent::GetDirection()
{
	return GetDirectionFromAngle(m_currentRotation);
}

void UnitNavigationComponent::SetDirection(AZ::Vector3 direction)
{
	m_currentRotation = GetAngleFromDirection(direction);
}




//void spark::UnitNavigationComponent::OnTransformChanged(const AZ::Transform & local, const AZ::Transform & world)
//{
//	if (!m_ignoreTransformEvent)
//	{
//		sLOG("called OnTransformChanged");
//		AZ::Vector3 currentRotation;
//		EBUS_EVENT_ID_RESULT(currentRotation, GetEntityId(), AZ::TransformBus, GetLocalRotation);
//
//		m_currentRotation = (float)currentRotation.GetZ() + M_PI_2;
//	}
//}


// chris's favorite piece of code :D
bool UnitNavigationComponent::ShouldTurnRight(float a1, float a2) {
	bool reversed = false;

	if (a2 <= 0 && a1 >= 0) {
		reversed = a1 < M_PI + a2;
	}
	else if (a1 <= 0 && a2 >= 0) {
		reversed = a2 > M_PI + a1;
	}
	else {
		reversed = (a1 - a2) > 0;
	}

	return !reversed;
}


void UnitNavigationComponent::SetPath(const SparkNavPath& path) {
	//AZ_Printf(0, "UnitNavigationComponent::SetPath called");

	m_pathMutex.lock();
	m_currentPath = path;
	m_pathMutex.unlock();

	EBUS_EVENT(NavigationManagerRequestBus, SetEntityPath, GetEntityId(), path);
}

bool UnitNavigationComponent::IsMoving()
{
	return m_currentPath.m_valid;
}

void UnitNavigationComponent::Stop()
{
	sLOG("UnitNavigationComponent::Stop()");

	m_pathMutex.lock();
	if (NavigationPathNotificationBus::Handler::BusIsConnected())NavigationPathNotificationBus::Handler::BusDisconnect();
	m_currentPath = SparkNavPath();
	m_pathMutex.unlock();
}

bool UnitNavigationComponent::HasArrivedToDestination()
{
	if (!m_currentPath)
	{
		return true;
	}

	AZ::Vector3 currentPosition = GetPosition();
	float currentRotation = m_currentRotation;

	//check arrival requirements
	auto &request = m_currentPath.m_request;
	if (request.HasOverrideArrivalDistance() || request.HasOverrideDestinationDirection())
	{
		bool done = true;
		AZ::Vector3 a, b = request.GetDestinationLocation();
		EBUS_EVENT_ID_RESULT(a, GetEntityId(), AZ::TransformBus, GetWorldTranslation);
		if (request.HasTargetEntity())
		{
			EBUS_EVENT_ID_RESULT(b, request.GetTargetEntityId(), AZ::TransformBus, GetWorldTranslation);
		}
		a.SetZ(0);
		b.SetZ(0);

		if (request.HasOverrideArrivalDistance()) {
			//Check if inside the arrival threshold
			if (a.GetDistance(b) > request.GetArrivalDistanceThreshold()) {
				return false;
			}
		}

		if (done && (request.HasOverrideDestinationDirection() || request.HasTargetEntity() || request.HasOverrideArrivalDistance()))
		{
			//we are in range,
			//check if the unit is facing the correct direction
			AZ::Vector3 target_direction = b - a;

			if (request.HasOverrideDestinationDirection())
			{
				target_direction = request.GetDestinationDirection();
			}

			target_direction.SetZ(0);
			target_direction.Normalize();

			if (!IsFacingDirection(target_direction)) //get difference in angle of the two vectors
			{
				return false;
			}
		}
		return true;
	}
	else
	{
		auto nextPathPoint = m_currentPath.GetNextPathPoint();
		while (nextPathPoint && Distance2D(currentPosition,*nextPathPoint) < MOVEMENT_DISTANCE_THRESHOLD)
		{
			if (!m_currentPath.Advance())return true;
			nextPathPoint = m_currentPath.GetNextPathPoint();
		}
	}
	
	return false;
}

MovementInfo UnitNavigationComponent::CalculateMovement(float deltaTime)
{
	MovementInfo movement;

	if (!m_currentPath)
	{
		return movement;
	}

	//draw
	auto *pPersistentDebug = gEnv->pGame->GetIGameFramework()->GetIPersistentDebug();
	if (pPersistentDebug)
	{
		pPersistentDebug->Begin("unitNav", true);

		m_pathMutex.lock();
		for (int i = 1; i < m_currentPath.m_pathPoints.size(); ++i)
		{
			pPersistentDebug->AddLine(AZVec3ToLYVec3(m_currentPath.m_pathPoints[i - 1]), AZVec3ToLYVec3(m_currentPath.m_pathPoints[i]), Col_Green, 500);
		}
		m_pathMutex.unlock();
	}


	Status status;
	EBUS_EVENT_ID_RESULT(status, GetEntityId(), UnitRequestBus, GetStatus);

	// movement
	AZ::Vector3 currentPosition;
	float currentRotation = m_currentRotation;
	EBUS_EVENT_ID_RESULT(currentPosition, GetEntityId(), AZ::TransformBus, GetLocalTranslation);
	currentPosition.SetZ(0);

	auto originalDeltaTime = deltaTime;

	//check arrival requirements
	auto &request = m_currentPath.m_request;
	if (request.HasOverrideArrivalDistance() || request.HasOverrideDestinationDirection())
	{
		bool done = true;
		AZ::Vector3 a, b = request.GetDestinationLocation();
		EBUS_EVENT_ID_RESULT(a, GetEntityId(), AZ::TransformBus, GetWorldTranslation);
		if (request.HasTargetEntity())
		{
			EBUS_EVENT_ID_RESULT(b, request.GetTargetEntityId(), AZ::TransformBus, GetWorldTranslation);
		}
		a.SetZ(0);
		b.SetZ(0);

		if (request.HasOverrideArrivalDistance()) {
			//Check if inside the arrival threshold
			if (a.GetDistance(b) > request.GetArrivalDistanceThreshold()) {
				done = false;
			}
		}

		if (done && (request.HasOverrideDestinationDirection() || request.HasTargetEntity() || request.HasOverrideArrivalDistance()))
		{
			//we are in range,
			//check if the unit is facing the correct direction
			AZ::Vector3 target_direction = b - a;

			if (request.HasOverrideDestinationDirection())
			{
				target_direction = request.GetDestinationDirection();
			}

			target_direction.SetZ(0);
			target_direction.Normalize();

			if (!IsFacingDirection(target_direction)) //get difference in angle of the two vectors
			{
				done = false;
				//not facing the correct direction -> turn

				//check if the unit is unable to turn
				if (status & CANT_ROTATE) {
					return movement; //we can return since there are no cases where you can move not being able to rotate (they are/can be handled by forced movemement)
				}

				movement.delta_rotation_z = GetMaxTurnMovement(deltaTime, currentRotation, GetAngleFromDirection(target_direction));

				if (deltaTime <= 0.f)
				{
					done = true;
				}
				else
				{
					deltaTime = 0.0f;
				}
			}
		}

		if (done) {
			return movement;
		}
	}

	//check if the unit is unable to rotate
	if (status & CANT_ROTATE) {
		return movement; //we can return since there are no cases where you can move not being able to rotate (they are/can be handled by forced movemement)
	}

	m_pathMutex.lock();

	auto nextPathPoint = m_currentPath.GetNextPathPoint();

	while(nextPathPoint && (Distance2D(currentPosition,*nextPathPoint) < MOVEMENT_DISTANCE_THRESHOLD))
	{
		m_currentPath.Advance();
		nextPathPoint = m_currentPath.GetNextPathPoint();
	}

	if(deltaTime > 0 && nextPathPoint)
	{
		auto nextPosition = *nextPathPoint;
		nextPosition.SetZ(0);

		auto direction = nextPosition - currentPosition;

		movement.delta_rotation_z = GetMaxTurnMovement(deltaTime, currentRotation, GetAngleFromDirection(direction));

		//check if the unit can move and if there is time to do so
		if (deltaTime > 0.f  && !(status & CANT_MOVE) ) {
			movement.delta_movement_forward = GetMaxForwardMovement(deltaTime, currentPosition, nextPosition);
		}
	}
	m_pathMutex.unlock();

	return movement;
}

void UnitNavigationComponent::ApplyMovement(MovementInfo movement)
{
	m_lastMovement = movement;

	if (movement.delta_rotation_z != 0.0f)
	{
		SetRotationZ(GetRotationZ() +  movement.delta_rotation_z);
	}

	if (movement.delta_movement_forward != 0.0f)
	{
		auto new_pos = GetPosition() + GetDirection() * movement.delta_movement_forward;
		float elevation;
		EBUS_EVENT_RESULT(elevation, MapRequestBus, GetTerrainHeightAt, new_pos);
		new_pos.SetZ(elevation);

		SetPosition(new_pos);
	}

	if (HasArrivedToDestination())
	{
		Done();
	}
}

bool UnitNavigationComponent::CheckMovement(MovementInfo movement, float deltaTime)
{
	return (movement.delta_rotation_z/(GetTurnRate()*TURNRATE_SCALE) + movement.delta_movement_forward/(GetMovementSpeed()*MOVEMENT_SCALE)) <= deltaTime + std::numeric_limits<float>::epsilon();
}

AZ::Vector3 UnitNavigationComponent::ConvertToVelocity(MovementInfo movement)
{
	float rotation = m_currentRotation + movement.delta_rotation_z;

	return GetDirectionFromAngle(rotation) * movement.delta_movement_forward;
}

void UnitNavigationComponent::Done()
{
	Stop();
	EBUS_EVENT_ID(GetEntityId(), UnitNavigationNotificationBus, OnMovingDone);
}

AZ::Vector3 UnitNavigationComponent::GetPosition()
{
	AZ::Vector3 pos;
	EBUS_EVENT_ID_RESULT(pos, GetEntityId(), AZ::TransformBus, GetWorldTranslation);
	return pos;
}
void UnitNavigationComponent::SetPosition(AZ::Vector3 new_pos)
{
	AZ::Vector3 pos;
	EBUS_EVENT_ID_RESULT(pos, GetEntityId(), AZ::TransformBus, GetWorldTranslation);
	
	{
		FilterResultTuple<AZ::Vector3> result;
		result.Push();
		EBUS_EVENT_ID(GetEntityId(), NavigationEntityNotificationBus, OnPositionChangedFilter, pos, new_pos);
		result.Pop();

		switch (result.action)
		{
		case FilterResult::FILTER_PREVENT:	return;
		case FilterResult::FILTER_MODIFY:	result.CopyTo(new_pos);
		}
	}
	{
		FilterResultTuple<AZ::Vector3> result;
		result.Push();
		EBUS_EVENT(NavigationEntitiesNotificationBus, OnNavigationEntityPositionChangedFilter, GetEntityId(), pos, new_pos);
		result.Pop();

		switch (result.action)
		{
		case FilterResult::FILTER_PREVENT:	return;
		case FilterResult::FILTER_MODIFY:	result.CopyTo(new_pos);
		}
	}
	
	EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus, SetWorldTranslation,new_pos);

	AZStd::vector<AZ::EntityId> entities;
	EBUS_EVENT_ID_RESULT(entities, GetEntityId(), AZ::TransformBus, GetEntityAndAllDescendants);

	for (auto e : entities)
	{
		EBUS_EVENT_ID(e, NavigationEntityNotificationBus, OnPositionChanged, pos, new_pos);
		EBUS_EVENT(NavigationEntitiesNotificationBus, OnNavigationEntityPositionChanged, e, pos, new_pos);
	}

}

void UnitNavigationComponent::SetRotationZ(float rotation)
{
	{
		FilterResultTuple<float> result;
		result.Push();
		EBUS_EVENT_ID(GetEntityId(), NavigationEntityNotificationBus, OnRotationZChangedFilter, m_currentRotation, rotation);
		result.Pop();

		switch (result.action)
		{
		case FilterResult::FILTER_PREVENT:	return;
		case FilterResult::FILTER_MODIFY:	result.CopyTo(rotation);
		}
	}
	{
		FilterResultTuple<float> result;
		result.Push();
		EBUS_EVENT(NavigationEntitiesNotificationBus, OnNavigationEntityRotationZChangedFilter, GetEntityId(), m_currentRotation, rotation);
		result.Pop();

		switch (result.action)
		{
		case FilterResult::FILTER_PREVENT:	return;
		case FilterResult::FILTER_MODIFY:	result.CopyTo(rotation);
		}
	}

	AZStd::swap(m_currentRotation, rotation);

	EBUS_EVENT_ID(GetEntityId(), NavigationEntityNotificationBus, OnRotationZChanged, m_currentRotation, rotation);
	EBUS_EVENT(NavigationEntitiesNotificationBus, OnNavigationEntityRotationZChanged, GetEntityId(), m_currentRotation, rotation);
	
}
float UnitNavigationComponent::GetRotationZ()
{
	return m_currentRotation;
}

float UnitNavigationComponent::GetRadius()
{
	Value value = 1;
	EBUS_EVENT_ID_RESULT(value, GetEntityId(), VariableHolderRequestBus, GetValue, VariableId(GetEntityId(), "radius"));
	return value;
}


void UnitNavigationComponent::OnPathFound(const spark::PathfindRequest &request, const SparkNavPath &path)
{
	//NavigationPathNotificationBus::Handler::BusDisconnect();

	//AZ_Printf(0, "UnitNavigationComponent::OnPathFound called with %d points", path.m_pathPoints.size());

	SetPath(path);
}

void UnitNavigationComponent::OnPathNotFound(const spark::PathfindRequest &request)
{
	NavigationPathNotificationBus::Handler::BusDisconnect();
	AZ_Printf(0, "UnitNavigationComponent::OnPathNotFound");
	

	AZ_Printf(0, "UnitNavigationComponent destination is unreachable");
	EBUS_EVENT_ID(GetEntityId(), UnitNavigationNotificationBus, OnDestinationUnreachable);
}

