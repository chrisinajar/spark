
#include "spark_precompiled.h"

#include "UnitComponent.h"

#include <AzCore/Component/TransformBus.h>
#include <AzCore/Math/Transform.h>
#include <AzFramework/Entity/GameEntityContextComponent.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Math/MathUtils.h>

#include <AzFramework/Network/NetworkContext.h>
#include <GridMate/Replica/ReplicaChunk.h>
#include <GridMate/Replica/ReplicaFunctions.h>
#include "Utils/Marshaler.h"
// NetQuery
#include <AzFramework/Network/NetBindingHandlerBus.h>

#include "Busses/UnitAttackBus.h"
#include "Busses/SlotBus.h"
#include "Busses/SelectionBus.h"
#include "Busses/UnitAbilityBus.h"
#include "Utils/Log.h"
#include "Utils/GridMate.h"
#include "Busses/ModifierBus.h"
#include "Busses/StaticDataBus.h"
#include "Busses/NetSyncBusses.h"
#include "Busses/GameManagerBus.h"
#include "Busses/ShopBus.h"


#include <LmbrCentral/Rendering/MeshComponentBus.h>
#include <Integration/AnimGraphComponentBus.h>

using namespace spark;
using namespace AzFramework;


/**
*	Interface to lua scripts
*/
class UnitNotificationBusHandler
	: public UnitNotificationBus::Handler
	, public AZ::BehaviorEBusHandler
{
public:
	AZ_EBUS_BEHAVIOR_BINDER(UnitNotificationBusHandler, "{D2CA3017-0430-4F1E-80AD-2034E8E6B261}", AZ::SystemAllocator,
	OnNewOrder, OnModifierAttached, OnModifierDetached, OnSpawned, OnDeath, OnKilled);
	
	void OnNewOrder (UnitOrder order, bool queue) override
	{
		Call(FN_OnNewOrder, order, queue);
	}
	void OnModifierAttached (ModifierId id) override
	{
		Call(FN_OnModifierAttached, id);
	}
	void OnModifierDetached (ModifierId id) override
	{
		Call(FN_OnModifierDetached, id);
	}
	void OnSpawned () override
	{
		Call(FN_OnSpawned);
	}
	void OnDeath () override
	{
		Call(FN_OnDeath);
	}
	void OnKilled (Damage damage) override
	{
		Call(FN_OnKilled, damage);
	}
};

class UnitsNotificationBusHandler
	: public UnitsNotificationBus::Handler
	, public AZ::BehaviorEBusHandler
{
public:
	AZ_EBUS_BEHAVIOR_BINDER(UnitsNotificationBusHandler, "{1D9ED5BB-A445-4C5B-9781-9C3D8E1C0FFC}", AZ::SystemAllocator,
		OnUnitSpawned, OnUnitDeath, OnUnitCreated, OnUnitKilled);

	void OnUnitSpawned(UnitId id) override
	{
		Call(FN_OnUnitSpawned, id);
	}

	void OnUnitDeath(UnitId id) override
	{
		Call(FN_OnUnitDeath, id);
	}

	void OnUnitCreated(UnitId id, AZStd::string name) override
	{
		Call(FN_OnUnitCreated, id, name);
	}

	void OnUnitKilled(UnitId id, Damage damage) override
	{
		Call(FN_OnUnitKilled, id, damage);
	}
};

class UnitReplicaChunk : public GridMate::ReplicaChunkBase
{
public:
	AZ_CLASS_ALLOCATOR(UnitReplicaChunk, AZ::SystemAllocator, 0);

	static const char* GetChunkName() { return "UnitReplicaChunk"; }

	   UnitReplicaChunk()
		: m_name("Name")
		, m_teamId("TeamId")
		, m_quickBuyItemList("QuickBuyItemList")
		, m_modifiers("Modifiers")
		, m_status("Status")
		, m_isAlive("IsAlive")
		, m_playerOwner("PlayerOwner")
	{
	}

	bool IsReplicaMigratable()
	{
		return true;
	}

	GridMate::DataSet<AZStd::string>::BindInterface<UnitComponent, &UnitComponent::OnNewName> m_name;
	GridMate::DataSet<TeamId>::BindInterface<UnitComponent, &UnitComponent::OnNewTeamId> m_teamId;
	GridMate::DataSet<AZStd::vector<ItemTypeId>>::BindInterface<UnitComponent, &UnitComponent::OnNewQuickBuyItemList> m_quickBuyItemList;
	GridMate::DataSet<AZStd::vector<ModifierId>>::BindInterface<UnitComponent, &UnitComponent::OnNewModifiers> m_modifiers;
	GridMate::DataSet<Status>::BindInterface<UnitComponent, &UnitComponent::OnNewStatus> m_status;
	GridMate::DataSet<bool>::BindInterface<UnitComponent, &UnitComponent::OnNewIsAlive> m_isAlive;
	GridMate::DataSet<AZ::EntityId>::BindInterface<UnitComponent, &UnitComponent::OnNewPlayerOwner> m_playerOwner;
};

