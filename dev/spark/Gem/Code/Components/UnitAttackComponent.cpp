
#include "spark_precompiled.h"

#include "UnitAttackComponent.h"

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Component/TransformBus.h>
#include <Integration/AnimGraphComponentBus.h>

#include "Busses/GameManagerBus.h"
#include "Busses/ProjectileManagerBus.h"
#include "Busses/NetSyncBusses.h"
#include "Busses/UnitBus.h"
#include "Utils/NavigationUtils.h"

using namespace spark;


void UnitAttackComponent::Reflect(AZ::ReflectContext* reflection)
{
	if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
	{
		serializationContext->Class<UnitAttackComponent, AZ::Component>()
			->Version(1)
			->Field("IsMelee", &UnitAttackComponent::m_isMelee)
			->Field("Max Range", &UnitAttackComponent::m_maxRange)
			->Field("Base attack point", &UnitAttackComponent::m_baseAttackPoint)
			->Field("Base attack backswing", &UnitAttackComponent::m_baseBackswingPoint)
			->Field("Increased Attack Speed", &UnitAttackComponent::m_IAS)
			->Field("Missile speed", &UnitAttackComponent::m_projectileSpeed)
			->Field("Projectile Slice",&UnitAttackComponent::m_projectileAsset);

		if (auto editContext = serializationContext->GetEditContext())
		{
			editContext->Class<UnitAttackComponent>("UnitAttackComponent", "Needed if the unit can attack")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
				->Attribute(AZ::Edit::Attributes::Category, "spark")
				->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"))
				->DataElement(AZ::Edit::UIHandlers::Default, &UnitAttackComponent::m_isMelee, "IsMelee","")
				->DataElement(AZ::Edit::UIHandlers::Default, &UnitAttackComponent::m_maxRange, "Max Range", "The maximum range to which the unit can attack")
				->DataElement(AZ::Edit::UIHandlers::Default, &UnitAttackComponent::m_baseAttackPoint, "Base attack point","")
				->DataElement(AZ::Edit::UIHandlers::Default, &UnitAttackComponent::m_baseBackswingPoint, "Base attack backswing", "")
				->DataElement(AZ::Edit::UIHandlers::Default, &UnitAttackComponent::m_IAS, "Increased Attack Speed", "")
				->DataElement(AZ::Edit::UIHandlers::Default, &UnitAttackComponent::m_projectileSpeed, "Missile speed", "")
				->DataElement(AZ::Edit::UIHandlers::Default, &UnitAttackComponent::m_projectileAsset,"Projectile Slice","");
		}
	}

	if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
	{
		behaviorContext->EBus<UnitAttackRequestBus>("UnitAttackRequestBus")
			->Event("IsMelee", &UnitAttackRequestBus::Events::IsMelee)
			->Event("SetMelee", &UnitAttackRequestBus::Events::SetMelee)
			->Event("SetProjectileAsset", &UnitAttackRequestBus::Events::SetProjectileAsset)
			->Event("GetProjectileAsset", &UnitAttackRequestBus::Events::GetProjectileAsset);
	}
}

void UnitAttackComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
{
	provided.push_back(AZ_CRC("UnitAttackComponentService"));
}
void UnitAttackComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
{
	incompatible.push_back(AZ_CRC("UnitAttackComponentService"));
}
void UnitAttackComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
{
    required.push_back(AZ_CRC("VariableHolderService"));
}
void UnitAttackComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
{
}

void UnitAttackComponent::Init()
{
	m_stateMachine.SetStateHandler(SSM_STATE_NAME(S_IDLE),      SSM::StateHandler(this, &UnitAttackComponent::OnStateIdle));
	m_stateMachine.SetStateHandler(SSM_STATE_NAME(S_FORESWING), SSM::StateHandler(this, &UnitAttackComponent::OnStateForeswing));
	m_stateMachine.SetStateHandler(SSM_STATE_NAME(S_BACKSWING), SSM::StateHandler(this, &UnitAttackComponent::OnStateBackswing));
	m_stateMachine.SetStateHandler(SSM_STATE_NAME(S_MOVING),    SSM::StateHandler(this, &UnitAttackComponent::OnStateMoving));

	m_stateMachine.SetOnEmptyStackHandler(SSM::EmptyStackHandler(this, &UnitAttackComponent::OnAttackingInterrupted));

	m_stateMachine.Start(S_IDLE);

	

	m_timeSinceLastAttack = 1000.0f; // to not limit the first attack

	AZ_Printf(0,"UnitAttackComponent::Init()");
}

