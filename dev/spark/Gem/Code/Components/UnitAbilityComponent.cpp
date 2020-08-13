
#include "spark_precompiled.h"

#include "UnitAbilityComponent.h"
#include "Busses/GameManagerBus.h"

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Component/TransformBus.h>
#include <AzFramework/Network/NetBindingHandlerBus.h>
#include <AzFramework/Network/NetworkContext.h>
#include <AzFramework/Network/NetBindingHandlerBus.h>
#include <AzFramework/Network/NetBindingComponent.h>
#include <GridMate/Replica/ReplicaFunctions.h>
#include <GridMate/Replica/ReplicaChunk.h>
#include <GridMate/Replica/DataSet.h>
#include <GridMate/Serialize/ContainerMarshal.h>
 
#include "Utils/Marshaler.h"

#include "Busses/VariableBus.h"
#include "Utils/Log.h"
#include "Busses/CooldownBus.h"
#include "Busses/UnitBus.h"
#include "Utils/NavigationUtils.h"

using namespace spark;

class UnitAbilityReplicaChunk : public GridMate::ReplicaChunkBase
{
public:
    AZ_CLASS_ALLOCATOR(UnitAbilityReplicaChunk, AZ::SystemAllocator, 0);

    static const char* GetChunkName() { return "UnitAbilityReplicaChunk"; }

       UnitAbilityReplicaChunk()
        : m_abilities("AbilityList")
    {
    }

    bool IsReplicaMigratable()
    {
        return true;
    }

    GridMate::DataSet<UnityAbilityMap, GridMate::MapContainerMarshaler<UnityAbilityMap>>::BindInterface<UnitAbilityComponent, &UnitAbilityComponent::OnNewAbilityList> m_abilities;
};


class UnitAbilityNotificationBusHandler
	: public UnitAbilityNotificationBus::Handler
	, public AZ::BehaviorEBusHandler
{
public:
	AZ_EBUS_BEHAVIOR_BINDER(UnitAbilityNotificationBusHandler, "{D0C1AC3F-7E32-4778-9371-392E292D6596}", AZ::SystemAllocator, OnAbilityAttached, OnAbilityDetached);

	void OnAbilityAttached(AbilityId id) override
	{
		Call(FN_OnAbilityAttached, id);
	}
	void OnAbilityDetached(AbilityId id) override
	{
		Call(FN_OnAbilityDetached, id);
	}
};


void UnitAbilityComponent::Reflect(AZ::ReflectContext* reflection)
{
	if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
	{
		serializationContext->Class<UnitAbilityComponent, AzFramework::NetBindable, AZ::Component>()
			->Version(2)
			->Field("Abilities", &UnitAbilityComponent::m_abilities);

		if (auto editContext = serializationContext->GetEditContext())
		{
			editContext->Class<UnitAbilityComponent>("UnitAbilityComponent", "Needed if the unit has abilities")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
				->Attribute(AZ::Edit::Attributes::Category, "spark")
				->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"))
				->DataElement(AZ::Edit::UIHandlers::Default, &UnitAbilityComponent::m_abilities, "Abilities", "todo description");//todo show in the editor the ability type/slot
		}
	}

	if (auto netContext = azrtti_cast<AzFramework::NetworkContext*>(reflection))
	{
	    netContext->Class<UnitAbilityComponent>()
			->Chunk<UnitAbilityReplicaChunk>()
	        ->Field("AbilityList", &UnitAbilityReplicaChunk::m_abilities)
	        ;
	}

	if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
	{
		behaviorContext->EBus<UnitAbilityRequestBus>("UnitAbilityRequestBus")
			->Event("HasAbilityInSlot", &UnitAbilityRequestBus::Events::HasAbilityInSlot)
			->Event("GetAbilityInSlot", &UnitAbilityRequestBus::Events::GetAbilityInSlot)
			->Event("SetAbilityInSlot", &UnitAbilityRequestBus::Events::SetAbilityInSlot)
			->Event("ClearAbilitySlot", &UnitAbilityRequestBus::Events::ClearAbilitySlot)
			->Event("ClearAllAbilities", &UnitAbilityRequestBus::Events::ClearAllAbilities)
			->Event("HasAbility", &UnitAbilityRequestBus::Events::HasAbility)
			->Event("GetAbilitySlot", &UnitAbilityRequestBus::Events::GetAbilitySlot)
			->Event("DetachAbility", &UnitAbilityRequestBus::Events::DetachAbility)
			->Event("GetAbilities",  &UnitAbilityRequestBus::Events::GetAbilities)
			->Event("ToString", &UnitAbilityRequestBus::Events::ToString)
			->Event("HasAbilityType", &UnitAbilityRequestBus::Events::HasAbilityType)
			; 
		
		behaviorContext->EBus<CastContextRequestBus>("CastContextRequestBus")
			->Event("IsItem", &CastContextRequestBus::Events::IsItem)
			->Event("GetCaster", &CastContextRequestBus::Events::GetCaster)
			->Event("GetCursorTarget", &CastContextRequestBus::Events::GetCursorTarget)
			->Event("GetCursorPosition", &CastContextRequestBus::Events::GetCursorPosition)
			->Event("GetBehaviorUsed", &CastContextRequestBus::Events::GetBehaviorUsed)
			;

		behaviorContext->EBus<UnitAbilityNotificationBus>("UnitAbilityNotificationBus")
			->Handler<UnitAbilityNotificationBusHandler>();
	}
}