void UnitOrder::Reflect(AZ::ReflectContext* reflection)
{
	if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
	{
		serializationContext->Class<UnitOrder>()
			->Version(1)
			->Field("type", &UnitOrder::type)
			->Field("position", &UnitOrder::position)
			->Field("target", &UnitOrder::target)
			->Field("typeId", &UnitOrder::typeId)
			->Field("distance", &UnitOrder::distance)
			->Field("castContext", &UnitOrder::castContext)
			;

	}
	if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
	{
		behaviorContext
			->Enum<(int)UnitOrder::MOVE>("UNIT_ORDER_MOVE")
			->Enum<(int)UnitOrder::FOLLOW>("UNIT_ORDER_FOLLOW")
			->Enum<(int)UnitOrder::ATTACK>("UNIT_ORDER_ATTACK")
			->Enum<(int)UnitOrder::ATTACK_MOVE>("UNIT_ORDER_ATTACK_MOVE")
			->Enum<(int)UnitOrder::STOP>("UNIT_ORDER_STOP")
			->Enum<(int)UnitOrder::CAST>("UNIT_ORDER_CAST")
			->Enum<(int)UnitOrder::BUY>("UNIT_ORDER_BUY")
			->Enum<(int)UnitOrder::SELL>("UNIT_ORDER_SELL")
			->Enum<(int)UnitOrder::DROP>("UNIT_ORDER_DROP")
			->Enum<(int)UnitOrder::PICKUP>("UNIT_ORDER_PICKUP")
			->Enum<(int)UnitOrder::UPGRADE>("UNIT_ORDER_UPGRADE")
			;

		behaviorContext->Class<UnitOrder>("UnitOrder")
			->Attribute(AZ::Script::Attributes::Storage, AZ::Script::Attributes::StorageType::Value)
			->Property("type",  BehaviorValueProperty(&UnitOrder::type))
			->Property("position",  BehaviorValueProperty(&UnitOrder::position))
			->Property("target",  BehaviorValueProperty(&UnitOrder::target))
			->Property("typeId",  BehaviorValueProperty(&UnitOrder::typeId))
			->Property("distance",  BehaviorValueProperty(&UnitOrder::distance))
			->Property("castContext",  BehaviorValueProperty(&UnitOrder::castContext))
			->Method("ToString", &UnitOrder::ToString);
			;

		behaviorContext->Method("MoveOrder", &UnitOrder::MoveOrder);
		behaviorContext->Method("FollowOrder", &UnitOrder::FollowOrder);
		behaviorContext->Method("AttackOrder", &UnitOrder::AttackOrder);
		behaviorContext->Method("AttackMoveOrder", &UnitOrder::AttackMoveOrder);
		behaviorContext->Method("StopOrder", &UnitOrder::StopOrder);
		behaviorContext->Method("CastOrder", &UnitOrder::CastOrder);
		behaviorContext->Method("BuyItemOrder", &UnitOrder::BuyItemOrder);
		behaviorContext->Method("SellItemOrder", &UnitOrder::SellItemOrder);
		behaviorContext->Method("DropItemOrder", &UnitOrder::DropItemOrder);
		behaviorContext->Method("PickUpItemOrder", &UnitOrder::PickUpItemOrder);
		behaviorContext->Method("UpgradeOrder", &UnitOrder::UpgradeOrder);
	}
}