void UnitAttackComponent::Activate()
{
	UnitAttackRequestBus::Handler::BusConnect(GetEntityId());
	AZ::TickBus::Handler::BusConnect();

	
	EBUS_EVENT(VariableManagerRequestBus, RegisterVariableAndInizialize, VariableId(GetEntityId(), "IAS"),m_IAS);
	EBUS_EVENT(VariableManagerRequestBus, RegisterVariableAndInizialize, VariableId(GetEntityId(), "attack_point"),m_baseAttackPoint);
	EBUS_EVENT(VariableManagerRequestBus, RegisterVariableAndInizialize, VariableId(GetEntityId(), "backswing_point"),m_baseBackswingPoint);
	
	EBUS_EVENT(VariableManagerRequestBus, RegisterVariableAndInizialize, VariableId(GetEntityId(), "damage"),1.0f);
	EBUS_EVENT(VariableManagerRequestBus, RegisterVariableAndInizialize, VariableId(GetEntityId(), "attack_range"),m_maxRange);
	EBUS_EVENT(VariableManagerRequestBus, RegisterVariableAndInizialize, VariableId(GetEntityId(), "projectile_speed"),m_projectileSpeed);
}

void UnitAttackComponent::Deactivate()
{
	AZ::TickBus::Handler::BusDisconnect();
	UnitNavigationNotificationBus::Handler::BusDisconnect();
	UnitAttackRequestBus::Handler::BusDisconnect();
}


void UnitAttackComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time)
{
	m_elapsedTime += deltaTime;
	m_timeSinceLastAttack += deltaTime;

	m_stateMachine.Dispatch(SSM::Event(E_UPDATE));
}

void UnitAttackComponent::OnMovingDone()
{
	AZ_Printf(0, "UnitAttackComponent::OnMovingDone");
	m_stateMachine.Dispatch(SSM::Event(E_TRAVERSAL_COMPLETE));
}

void UnitAttackComponent::OnDestinationUnreachable()
{
	AZ_Printf(0, "UnitAttackComponent::OnDestinationUnreachable state: %d",m_stateMachine.GetCurrentState());
	m_stateMachine.Dispatch(SSM::Event(E_DESTINATION_UNREACHABLE));
}

Value UnitAttackComponent::GetDamage()
{
	Value damage=1;
	EBUS_EVENT_RESULT(damage, VariableManagerRequestBus, GetValue, VariableId(GetEntityId(), "damage"));
	return damage;
}

Value UnitAttackComponent::GetProjectileSpeed()
{
	Value value=10;
	EBUS_EVENT_RESULT(value, VariableManagerRequestBus, GetValue, VariableId(GetEntityId(), "projectile_speed"));
	if (value <= 0)value = 10;
	value *= MOVEMENT_SCALE;
	return value;
}

void UnitAttackComponent::SendAttackEvents(AttackInfo info)
{
	//todo check miss/evasion
	if (!SparkAttackFilterEvent("OnAttackLanded", info))
	{
		//SPARK_ATTACK_EVENT(info.attacker, "OnAttackFailed", info);
		return;
	}

	Value hp_damage;
	Damage damage;
	damage.damage = info.damage;
	damage.source = info.attacker;
	damage.target = info.attacked;
	EBUS_EVENT_ID_RESULT(hp_damage,info.attacked, OnDamageTakenRequestBus, ApplyDamage, damage);
	info.damage = hp_damage;

	SparkAttackEvent(info.attacker, "OnAttackSuccessful", info);
	SparkAttackEvent(info.attacked, "OnAttackSuccessful", info);
}

void UnitAttackComponent::Attack()
{
	AZ_Printf(0, "Attack!");
	if (m_isMelee) {
		//damage directly
		m_attackInfo.damage = GetDamage();

		SendAttackEvents(m_attackInfo);
	}
	else 
	{
		//fire projectile
		AZ_Printf(0,"projectile fired!");

		ProjectileInfo info;
		info.attacker = GetEntityId();
		info.targetEntity = m_target;
		info.targetType = ProjectileInfo::TARGET_ENTITY;
		info.type = ProjectileInfo::NORMAL_ATTACK;
		info.asset = "projectile_001";// m_projectileAsset.GetId();

		info.speed = GetProjectileSpeed();
		info.damage.damage = GetDamage();
		info.autoRelease = true;

		EBUS_EVENT_ID_RESULT(info.startingPosition, GetEntityId(), AZ::TransformBus, GetLocalTranslation);

		EBUS_EVENT(ProjectileManagerRequestBus, FireProjectile, info);
	}
}