void UnitAbilityComponent::OnNewAbilityList (const UnityAbilityMap& abilities, const GridMate::TimeContext& tc)
{
	(void)tc;
	m_abilities = abilities;
}

GridMate::ReplicaChunkPtr UnitAbilityComponent::GetNetworkBinding()
{
    auto replicaChunk = GridMate::CreateReplicaChunk<UnitAbilityReplicaChunk>();
    replicaChunk->SetHandler(this);
    m_replicaChunk = replicaChunk;

    return m_replicaChunk;
}

void UnitAbilityComponent::SetNetworkBinding(GridMate::ReplicaChunkPtr chunk)
{
	chunk->SetHandler(this);
	m_replicaChunk = chunk;

    UnitAbilityReplicaChunk* abilitiesChunk = static_cast<UnitAbilityReplicaChunk*>(m_replicaChunk.get());
	m_abilities = abilitiesChunk->m_abilities.Get();
}

void UnitAbilityComponent::UnbindFromNetwork()
{
    m_replicaChunk->SetHandler(nullptr);
    m_replicaChunk = nullptr;
}

void UnitAbilityComponent::SyncAbilityList ()
{
    if (m_replicaChunk && AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
    {
        // If you are authoritative over the entity and the component is replicated, update the value of the DataSet and propagate to clients 
        auto abilitiesChunk = static_cast<UnitAbilityReplicaChunk*>(m_replicaChunk.get());
        abilitiesChunk->m_abilities.Set(m_abilities);
    }
}

AZ::EntityId UnitAbilityComponent::GetAbilityInSlot(Slot slot) {
	if (!slot.IsValid())return AZ::EntityId();

	//AZ_Printf(0, "UnitAbilityComponent::GetAbilityInSlot slot: %s", slot.ToString().c_str());

	auto it = m_abilities.find(slot);
	return it != m_abilities.end() ? it->second : AZ::EntityId();
}

bool UnitAbilityComponent::HasAbilityInSlot(Slot slot) {
	if (!slot.IsValid())return false;

	auto it = m_abilities.find(slot);
	return it != m_abilities.end() ? it->second.IsValid() : false;
}

void UnitAbilityComponent::SetAbilityInSlot(Slot slot, AZ::EntityId ability) {
	if (!slot.IsValid())return;

	if (!ability.IsValid()) {
		ClearAbilitySlot(slot);
		return;
	}

	auto it = m_abilities.find(slot);
	if( it != m_abilities.end())
	{//we already have an ability attached
		//if is the same ability we are trying to attach, just return
		if (it->second == ability)return;

		//detach it
		EBUS_EVENT_ID(it->second, AbilityNotificationBus, OnDetached, GetEntityId());
		EBUS_EVENT(AbilitiesNotificationBus, OnDetached, it->second, GetEntityId());
		EBUS_EVENT_ID(GetEntityId(), UnitAbilityNotificationBus, OnAbilityDetached, it->second);
	}

	m_abilities[slot] = ability;
	SyncAbilityList();

	AZ_Printf(0, "[%s]UnitAbilityComponent:  attaching ability %s in %s", GetEntityId().ToString().c_str(), ability.ToString().c_str(), slot.ToString().c_str());
	EBUS_EVENT_ID(ability, AbilityNotificationBus, OnAttached, GetEntityId());
	EBUS_EVENT(AbilitiesNotificationBus, OnAttached, it->second, GetEntityId());
	EBUS_EVENT_ID(GetEntityId(), UnitAbilityNotificationBus, OnAbilityAttached, ability);
}

void UnitAbilityComponent::ClearAbilitySlot(Slot slot) {
	if (!slot.IsValid())return;

	//AZ_Printf(0, "UnitAbilityComponent::ClearAbilitySlot slot: %s", slot.ToString().c_str());
	//AZ_Printf(0, "UnitAbilityComponent before: %s", ToString().c_str());
	auto it = m_abilities.find(slot);
	if (it != m_abilities.end())
	{
		//detach it
		EBUS_EVENT_ID(it->second, AbilityNotificationBus, OnDetached, GetEntityId());
		EBUS_EVENT(AbilitiesNotificationBus, OnDetached, it->second, GetEntityId());
		EBUS_EVENT_ID(GetEntityId(), UnitAbilityNotificationBus, OnAbilityDetached, it->second);
		m_abilities.erase(it);
		SyncAbilityList();
	}
	//AZ_Printf(0, "UnitAbilityComponent after: %s", ToString().c_str());
}

void UnitAbilityComponent::ClearAllAbilities()
{
	//sLOG("UnitAbilityComponent:ClearAllAbilities()");
	for(auto &it : m_abilities)
	{
		EBUS_EVENT_ID(it.second, AbilityNotificationBus, OnDetached, GetEntityId());
		EBUS_EVENT(AbilitiesNotificationBus, OnDetached, it.second, GetEntityId());
		EBUS_EVENT_ID(GetEntityId(), UnitAbilityNotificationBus, OnAbilityDetached, it.second);
		EBUS_EVENT(AzFramework::GameEntityContextRequestBus, DestroyGameEntityAndDescendants, it.second);
	}
	m_abilities.clear();
	SyncAbilityList();
}

AbilityId UnitAbilityComponent::HasAbilityType(AbilityTypeId type)
{
	for (auto it : m_abilities)
	{
		AbilityTypeId it_type;
		EBUS_EVENT_ID_RESULT(it_type, it.second, AbilityRequestBus, GetAbilityTypeId);
		if (it_type == type)return it.second;
	}
	return AbilityId();
}

bool UnitAbilityComponent::HasAbility(AbilityId id)
{
	for (auto it : m_abilities)
	{
		if (it.second == id)return true;
	}
	return false;
}

Slot spark::UnitAbilityComponent::GetAbilitySlot(AbilityId id)
{
	for (auto it : m_abilities)
	{
		if (it.second == id)return it.first;
	}
	return Slot();
}

void UnitAbilityComponent::DetachAbility(AbilityId id)
{
	ClearAbilitySlot(GetAbilitySlot(id));
}

AZStd::vector<AbilityId> UnitAbilityComponent::GetAbilities()
{
	AZStd::vector<AbilityId> abilities;
	for (auto it : m_abilities)
	{
		abilities.push_back(it.second);
	}
	return abilities;
}


bool UnitAbilityComponent::CanCastAbility(AZ::EntityId ability)
{
	//check if the ability exists
	if (!ability.IsValid())
	{
		sWARNING("Ability doesn't exist!");
		return false;
	}

	//check if the ability is learned
	int level;
	EBUS_EVENT_ID_RESULT(level, ability, AbilityRequestBus, GetLevel);
	if (level<=0)
	{
		sWARNING("Ability is not learned!");
		return false;
	}


	//check if the ability is ready(not in cooldown)
	Value cooldown_timer=0;
	EBUS_EVENT_RESULT(cooldown_timer, VariableManagerRequestBus, GetValue, VariableId(ability, "cooldown_timer"));
	if (cooldown_timer > 0)
	{
		sWARNING("Ability is on cooldown!");
		return false;
	}

	//check if the unit can cover the costs of the casting
	Costs costs;
	EBUS_EVENT_ID_RESULT(costs, ability, AbilityRequestBus, GetCosts);
	if (!HasEnoughResources(GetEntityId(), costs))
	{
		return false;
	}
	
	return true;
}

AZ::EntityId UnitAbilityComponent::CanCastAbilityInSlot(Slot slot)
{
	AZ::EntityId ability = GetAbilityInSlot(slot);

	Status status;
	EBUS_EVENT_ID_RESULT(status, GetEntityId(), UnitRequestBus, GetStatus);

	if (slot.GetType() == Slot::Ability) {
		if (status & CANT_CAST_ABILITIES)
		{
			return AZ::EntityId();
		}
	}
	if (slot.GetType() == Slot::Inventory) {
		if (status & CANT_CAST_ITEMS)
		{
			return AZ::EntityId();
		}
	}

	return CanCastAbility(ability) ? ability : AZ::EntityId();
}

void UnitAbilityComponent::CastAbility(CastContext context)
{
	sLOG("UnitAbilityComponent::CastAbility()");
	if (m_casting)
	{	//interrupt current casting
		InterruptCasting();
	}
	m_castContext = context;
	m_casting = true;


	CastContextRequestBus::Handler::BusConnect(m_castContext.ability);
	UnitNavigationNotificationBus::Handler::BusConnect(GetEntityId());
	
	m_stateMachine.ResizeStack(0);
	m_stateMachine.PushState(S_FORESWING);

	AZ::TickBus::Handler::BusConnect();
}

void UnitAbilityComponent::InterruptCasting()
{
	sLOG("UnitAbilityComponent::InterruptCasting()");
	m_casting = false;
	m_stateMachine.Dispatch(SSM::Event(E_STOP));
	m_stateMachine.ResizeStack(0);
	CastContextRequestBus::Handler::BusDisconnect();
	AZ::TickBus::Handler::BusDisconnect();
	UnitNavigationNotificationBus::Handler::BusDisconnect();
}

//Cast Context

bool UnitAbilityComponent::IsItem()
{
	return m_castContext.isItem;
}

UnitId UnitAbilityComponent::GetCaster()
{
	return m_castContext.caster;
}

UnitId UnitAbilityComponent::GetCursorTarget()
{
	return m_castContext.target;
}

AZ::Vector3 UnitAbilityComponent::GetCursorPosition()
{
	return m_castContext.cursorPosition;
}

CastingBehavior UnitAbilityComponent::GetBehaviorUsed()
{
	return m_castContext.behaviorUsed;
}

AZStd::string spark::UnitAbilityComponent::ToString()
{
	AZStd::string str;
	str += "Unit has:\n";

	for (auto it : m_abilities) {
		AZStd::string name;
		EBUS_EVENT_ID_RESULT(name, it.second, AbilityRequestBus, GetAbilityTypeId);
		str += AZStd::string::format("%s -> %s [%s]\n", it.first.ToString().c_str(), name.c_str(),it.second.IsValid() ? it.second.ToString().c_str() :"invalid");
	}

	return str;
}



//State machine


void UnitAbilityComponent::Init()
{
	m_stateMachine.SetStateHandler(SSM_STATE_NAME(S_IDLE),      SSM::StateHandler(this, &UnitAbilityComponent::OnStateIdle));
	m_stateMachine.SetStateHandler(SSM_STATE_NAME(S_FORESWING), SSM::StateHandler(this, &UnitAbilityComponent::OnStateForeswing));
	m_stateMachine.SetStateHandler(SSM_STATE_NAME(S_BACKSWING), SSM::StateHandler(this, &UnitAbilityComponent::OnStateBackswing));
	m_stateMachine.SetStateHandler(SSM_STATE_NAME(S_MOVING),    SSM::StateHandler(this, &UnitAbilityComponent::OnStateMoving));

	m_stateMachine.SetOnEmptyStackHandler(SSM::EmptyStackHandler(this, &UnitAbilityComponent::OnCastFinished));

	m_stateMachine.Start(S_IDLE);

	

	AZ_Printf(0,"UnitAbilityComponent::Init()");
}

void UnitAbilityComponent::Activate()
{
	UnitAbilityRequestBus::Handler::BusConnect(GetEntityId());

	AZ_Printf(0,"UnitAbilityComponent::Activate()");
}

void UnitAbilityComponent::Deactivate()
{
	ClearAllAbilities();

	UnitAbilityRequestBus::Handler::BusDisconnect();
}


void UnitAbilityComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time)
{
	if (m_casting) {
		m_elapsedTime += deltaTime;

		m_stateMachine.Dispatch(SSM::Event(E_UPDATE));
	}
}