void UnitComponent::Reflect(AZ::ReflectContext* reflection)
{
	Slot::Reflect(reflection);

	if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
	{
		serializationContext->Class<UnitComponent, AzFramework::NetBindable, AZ::Component>()
			->Version(2)
			->Field("Name", &UnitComponent::m_name)
			->Field("TeamId", &UnitComponent::m_teamId);

		if (auto editContext = serializationContext->GetEditContext())
		{
			editContext->Class<UnitComponent>("UnitComponent", "Base unit for all units")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
				->Attribute(AZ::Edit::Attributes::Category, "spark")
				->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"))
				->DataElement(nullptr, &UnitComponent::m_name, "Name", "The name of the unit")
				->DataElement(nullptr, &UnitComponent::m_teamId, "TeamId", "The team id of the unit");
		}
	}

	if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
	{

		behaviorContext->EBus<UnitRequestBus>("UnitRequestBus")
			->Attribute(AZ::Script::Attributes::Category, "Unit")
			->Event("GetName", &UnitRequestBus::Events::GetName)
			->Event("SetName", &UnitRequestBus::Events::GetName)
			->Event("GetUnitTypeId", &UnitRequestBus::Events::GetUnitTypeId)
			->Event("SetUnitTypeId", &UnitRequestBus::Events::SetUnitTypeId)
			->Event("GetTeamId", &UnitRequestBus::Events::GetTeamId)
			->Event("SetTeamId", &UnitRequestBus::Events::SetTeamId)
			->Event("GetPlayerOwner", &UnitRequestBus::Events::GetPlayerOwner)
			->Event("IsAlive", &UnitRequestBus::Events::IsAlive)
			->Event("SetAlive", &UnitRequestBus::Events::SetAlive)
			->Event("IsVisible", &UnitRequestBus::Events::IsVisible)
			->Event("SetVisible", &UnitRequestBus::Events::SetVisible)
			->Event("Kill", &UnitRequestBus::Events::Kill)
			->Event("KillWithAbility", &UnitRequestBus::Events::KillWithAbility)
			->Event("GetStatus", &UnitRequestBus::Events::GetStatus)
			->Event("SetStatus", &UnitRequestBus::Events::SetStatus)
			->Event("GetQuickBuyItemList", &UnitRequestBus::Events::GetQuickBuyItemList)
			->Event("SetQuickBuyItemList", &UnitRequestBus::Events::SetQuickBuyItemList)
			->Event("GetModifiers", &UnitRequestBus::Events::GetModifiers)
			->Event("SetModifiers", &UnitRequestBus::Events::SetModifiers)
			->Event("ClearModifiers", &UnitRequestBus::Events::ClearModifiers)
			->Event("AddModifier", &UnitRequestBus::Events::AddModifier)
			->Event("RemoveModifier", &UnitRequestBus::Events::RemoveModifier)
			->Event("FindModifierByTypeId", &UnitRequestBus::Events::FindModifierByTypeId)
			->Event("ApplyDispel", &UnitRequestBus::Events::ApplyDispel)
			->Event("GetCurrentOrder", &UnitRequestBus::Events::GetCurrentOrder)
			->Event("NewOrder", &UnitRequestBus::Events::NewOrder)
			->Event("ToString", &UnitRequestBus::Events::ToString)
			->Event("Destroy", &UnitRequestBus::Events::Destroy)
			;
		
		behaviorContext->EBus<UnitNotificationBus>("UnitNotificationBus")
			->Handler<UnitNotificationBusHandler>()
			->Event("OnNewOrder", &UnitNotificationBus::Events::OnNewOrder)
			->Event("OnModifierAttached", &UnitNotificationBus::Events::OnModifierAttached)
			->Event("OnModifierDetached", &UnitNotificationBus::Events::OnModifierDetached)
			->Event("OnSpawned", &UnitNotificationBus::Events::OnSpawned)
			->Event("OnDeath", &UnitNotificationBus::Events::OnDeath)
			->Event("OnKilled", &UnitNotificationBus::Events::OnKilled)
			;

		behaviorContext->EBus<UnitsNotificationBus>("UnitsNotificationBus")
			->Handler<UnitsNotificationBusHandler>()
			->Event("OnUnitSpawned", &UnitsNotificationBus::Events::OnUnitSpawned)
			->Event("OnUnitDeath", &UnitsNotificationBus::Events::OnUnitDeath)
			->Event("OnUnitCreated", &UnitsNotificationBus::Events::OnUnitCreated)
			->Event("OnUnitKilled", &UnitsNotificationBus::Events::OnUnitKilled)
			;

		behaviorContext
			->Enum<(int)DEAD>("UNIT_STATUS_DEAD")
			->Enum<(int)ATTACK_IMMUNITY>("UNIT_STATUS_ATTACK_IMMUNITY")
			->Enum<(int)DISARM>("UNIT_STATUS_DISARM")
			->Enum<(int)ETHEREAL>("UNIT_STATUS_ETHEREAL")
			->Enum<(int)HEX>("UNIT_STATUS_HEX")
			->Enum<(int)INVULNERABILITY>("UNIT_STATUS_INVULNERABILITY")
			->Enum<(int)SILENCE>("UNIT_STATUS_SILENCE")
			->Enum<(int)MUTED>("UNIT_STATUS_MUTED")
			->Enum<(int)ROOTED>("UNIT_STATUS_ROOTED")
			->Enum<(int)GROUNDED>("UNIT_STATUS_GROUNDED")
			->Enum<(int)STUN>("UNIT_STATUS_STUN")
			->Enum<(int)BROKEN>("UNIT_STATUS_BROKEN")
			->Enum<(int)FEAR>("UNIT_STATUS_FEAR")
			->Enum<(int)FLYING_MOVEMENT>("UNIT_STATUS_FLYING_MOVEMENT")
			->Enum<(int)FORCED_MOVEMENT>("UNIT_STATUS_FORCED_MOVEMENT")
			->Enum<(int)STEALTH>("UNIT_STATUS_STEALTH")
			->Enum<(int)PHASED>("UNIT_STATUS_PHASED")
			->Enum<(int)STATUS_IMMUNITY>("UNIT_STATUS_STATUS_IMMUNITY")
			->Enum<(int)TAUNT>("UNIT_STATUS_TAUNT")
			->Enum<(int)BLIND>("UNIT_STATUS_BLIND")
			->Enum<(int)LEASH>("UNIT_STATUS_LEASH")
			->Enum<(int)HIDDEN>("UNIT_STATUS_HIDDEN")
			->Enum<(int)BLUR>("UNIT_STATUS_BLUR")
			->Enum<(int)SPELL_IMMUNITY>("UNIT_STATUS_SPELL_IMMUNITY")
			->Enum<(int)CANT_MOVE>("UNIT_STATUS_CANT_MOVE")
			->Enum<(int)CANT_ROTATE>("UNIT_STATUS_CANT_ROTATE")
			->Enum<(int)CANT_ATTACK>("UNIT_STATUS_CANT_ATTACK")
			->Enum<(int)CANT_CAST_ABILITIES>("UNIT_STATUS_CANT_CAST_ABILITIES")
			->Enum<(int)CANT_CAST_ITEMS>("UNIT_STATUS_CANT_CAST_ITEMS")
			->Enum<(int)CANT_BE_ATTACKED>("UNIT_STATUS_CANT_BE_ATTACKED")
			->Enum<(int)CANT_BE_SPELL_TARGETED>("UNIT_STATUS_CANT_BE_SPELL_TARGETED")
			->Enum<(int)CANT_TELEPORT_OR_LEAP>("UNIT_STATUS_CANT_TELEPORT_OR_LEAP")
			->Enum<(int)CUSTOM_1>("UNIT_STATUS_CUSTOM_1")
			->Enum<(int)CUSTOM_2>("UNIT_STATUS_CUSTOM_2")
			->Enum<(int)CUSTOM_3>("UNIT_STATUS_CUSTOM_3")
			->Enum<(int)CUSTOM_4>("UNIT_STATUS_CUSTOM_4")
			->Enum<(int)MAX_STATUS>("UNIT_STATUS_MAX_STATUS")
			;

		AZ_Printf(0, "added UnitRequestBus to behaviorContext");
	}

	OnDamageTakenNotifications::Reflect(reflection);
	AttackEventsNotifications::Reflect(reflection);
	UnitOrder::Reflect(reflection);

	if (auto netContext = azrtti_cast<AzFramework::NetworkContext*>(reflection))
	{
		netContext->Class<UnitComponent>()
			->Chunk<UnitReplicaChunk>()
			->Field("Name", &UnitReplicaChunk::m_name)
			->Field("TeamId", &UnitReplicaChunk::m_teamId)
			->Field("QuickBuyItemList", &UnitReplicaChunk::m_quickBuyItemList)
			->Field("Modifiers", &UnitReplicaChunk::m_modifiers)
			->Field("Status", &UnitReplicaChunk::m_status)
			->Field("IsAlive", &UnitReplicaChunk::m_isAlive)
			->Field("PlayerOwner", &UnitReplicaChunk::m_playerOwner)
			;
	}
}