void UnitAttackComponent::OnAttackingInterrupted()
{
	sLOG("UnitAttackComponent::OnAttackingInterrupted()");
	EBUS_EVENT_ID(GetEntityId(), UnitAttackNotificationBus, OnAttackingInterrupted);
}

bool UnitAttackComponent::OnStateIdle(SSM& m, const SSM::Event& e)
{
	switch (e.id) {
	case SSM::EnterEventId:
	{
		//start idle animation
		EBUS_EVENT_ID(GetEntityId(), UnitNetSyncRequestBus, SetNamedParameterFloat,"AttackSpeed",0.0f);
		m_elapsedTime = 0.0f;
		return true; 
	}
	case E_UPDATE:
	{
		if (m_attacking && m_timeSinceLastAttack >= GetBackswingPoint() + GetIdlePoint()) {
			
			m.ChangeState(S_FORESWING);
		}
		return true;
	}
	}
	return false;
}

bool UnitAttackComponent::OnStateForeswing(SSM& m, const SSM::Event& e)
{
	switch (e.id) {
	case SSM::OnResume:
	case SSM::EnterEventId:
	{
		Status status=DEAD;
		EBUS_EVENT_ID_RESULT(status, m_target, UnitRequestBus, GetStatus);

		if (status & CANT_BE_ATTACKED) {
			m.PopState();
			return true;
		}

		AZ_Printf(0, "EnterEventId in OnStateForeswing");
		//check if the unit can attack
		if (!m_attacking && m_timeSinceLastAttack >= GetBackswingPoint() + GetIdlePoint()) 
		{
			m.ChangeState(S_IDLE);
			return true;
		}

		//check if the target unit is in range
		AZ::Vector3 a,b;
		EBUS_EVENT_ID_RESULT(a, GetEntityId(), AZ::TransformBus, GetWorldTranslation);
		EBUS_EVENT_ID_RESULT(b, m_target, AZ::TransformBus, GetWorldTranslation);
		a.SetZ(0); b.SetZ(0);

		float distance = a.GetDistance(b);
		//EBUS_EVENT_RESULT(distance, GameManagerRequestBus, GetDistance, GetEntityId(), m_target);

		if (distance > GetMaxRange()) {
			sLOG("UnitAttackComponent not in range");
			m.PushState(S_MOVING);
			return true;
		}

		//check if the unit is facing the correct direction
		AZ::Vector3 target_direction=b-a;
		target_direction.Normalize();
		bool ok = true;
		EBUS_EVENT_ID_RESULT(ok, GetEntityId(), UnitNavigationRequestBus, IsFacingDirection, target_direction);
		if ( !ok )
		{
			sLOG("UnitAttackComponent not facing the correct direction");
			m.PushState(S_MOVING);
			return true;
		}

		EBUS_EVENT_ID_RESULT(status, GetEntityId(), UnitRequestBus, GetStatus);
		if (status & CANT_ATTACK) {
			m.ChangeState(S_IDLE);
			return true;
		}

		if (!SparkAttackFilterEvent("OnAttackStart", m_attackInfo))
		{
			m.PopState();
			return true;
		}

		AZ_Printf(0, "start attack animation");
		//start attack animation
		EBUS_EVENT_ID(GetEntityId(), UnitNetSyncRequestBus, SetNamedParameterFloat, "AttackSpeed", 1.0f + GetIncreasedAttackSpeed());

		m_elapsedTime = 0.0f;
		//SparkAttackEvent(GetEntityId(), "OnAttackStart", m_attackInfo);
		return true;
	}
	case E_UPDATE:
	{
		if (m_elapsedTime >= GetAttackPoint()) {
			Attack();
			m_timeSinceLastAttack = 0.0f;
			
			AZ_Printf(0, "Attack point");

			m.ChangeState(S_BACKSWING);
		}
		return true;
	}
	}
	return false;
}

bool UnitAttackComponent::OnStateBackswing(SSM& m, const SSM::Event& e)
{
	switch (e.id) {
	case SSM::EnterEventId:
	{
		//start backswing animation
		m_elapsedTime = 0.0f;
		return true; 
	}
	case E_UPDATE:
	{
		if (m_elapsedTime >= GetBackswingPoint()) {
			
			m.ChangeState(S_IDLE);
		}
		return true;
	}
	}
	return false;
}