void UnitAbilityComponent::OnMovingDone()
{
	AZ_Printf(0, "UnitAbilityComponent::OnMovingDone");
	m_stateMachine.Dispatch(SSM::Event(E_TRAVERSAL_COMPLETE));
}

void UnitAbilityComponent::OnDestinationUnreachable()
{
	AZ_Printf(0, "UnitAbilityComponent::OnDestinationUnreachable state: %d", m_stateMachine.GetCurrentState());
	m_stateMachine.Dispatch(SSM::Event(E_DESTINATION_UNREACHABLE));
}



bool UnitAbilityComponent::OnStateIdle(SSM& m, const SSM::Event& e)
{
	switch (e.id) {
	case SSM::EnterEventId:
	{
		//start idle animation
		m_elapsedTime = 0.0f;
		return true; 
	}
	case E_UPDATE:
	{
		if (m_casting) {
			
			m.PopState();
			m.PushState(S_FORESWING);
		}
		return true;
	}
	}
	return false;
}

bool UnitAbilityComponent::OnStateForeswing(SSM& m, const SSM::Event& e)
{
	switch (e.id) {
	case SSM::OnResume:
	case SSM::EnterEventId:
	{
		AZ_Printf(0, "EnterEventId in OnStateForeswing");
		//check if the unit is casting
		if (!m_casting) 
		{
			m.ChangeState(S_IDLE);
			return true;
		}

		Status status = 0;
		EBUS_EVENT_ID_RESULT(status, GetEntityId(), UnitRequestBus, GetStatus);

		//check if muted/sileced
		if ((m_castContext.isItem && (status & CANT_CAST_ITEMS)) || (!m_castContext.isItem && (status & CANT_CAST_ABILITIES))
		//check the ability can be casted
		|| !CanCastAbility(m_castContext.ability))
		{
			InterruptCasting();
			m.PopState();
			return true;
		}

		//check if in position
		if(!m_castContext.behaviorUsed.Contains(CastingBehavior::IMMEDIATE) && m_castContext.target!=m_castContext.caster){
			//check if in range
			if (GetDistanceFromTarget() > GetMaxRange())
			{
				m.PushState(S_MOVING);
				return true;
			}

			//check if the target unit is in range
			AZ::Vector3 a, b = GetTargetPosition();
			EBUS_EVENT_ID_RESULT(a, GetEntityId(), AZ::TransformBus, GetWorldTranslation);
			a.SetZ(0); b.SetZ(0);

			float distance = a.GetDistance(b);
			if (distance > GetMaxRange()) {
				m.PushState(S_MOVING);
				return true;
			}

			//check if the unit is facing the correct direction
			AZ::Vector3 target_direction = b - a;
			target_direction.Normalize();
			bool ok = true;
			EBUS_EVENT_ID_RESULT(ok, GetEntityId(), UnitNavigationRequestBus, IsFacingDirection, target_direction);
			if (!ok)
			{
				sLOG("UnitAbilityComponent not facing the correct direction");
				m.PushState(S_MOVING);
				return true;
			}
		}

		//OnAbilityPhaseStartFilter
		if ( SPARK_FILTER_EVENT_PREVENT(AbilitiesNotificationBus, OnAbilityPhaseStartFilter, m_castContext.ability)
			|| SPARK_FILTER_EVENT_ID_PREVENT(m_castContext.ability, AbilityNotificationBus, OnAbilityPhaseStartFilter))
		{
			InterruptCasting();
			m.PopState(); 
			return true; 
		}

		EBUS_EVENT_ID(GetEntityId(), UnitNavigationRequestBus, Stop);

		AZ_Printf(0, "start cast animation");
		//start cast animation
		m_elapsedTime = 0.0f;

		EBUS_EVENT(AbilitiesNotificationBus, OnAbilityPhaseStart, m_castContext.ability);
		EBUS_EVENT_ID(m_castContext.ability, AbilityNotificationBus, OnAbilityPhaseStart);

		return true;
	}
	case E_UPDATE:
	{
		if (!CanCastAbility(m_castContext.ability))
		{
			InterruptCasting();
			m.PopState(); 
			return true; 
		}

		if (m_casting && m_elapsedTime >= GetCastPoint()) 
		{
			AZ_Printf(0, "Cast point");

			if (!(SPARK_FILTER_EVENT_PREVENT(AbilitiesNotificationBus, OnSpellStartFilter, m_castContext.ability)
				|| SPARK_FILTER_EVENT_ID_PREVENT(m_castContext.ability, AbilityNotificationBus, OnSpellStartFilter)))
			{

				if (AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
				{
					//set cooldown
					Value cooldown = 0;
					bool ok = false;
					EBUS_EVENT_RESULT(cooldown, VariableManagerRequestBus, GetValue, VariableId(m_castContext.ability, "cooldown"));
					EBUS_EVENT_RESULT(ok, VariableManagerRequestBus, SetValue, VariableId(m_castContext.ability, "cooldown_timer"), cooldown);
					EBUS_EVENT_ID(m_castContext.ability, CooldownNotificationBus, OnCooldownStarted, cooldown);

					sLOG("cooldown_timer updated : " + ok);

					//consume resources
					Costs costs;
					EBUS_EVENT_ID_RESULT(costs, m_castContext.ability, AbilityRequestBus, GetCosts);
					ConsumeResources(GetEntityId(), costs);
				}

				AZ_Printf(0, "OnStateCast OnSpellStart");
				EBUS_EVENT(AbilitiesNotificationBus, OnSpellStart, m_castContext.ability);
				EBUS_EVENT_ID(m_castContext.ability, AbilityNotificationBus, OnSpellStart);
			}

			m.ChangeState(S_BACKSWING);
		}
		return true;
	}
	}
	return false;
}

bool UnitAbilityComponent::OnStateBackswing(SSM& m, const SSM::Event& e)
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
		if (m_elapsedTime >= GetBackswingPoint()) 
		{
			m.PopState();
		}
		return true;
	}
	}
	return false;
}