GridMate::ReplicaChunkPtr UnitComponent::GetNetworkBinding ()
{
	sDEBUG("UnitComponent::GetNetworkBinding()");
	auto replicaChunk = GridMate::CreateReplicaChunk<UnitReplicaChunk>();
	replicaChunk->SetHandler(this);
	m_replicaChunk = replicaChunk;

	if (AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		replicaChunk->m_name.Set(m_name);
		replicaChunk->m_teamId.Set(m_teamId);
		replicaChunk->m_quickBuyItemList.Set(m_quickBuyItemList);
		replicaChunk->m_modifiers.Set(m_modifiers);
		replicaChunk->m_status.Set(m_status);
		replicaChunk->m_isAlive.Set(m_isAlive);
	}
	else
	{
		m_name = replicaChunk->m_name.Get();
		m_teamId = replicaChunk->m_teamId.Get();
		m_quickBuyItemList = replicaChunk->m_quickBuyItemList.Get();
		m_modifiers = replicaChunk->m_modifiers.Get();
		m_status = replicaChunk->m_status.Get();
		m_isAlive = replicaChunk->m_isAlive.Get();
	}

	return m_replicaChunk;
}

void UnitComponent::SetNetworkBinding (GridMate::ReplicaChunkPtr chunk)
{
	sDEBUG("UnitComponent::SetNetworkBinding()");
	chunk->SetHandler(this);
	m_replicaChunk = chunk;

	UnitReplicaChunk* replicaChunk = static_cast<UnitReplicaChunk*>(m_replicaChunk.get());

	if (AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		replicaChunk->m_name.Set(m_name);
		replicaChunk->m_teamId.Set(m_teamId);
		replicaChunk->m_quickBuyItemList.Set(m_quickBuyItemList);
		replicaChunk->m_modifiers.Set(m_modifiers);
		replicaChunk->m_status.Set(m_status);
		replicaChunk->m_isAlive.Set(m_isAlive);
	}
	else
	{
		m_name = replicaChunk->m_name.Get();
		m_teamId = replicaChunk->m_teamId.Get();
		m_quickBuyItemList = replicaChunk->m_quickBuyItemList.Get();
		m_modifiers = replicaChunk->m_modifiers.Get();
		m_status = replicaChunk->m_status.Get();
		m_isAlive = replicaChunk->m_isAlive.Get();
	}
}

void UnitComponent::UnbindFromNetwork ()
{
	sDEBUG("UnitComponent::UnbindFromNetwork()");
	m_replicaChunk->SetHandler(nullptr);
	m_replicaChunk = nullptr;
}

void UnitComponent::OnNewName (const AZStd::string& name, const GridMate::TimeContext& tc)
{
	sDEBUG("UnitComponent::OnNewName()");
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		m_name = name;
	}
}

void UnitComponent::OnNewTeamId (const TeamId& teamId, const GridMate::TimeContext& tc)
{
	sDEBUG("UnitComponent::OnNewTeamId()");
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		m_teamId = teamId;
	}
}

void UnitComponent::OnNewQuickBuyItemList (const AZStd::vector<ItemTypeId>& quickBuyItemList, const GridMate::TimeContext& tc)
{
	sDEBUG("UnitComponent::OnNewQuickBuyItemList()");
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		m_quickBuyItemList = quickBuyItemList;
	}
}

void UnitComponent::OnNewModifiers (const AZStd::vector<ModifierId>& modifiers, const GridMate::TimeContext& tc)
{
	AZ_Printf(0, "UnitComponent::OnNewModifiers() %d", modifiers.size());
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		m_modifiers = modifiers;
	}
}

void UnitComponent::OnNewStatus (const Status& status, const GridMate::TimeContext& tc)
{
	sDEBUG("UnitComponent::OnNewStatus()");
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		m_status = status;
	}
}

void UnitComponent::OnNewPlayerOwner (const AZ::EntityId& playerOwner, const GridMate::TimeContext& tc)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		m_playerOwner = playerOwner;
	}
}

void UnitComponent::OnNewIsAlive (const bool& isAlive, const GridMate::TimeContext& tc)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		m_isAlive = isAlive;
	}
}

void UnitComponent::Init()
{

	m_stateMachine.SetStateHandler(SSM_STATE_NAME(S_IDLE), SSM::StateHandler(this, &UnitComponent::OnStateIdle));
	m_stateMachine.SetStateHandler(SSM_STATE_NAME(S_MOVE), SSM::StateHandler(this, &UnitComponent::OnStateMove));
	m_stateMachine.SetStateHandler(SSM_STATE_NAME(S_ATTACK), SSM::StateHandler(this, &UnitComponent::OnStateAttack));
	m_stateMachine.SetStateHandler(SSM_STATE_NAME(S_FOLLOW), SSM::StateHandler(this, &UnitComponent::OnStateFollow));
	m_stateMachine.SetStateHandler(SSM_STATE_NAME(S_CAST), SSM::StateHandler(this, &UnitComponent::OnStateCast));

	m_stateMachine.SetOnEmptyStackHandler(SSM::EmptyStackHandler(this, &UnitComponent::OrderDone));

	m_stateMachine.Start(S_IDLE);
}