bool UnitAttackComponent::OnStateMoving(SSM& m, const SSM::Event& e)
{
	switch (e.id) {
	case SSM::EnterEventId:
	{

		UnitNavigationNotificationBus::Handler::BusConnect(GetEntityId());
		AZ_Printf(0, "following started");
		//request moving
		EBUS_EVENT_ID(GetEntityId(),UnitNavigationRequestBus, Follow, m_target, GetMaxRange());
		return true; 
	}
	//case E_UPDATE: //necessary beacause follow doesn't work as intended yet
	//{
	//	float distance=0.0f;
	//	EBUS_EVENT_RESULT(distance, GameManagerRequestBus, GetDistance, GetEntityId(), m_target);

	//	if (distance > GetMaxRange())break;//go in E_TRAVERSAL_COMPLETE case if in range

	//	EBUS_EVENT_ID(GetEntityId(),UnitNavigationRequestBus, Stop);
	//}
	case E_DESTINATION_UNREACHABLE:
	{
		AZ_Printf(0, "impossible to reach the target");
		m.PopStates(); //terminate the attack order
		return true;
	}
	case E_TRAVERSAL_COMPLETE:
	{
		AZ_Printf(0, "following finished");
		m.PopState();
		return true;
	}
	case E_STOP:
	{
		EBUS_EVENT_ID(GetEntityId(), UnitNavigationRequestBus, Stop);
		m.PopStates();
		return true;
	}
	case SSM::ExitEventId:
	{
		UnitNavigationNotificationBus::Handler::BusDisconnect();
		return true;
	}
	}
	return false;
}



void UnitAttackComponent::Attack(AZ::EntityId target)
{
	if (!target.IsValid())
	{
		sWARNING("sent an attack order with an invalid target!");
		OnAttackingInterrupted();
		return;
	}

	m_attackInfo.attacked = target;
	m_attackInfo.attacker = GetEntityId();
	m_attackInfo.damage = 0;

	m_target = target;
	m_attacking = true;
	m_stateMachine.ResizeStack(0);
	m_stateMachine.PushState(S_FORESWING);	
}

void UnitAttackComponent::Stop()
{
	EBUS_EVENT_ID(GetEntityId(), UnitNetSyncRequestBus, SetNamedParameterFloat,"AttackSpeed",0.0f);
	m_attacking = false;
	m_stateMachine.Dispatch(SSM::Event(E_STOP));
	m_stateMachine.ResizeStack(0);
}

float UnitAttackComponent::GetMaxRange()
{
	Value value=1;
	EBUS_EVENT_RESULT(value, VariableManagerRequestBus, GetValue, VariableId(GetEntityId(), "attack_range"));
	return value * MOVEMENT_SCALE;
	//return m_maxRange;
}

float UnitAttackComponent::GetAttackPoint()
{
	return GetBaseAttackPoint()/(1.0f+GetIncreasedAttackSpeed());
}

float UnitAttackComponent::GetBackswingPoint()
{
	return GetBaseBackswingPoint()/(1.0f+GetIncreasedAttackSpeed());
}

float UnitAttackComponent::GetIdlePoint()
{
	return GetBaseIdlePoint()/(1.0f+GetIncreasedAttackSpeed());
}

float UnitAttackComponent::GetBaseAttackPoint()
{
	Value value=0.6;
	EBUS_EVENT_RESULT(value, VariableManagerRequestBus, GetValue, VariableId(GetEntityId(), "attack_point"));
	return value;
	//return m_baseAttackPoint;
}
float UnitAttackComponent::GetBaseBackswingPoint()
{
	Value value=0.4;
	EBUS_EVENT_RESULT(value, VariableManagerRequestBus, GetValue, VariableId(GetEntityId(), "backswing_point"));
	return value;
	//return m_baseBackswingPoint;
}
float UnitAttackComponent::GetBaseIdlePoint()
{
	return 1 - GetBaseAttackPoint() - GetBaseBackswingPoint();
}

float UnitAttackComponent::GetIncreasedAttackSpeed()
{
	Value value=0;
	EBUS_EVENT_RESULT(value, VariableManagerRequestBus, GetValue, VariableId(GetEntityId(), "IAS"));
	return value;
	//return m_IAS;
}
void  UnitAttackComponent::SetIncreasedAttackSpeed(float value)
{
	m_IAS = value;
}

bool UnitAttackComponent::IsMelee()
{
	return m_isMelee;
}

void UnitAttackComponent::SetMelee(bool melee)
{
	m_isMelee = melee;
}

void UnitAttackComponent::SetProjectileAsset(ProjectileAsset asset)
{
	m_projectileAsset = asset;
}

ProjectileAsset UnitAttackComponent::GetProjectileAsset()
{
	return m_projectileAsset;
}