bool UnitAbilityComponent::OnStateMoving(SSM& m, const SSM::Event& e)
{
	switch (e.id) {
	case SSM::EnterEventId:
	{
		AZ_Printf(0, "following started");

		//request moving
		if (m_castContext.behaviorUsed.GetFlags() & CastingBehavior::CastingBehavior::TargetingBehavior::UNIT_TARGET)
		{	
			EBUS_EVENT_ID(GetEntityId(), UnitNavigationRequestBus, Follow, m_castContext.target, GetMaxRange());
		}
		else
		{
			spark::PathfindRequest request;
			request.SetDestinationLocation(GetTargetPosition());
			request.SetArrivalDistanceThreshold(GetMaxRange());
			EBUS_EVENT_ID(GetEntityId(), UnitNavigationRequestBus, SendPathfindRequest, request);
		}

		return true; 
	}
	//case E_UPDATE: //necessary beacause follow doesn't work as intended yet
	//{
	//	float distance=0.0f;
	//	EBUS_EVENT_RESULT(distance, GameManagerRequestBus, GetDistance, GetEntityId(), m_castContext.target);

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
		m.PopState();
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

void UnitAbilityComponent::OnCastFinished()
{
	InterruptCasting();
	EBUS_EVENT_ID(GetEntityId(), UnitAbilityNotificationBus, OnCastFinished);
}

AZ::Vector3 UnitAbilityComponent::GetTargetPosition()
{
	switch (m_castContext.behaviorUsed.GetFlags() & CastingBehavior::TargetingBehavior::TARGETING_BEHAVIOR_MASK)
	{
	case CastingBehavior::TargetingBehavior::POINT_TARGET:
	{
		return m_castContext.cursorPosition;
	}
	case CastingBehavior::TargetingBehavior::UNIT_TARGET:
	{
		AZ::Vector3 pos = m_castContext.cursorPosition;
		EBUS_EVENT_ID_RESULT(pos, m_castContext.target, AZ::TransformBus, GetWorldTranslation);
		return pos;
	}
	default:
	{
		AZ::Vector3 pos = m_castContext.cursorPosition;
		EBUS_EVENT_ID_RESULT(pos, m_castContext.caster, AZ::TransformBus, GetWorldTranslation);
		return pos;
	}
	}
}

float UnitAbilityComponent::GetDistanceFromTarget()
{
	AZ::Vector3 pos;
	EBUS_EVENT_ID_RESULT(pos, m_castContext.caster, AZ::TransformBus, GetWorldTranslation);

	return Distance2D(pos,GetTargetPosition());
}






float UnitAbilityComponent::GetMaxRange()
{
	Value value=10;
	EBUS_EVENT_RESULT(value, VariableManagerRequestBus, GetValue, VariableId(m_castContext.ability, "max_range"));
	return AZStd::max(1.0f,value) * MOVEMENT_SCALE;
}

float UnitAbilityComponent::GetCastPoint()
{
	return GetBaseCastPoint()/(1.0f+GetIncreasedCastSpeed());
}

float UnitAbilityComponent::GetBackswingPoint()
{
	return GetBaseBackswingPoint()/(1.0f+GetIncreasedCastSpeed());
}


float UnitAbilityComponent::GetBaseCastPoint()
{
	Value value=0;
	EBUS_EVENT_RESULT(value, VariableManagerRequestBus, GetValue, VariableId(m_castContext.ability, "cast_point"));
	return value;
	//return m_baseCastPoint;
}
float UnitAbilityComponent::GetBaseBackswingPoint()
{
	Value value=1;
	EBUS_EVENT_RESULT(value, VariableManagerRequestBus, GetValue, VariableId(m_castContext.ability, "cast_time"));
	return value-GetBaseCastPoint();
	//return m_baseBackswingPoint;
}


float UnitAbilityComponent::GetIncreasedCastSpeed()
{
	Value value=0;
	EBUS_EVENT_RESULT(value, VariableManagerRequestBus, GetValue, VariableId(m_castContext.caster, "ICS"));
	return value;
}
void  UnitAbilityComponent::SetIncreasedCastSpeed(float value)
{
	//m_ICS = value;
}