void UnitComponent::Activate()
{
	AZ::TickBus::Handler::BusConnect();
	UnitRequestBus::Handler::BusConnect(GetEntityId());
	UnitsRequestBus::Handler::BusConnect();
	UnitNavigationNotificationBus::Handler::BusConnect(GetEntityId());
	UnitAbilityNotificationBus::Handler::BusConnect(GetEntityId());
	UnitAbilityNotificationBus::Handler::BusConnect(GetEntityId());
	UnitAttackNotificationBus::Handler::BusConnect(GetEntityId());

	OnDamageTakenRequestBus::Handler::BusConnect(GetEntityId());
	LocalizationNotificationBus::Handler::BusConnect();

	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		EBUS_EVENT(UnitsNotificationBus, OnUnitCreated, GetEntityId(), GetUnitTypeId());	
	}
}

void UnitComponent::Deactivate()
{
	ClearModifiers();

	LocalizationNotificationBus::Handler::BusDisconnect();

	OnDamageTakenRequestBus::Handler::BusDisconnect();
	UnitNavigationNotificationBus::Handler::BusDisconnect();
	UnitAbilityNotificationBus::Handler::BusDisconnect();
	UnitAttackNotificationBus::Handler::BusDisconnect();

	AZ::TickBus::Handler::BusDisconnect();
	UnitRequestBus::Handler::BusDisconnect();
	UnitsRequestBus::Handler::BusDisconnect();
}

void UnitComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time)
{
	const rapidjson::Value* info = nullptr;
	EBUS_EVENT_ID_RESULT(info, GetEntityId(), StaticDataRequestBus, GetJson);

	if (!m_hasSpawned && m_isAlive && info && info->HasMember("name"))
	{
		// add tags from json values
		EBUS_EVENT_ID(GetEntityId(), LmbrCentral::TagComponentRequestBus, AddTag, LmbrCentral::Tag("unit"));

		auto it = info->FindMember("tags");
		if (it != info->MemberEnd() && it->value.IsArray())//initialize tags
		{
			auto &tags = it->value;
			for (auto tag = tags.Begin(); tag != tags.End(); ++tag)
			{
				if (tag->IsString())
				{
					EBUS_EVENT_ID(GetEntityId(), LmbrCentral::TagComponentRequestBus, AddTag, LmbrCentral::Tag(tag->GetString()));
				}
			}
		}

		it = info->FindMember("is_hero");
		if (it != info->MemberEnd() && it->value.IsBool() && it->value.GetBool())//initialize tags
		{
			EBUS_EVENT_ID(GetEntityId(), LmbrCentral::TagComponentRequestBus, AddTag, LmbrCentral::Tag("hero"));
		}

		m_hasSpawned = true;
		EBUS_EVENT_ID(GetEntityId(), UnitNotificationBus, OnSpawned);
		EBUS_EVENT(UnitsNotificationBus, OnUnitSpawned, GetEntityId());
	}

	if (m_isDestroying)
	{
		// pretty sure that the eventbus disconnects will be called syncronously from this, but just in case lets prevent multiple deletion
		m_isDestroying = false;
		EBUS_EVENT(AzFramework::GameEntityContextRequestBus, DestroyGameEntityAndDescendants, GetEntityId());
	}
}

void UnitComponent::OnMovingDone()
{
	AZ_Printf(0, "UnitComponent::OnMovingDone");
	m_stateMachine.Dispatch(SSM::Event(E_TRAVERSAL_COMPLETE));
}

void UnitComponent::OnCastFinished()
{
	AZ_Printf(0, "UnitComponent::OnCastFinished");
	m_stateMachine.Dispatch(SSM::Event(E_CAST_FINISHED));
}

void spark::UnitComponent::OnAttackingInterrupted()
{
	AZ_Printf(0, "UnitComponent::OnAttackingInterrupted");
	m_stateMachine.Dispatch(SSM::Event(E_ATTACK_INTERRUPTED));
}


// metadata getters
bool UnitComponent::IsSelectable()
{
	sDEBUG("called UnitComponent::IsSelectable");
	return true;
}
bool UnitComponent::IsFriendly()
{
	return true;
}
bool UnitComponent::IsControllable()
{
	return true;
}
bool UnitComponent::IsFriendly(const AZ::EntityId &playerId)
{
	return playerId == m_playerOwner;
}
bool UnitComponent::IsControllable(const AZ::EntityId &playerId)
{
	return true;
}

AZ::EntityId UnitComponent::GetPlayerOwner()
{
	return m_playerOwner;
}

void UnitComponent::SetPlayerOwner(AZ::EntityId playerOwner)
{
	SPARK_UPDATE_VALUE(Unit, playerOwner);
}

bool UnitComponent::IsAlive()
{
	return m_isAlive;
}

void UnitComponent::KillWithAbility (AZ::EntityId abilityId, AZ::EntityId attackerId)
{
	Damage damage;
	damage.type = Damage::DAMAGE_TYPE_KILL;
	damage.source = attackerId;
	damage.ability = abilityId;
	damage.target = GetEntityId();

	Kill(damage);
}

void UnitComponent::Kill (Damage damage)
{
	EBUS_EVENT_ID(GetEntityId(), UnitNotificationBus, OnKilled, damage);
	EBUS_EVENT(UnitsNotificationBus, OnUnitKilled, GetEntityId(), damage);
	SetAlive(false);
}

void UnitComponent::SetAlive(bool alive)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return;
	}
	if (alive == m_isAlive)
	{
		return;
	}
	m_isAlive = alive;
	auto old_status = m_status;

	if (alive)
	{
		m_status &= ~DEAD; // remove the DEAD bit

		//if is not already alive
		EBUS_EVENT_ID(GetEntityId(), UnitNotificationBus, OnSpawned);
		EBUS_EVENT(UnitsNotificationBus, OnUnitSpawned, GetEntityId());
	}
	else 
	{
		m_status |= DEAD; // add the DEAD bit

		//if was alive and now is dead
		//stop
		EBUS_EVENT_ID(GetEntityId(), UnitNavigationRequestBus, Stop);
		//reset the state machine, will stop also the other components
		m_ordersList.clear();
		ExecuteCurrentOrder();

		EBUS_EVENT_ID(GetEntityId(), UnitNotificationBus, OnDeath);
		EBUS_EVENT(UnitsNotificationBus, OnUnitDeath, GetEntityId());

		ApplyDispel(Dispel::DeathDispel());
	}

	UpdateVisibility();

	EBUS_EVENT_ID(GetEntityId(), UnitNetSyncRequestBus, SetNamedParameterBool,"Dead",!alive);

	if (m_replicaChunk)
	{
		UnitReplicaChunk* replicaChunk = static_cast<UnitReplicaChunk*>(m_replicaChunk.get());
		if (old_status != m_status)
		{
			replicaChunk->m_status.Set(m_status);
		}
		replicaChunk->m_isAlive.Set(m_isAlive);
	}
}

AZStd::string UnitComponent::GetName()
{
	if (m_name.empty())
	{
		EBUS_EVENT_ID_RESULT(m_name, GetEntityId(), StaticDataRequestBus, GetValue, "name");
	}
	return m_name;
}

void UnitComponent::SetName(AZStd::string name)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return;
	}

	m_name = name;

	if (m_replicaChunk)
	{
		UnitReplicaChunk* replicaChunk = static_cast<UnitReplicaChunk*>(m_replicaChunk.get());
		replicaChunk->m_name.Set(m_name);
	}
}

UnitTypeId UnitComponent::GetUnitTypeId()
{
	return m_typeId;
}
void UnitComponent::SetUnitTypeId(UnitTypeId unitTypeId)
{
	m_typeId = unitTypeId;
}

TeamId UnitComponent::GetTeamId()
{
	return m_teamId;
}

void UnitComponent::SetTeamId(TeamId team)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return;
	}
	m_teamId = team;

	if (m_replicaChunk)
	{
		UnitReplicaChunk* replicaChunk = static_cast<UnitReplicaChunk*>(m_replicaChunk.get());
		replicaChunk->m_teamId.Set(m_teamId);
	}
}

void spark::UnitComponent::SetQuickBuyItemList(AZStd::vector<ItemTypeId> list)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return;
	}
	m_quickBuyItemList = list;

	if (m_replicaChunk)
	{
		UnitReplicaChunk* replicaChunk = static_cast<UnitReplicaChunk*>(m_replicaChunk.get());
		replicaChunk->m_quickBuyItemList.Set(m_quickBuyItemList);
	}
}

AZStd::vector<ItemTypeId> spark::UnitComponent::GetQuickBuyItemList()
{
	return m_quickBuyItemList;
}

bool UnitComponent::IsVisible()
{
	return m_visible && IsAlive();
}

void UnitComponent::SetVisible(bool visible)
{
	m_visible = visible;

	UpdateVisibility();
}

void UnitComponent::UpdateVisibility(bool force_update)
{
	bool visible = IsVisible();

	//if (!force_update && m_visible_pre!=-1 && visible == (bool)m_visible_pre)return;

	AZStd::vector<AZ::EntityId> children;
	EBUS_EVENT_ID_RESULT(children, GetEntityId(), AZ::TransformBus, GetChildren);

	for (auto c : children)
	{
		EBUS_EVENT_ID(c, LmbrCentral::MeshComponentRequestBus, SetVisibility, visible);
	}

	if ((bool)m_visible_pre != visible)
	{
		EBUS_EVENT(MapVisibilityNotificationBus, OnUnitVisibilityChange, GetEntityId(), (VisibilityEnum)visible);
	}

	m_visible_pre = visible ? 1 : 0;
}

Status UnitComponent::GetStatus()
{
	return m_status;
}

void UnitComponent::SetStatus(Status status)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return;
	}

	if ((status & DEAD) != (m_isAlive ? 0 : DEAD))
	{
		//if the DEAD bit change -> ignore change, they should use kill/respawn
		// toggle DEAD bit
		status ^= DEAD;
	}

	auto old_status = m_status;
	m_status = status;

	if (m_replicaChunk && old_status != m_status)
	{
		UnitReplicaChunk* replicaChunk = static_cast<UnitReplicaChunk*>(m_replicaChunk.get());
		replicaChunk->m_status.Set(m_status);
	}
}

AZStd::string UnitComponent::ToString()
{
	auto str=AZStd::string::format("Unit(name:\"%s\",status=%d,teamId=%d,modifiers:[ ",m_name.c_str(),m_status,m_teamId);

	for (auto m : m_modifiers)
	{
		AZStd::string modStr;
		EBUS_EVENT_ID_RESULT(modStr, m, ModifierRequestBus, ToString);
		str += AZStd::string::format("\n%s,", modStr.c_str());
	}
	str.pop_back();
	str += "])";
	return str;
}

void UnitComponent::GetAllUnits(AZStd::vector<AZ::EntityId> &result)
{
	result.push_back(GetEntityId());
}

void UnitComponent::GetAllUnitsComponents(AZStd::vector<UnitComponent*>& result)
{
	result.push_back(this);
}

void UnitComponent::GetUnits(AZStd::vector<AZ::EntityId>& result, const UnitsFilter & filter)
{
	if(filter(GetEntityId()))result.push_back(GetEntityId());
}

void UnitComponent::GetUnitsEntities(AZStd::vector<AZ::Entity*> &result, const UnitsEntitiesFilter &filter)
{
	if (filter(GetEntity()))result.push_back(GetEntity());
}


AZStd::vector<ModifierId> UnitComponent::GetModifiers()
{
	return m_modifiers;
}
void UnitComponent::SetModifiers(AZStd::vector<ModifierId> modifiers)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return;
	}
	ClearModifiers();

	m_modifiers = modifiers;

	for (auto it : m_modifiers)
	{
		EBUS_EVENT_ID(it, ModifierRequestBus, SetParent, GetEntityId());
		// EBUS_EVENT_ID(GetEntityId(), UnitNotificationBus, OnModifierAttached, it);
	}

	if (m_replicaChunk)
	{
		UnitReplicaChunk* replicaChunk = static_cast<UnitReplicaChunk*>(m_replicaChunk.get());
		replicaChunk->m_modifiers.Set(m_modifiers);
	}
}
void UnitComponent::ClearModifiers()
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return;
	}
	auto modifiers = m_modifiers;

	//sLOG("UnitComponent:ClearModifiers()");
	for (auto it : modifiers)
	{
		EBUS_EVENT_ID(it, ModifierRequestBus, Destroy);
		//EBUS_EVENT_ID(it, ModifierRequestBus, SetParent, AZ::EntityId());
		// EBUS_EVENT_ID(GetEntityId(), UnitNotificationBus, OnModifierDetached, it);
	}

	m_modifiers.clear();

	if (m_replicaChunk)
	{
		UnitReplicaChunk* replicaChunk = static_cast<UnitReplicaChunk*>(m_replicaChunk.get());
		replicaChunk->m_modifiers.Set(m_modifiers);
	}
}
void UnitComponent::AddModifier(ModifierId id)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return;
	}
	auto it=AZStd::find(m_modifiers.begin(), m_modifiers.end(), id);
	if (it == m_modifiers.end())
	{
		m_modifiers.push_back(id);
		EBUS_EVENT_ID(id, ModifierRequestBus, SetParent, GetEntityId());
		// EBUS_EVENT_ID(GetEntityId(), UnitNotificationBus, OnModifierAttached, id);
	}

	if (m_replicaChunk)
	{
		UnitReplicaChunk* replicaChunk = static_cast<UnitReplicaChunk*>(m_replicaChunk.get());
		replicaChunk->m_modifiers.Set(m_modifiers);
	}
}
void UnitComponent::RemoveModifier(ModifierId id)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return;
	}
	auto it=AZStd::find(m_modifiers.begin(), m_modifiers.end(), id);
	if (it != m_modifiers.end())
	{
		m_modifiers.erase(it);
		EBUS_EVENT_ID(id, ModifierRequestBus, SetParent, AZ::EntityId());
		// EBUS_EVENT_ID(GetEntityId(), UnitNotificationBus, OnModifierDetached, id);
	}

	if (m_replicaChunk)
	{
		UnitReplicaChunk* replicaChunk = static_cast<UnitReplicaChunk*>(m_replicaChunk.get());
		replicaChunk->m_modifiers.Set(m_modifiers);
	}
}

ModifierId spark::UnitComponent::FindModifierByTypeId(ModifierTypeId id)
{
	for (auto it : m_modifiers)
	{
		ModifierTypeId t;
		EBUS_EVENT_ID_RESULT(t, it, ModifierRequestBus, GetModifierTypeId);
		if (t == id)
		{
			return it;
		}
	}
	return ModifierId();
}

void UnitComponent::ApplyDispel(Dispel dispel)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return;
	}
	if (dispel.flag & Dispel::DISPEL_FLAG_TOTAL)
	{
		for (auto it : m_modifiers)
		{
			EBUS_EVENT_ID(it, ModifierRequestBus, Destroy);
		}
		m_modifiers.clear();
		return;
	}

	for (auto it : m_modifiers)
	{
		bool dispellable = true;
		EBUS_EVENT_ID_RESULT(dispellable, it, ModifierNotificationBus, IsDispellable, dispel);

		if (dispellable)
		{
			EBUS_EVENT_ID(it, ModifierRequestBus, Destroy);
		}
	}
	// destroy calls call RemoveModifier, so we don't need to update the gridmate values here
}

// orders
UnitOrder UnitComponent::GetCurrentOrder() const 
{
	return m_ordersList.empty() ? UnitOrder::StopOrder() : m_ordersList.front();
}
void UnitComponent::NewOrder(UnitOrder c,bool queue)
{
	if (GetCurrentOrder() == c && !queue) return;

	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		EBUS_EVENT(GameNetSyncRequestBus, SendNewOrder, GetEntityId(), c, queue);
		return;
	}

	EBUS_EVENT_ID(GetEntityId(), UnitNotificationBus, OnNewOrder, c, queue);

	switch(c.type)
	{
	case UnitOrder::UPGRADE: 
		// handled by lua
		return;
	case UnitOrder::BUY:
	{
		ShopId shopId;
		EBUS_EVENT_RESULT(shopId, GameManagerRequestBus, GetNearestShop, GetEntityId());
		if (shopId.IsValid())
		{
			ItemId new_item;
			EBUS_EVENT_ID_RESULT(new_item,shopId, ShopRequestBus, BuyItem, GetEntityId(), c.typeId);
		}
	}return;
	}

	////@TODO handle psuedo-queue abilities (can use without interrupting other things or affecting the queue)
	// example: toggle shield/rot while channeling with something shift-queued afterwards
	if (!queue)m_ordersList.clear();
	
	m_ordersList.push_back(c);

	AZ_Printf(0, "[%s] New order[%s], queuing:%s,\torders queue size:%d", GetEntityId().ToString().c_str(),c.ToString().c_str(),queue?"true":"false", m_ordersList.size());


	if (m_ordersList.size() == 1)ExecuteCurrentOrder();
}

void UnitComponent::OrderDone()
{
	AZ_Printf(0,"Order done!");
	if(!m_ordersList.empty())m_ordersList.pop_front();

	ExecuteCurrentOrder();
}
void UnitComponent::ExecuteCurrentOrder()
{
	auto order = GetCurrentOrder();
	AZ_Printf(0,"Executing order : %s", order.ToString().c_str());
	m_stateMachine.ResizeStack(0);

	switch (order.type) {
	case UnitOrder::MOVE:
		m_targetPostion = order.position;
		m_stateMachine.PushState(S_MOVE);
		break;
	case UnitOrder::ATTACK:
		m_target = order.target;
		m_stateMachine.PushState(S_ATTACK);
		break;
	case UnitOrder::FOLLOW:
		m_target = order.target;
		m_followDistance = order.distance;
		m_stateMachine.PushState(S_FOLLOW);
		break;
	case UnitOrder::CAST:
		m_stateMachine.PushState(S_CAST);
		break;
	default:
		m_stateMachine.PushState(S_IDLE);
	}
}
	


//state

bool UnitComponent::OnStateIdle(SSM& hsm, const SSM::Event& e) {

	switch (e.id) {
	case SSM::EnterEventId:
	case E_UPDATE:
		return true;
	}
	return false;
}

bool UnitComponent::OnStateAttack(SSM& ssm, const SSM::Event& e) {

	switch (e.id) {
	case SSM::EnterEventId:
	{
		EBUS_EVENT_ID(GetEntityId(), UnitAttackRequestBus, Attack, m_target);
		return true;
	}			  
	case SSM::ExitEventId:
	{
		EBUS_EVENT_ID(GetEntityId(), UnitAttackRequestBus, Stop);
		return true;
	}
	case E_ATTACK_INTERRUPTED:
	{
		ssm.PopState();
		return true;
	}
	}
	return false;
}

bool UnitComponent::OnStateMove(SSM& ssm, const SSM::Event& e) {

	switch (e.id) {
	case SSM::EnterEventId:
	{
		EBUS_EVENT_ID(GetEntityId(), UnitNavigationRequestBus, UnitNavigationRequests::MoveToPosition, m_targetPostion);
		return true;
	}
	case E_TRAVERSAL_COMPLETE:
	{
		ssm.PopState();
		return true;
	}
	case SSM::ExitEventId:
	{
		EBUS_EVENT_ID(GetEntityId(), UnitNavigationRequestBus, UnitNavigationRequests::Stop);
		return true;
	}
	}
	return false;
}

bool UnitComponent::OnStateFollow(SSM & ssm, const SSM::Event & e)
{
	switch (e.id) {
	case E_SET_TARGET:
		m_target = e.GetData<AZ::EntityId>();
	case SSM::EnterEventId:
		{
			//EBUS_EVENT_ID(GetEntityId(), UnitNavigationRequestBus, UnitNavigationRequests::Follow, m_target);
			return true;
		}			  
	case E_UPDATE:
	{
		//if ((!m_target.IsValid()) || GetDistance(m_target) < m_followDistance) {
		//	ssm.PopState();
		//}
		return true;
	}
	case E_TRAVERSAL_COMPLETE:
	{
		ssm.PopState();
		return true;
	}
	case SSM::ExitEventId:
	{
		EBUS_EVENT_ID(GetEntityId(), UnitNavigationRequestBus, UnitNavigationRequests::Stop);
		return true;
	}
	}
	return false;
}

bool UnitComponent::OnStateCast(SSM & ssm, const SSM::Event & e)
{
	auto order = GetCurrentOrder();


	switch (e.id) {
	case SSM::EnterEventId:
	{
		EBUS_EVENT_ID(GetEntityId(), UnitAbilityRequestBus, CastAbility, order.castContext);
		return true;
	}
	case E_CAST_FINISHED:
	{
		ssm.PopState();
		return true;
	}
	case SSM::ExitEventId:
	{
		EBUS_EVENT_ID(GetEntityId(), UnitAbilityRequestBus, InterruptCasting);
		return true;
	}
	}
	return false;
}

void UnitComponent::OnLanguageChanged(const AZStd::string &language)
{
	// const rapidjson::Value *pinfo = nullptr;
	// EBUS_EVENT_ID_RESULT(pinfo, GetEntityId(), StaticDataRequestBus, GetJson);

	// if (pinfo && pinfo->IsObject()) {
	// 	auto &info = *pinfo;
	// 	if (info.HasMember("name"))SetName(GetLocalizatedString(info["name"]));
	// }
}

void UnitComponent::Destroy()
{
	// destroy next tick so that it can be called syncronously from an event handler and still allow all other handlers to execute with a valid unit
	m_isDestroying = true;
}
