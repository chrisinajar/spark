#include "spark_precompiled.h"

#include "AbilityComponent.h"

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Math/Transform.h>

#include <AzCore/Script/ScriptContext.h>
#include <AzCore/Script/ScriptSystemBus.h>
#include <AzCore/Script/ScriptProperty.h>

#include <AzCore/Asset/AssetManagerBus.h>
#include <AzCore/std/string/conversions.h>
#include <AzFramework/StringFunc/StringFunc.h>
#include <AzFramework/Network/NetworkContext.h>
#include <AzFramework/Network/NetBindingHandlerBus.h>
#include <AzFramework/Network/NetBindingComponent.h>
#include <GridMate/Replica/ReplicaFunctions.h>
#include <GridMate/Replica/ReplicaChunk.h>
#include <GridMate/Replica/DataSet.h>
#include <GridMate/Serialize/ContainerMarshal.h>

#include "Busses/VariableBus.h"
#include "Busses/StaticDataBus.h"
#include "Busses/CooldownBus.h"
#include "Busses/UnitAbilityBus.h"

#include "Utils/StringUtils.h"
#include "Utils/JsonUtils.h"
#include "Utils/Log.h"
#include "Utils/Marshaler.h"
#include "Utils/Filter.h"

//extern "C" {
//#   include <Lua/lualib.h>
//#   include <Lua/lauxlib.h>
//}

using namespace spark;

CastingBehavior::CastingBehavior(int flags)
	:m_flags(flags)
{
}

int CastingBehavior::GetFlags() const
{
	return m_flags;
}

bool CastingBehavior::Contains(int flag) const
{
	return m_flags & flag;
}

void CastingBehaviorScriptConstructor(CastingBehavior* self, AZ::ScriptDataContext& dc)
{
	int flags = 0;
	int flag = 0;
	for (int i = 0; i < dc.GetNumArguments(); i++)
	{
		if (dc.IsNumber(i))
		{
			dc.ReadArg(i, flag);
			flags |= flag;
		}
		else
		{
			dc.GetScriptContext()->Error(AZ::ScriptContext::ErrorType::Error, true, "Invalid argument in position %d passed to CastingBehavior(...).",i);
		}

	}
	
	new(self) CastingBehavior(flags);
	AZ_Printf(0,"Casting behavior created with flags : %d", self->GetFlags());
}

void CastingBehavior::Reflect(AZ::ReflectContext* reflection)
{
	if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
	{
		serializationContext->Class<CastingBehavior>()
			->Version(1)
			->Field("flags", &CastingBehavior::m_flags);

		if (auto editContext = serializationContext->GetEditContext())
		{
			editContext->Class<CastingBehavior>("CastingBehavior", "Using to describe an ability behavior")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
				->DataElement(nullptr, &CastingBehavior::m_flags, "flags", "");
		}


	}
	if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
	{
		behaviorContext->Class<CastingBehavior>("CastingBehavior")
			->Attribute(AZ::Script::Attributes::Storage, AZ::Script::Attributes::StorageType::Value)
			->Attribute(AZ::Script::Attributes::ConstructorOverride, &CastingBehaviorScriptConstructor)

			->Enum<(int)CastingBehavior::NONE>("NONE")

			->Enum<(int)CastingBehavior::TargetingBehavior::PASSIVE>("PASSIVE")
			->Enum<(int)CastingBehavior::TargetingBehavior::IMMEDIATE>("IMMEDIATE")
			->Enum<(int)CastingBehavior::TargetingBehavior::POINT_TARGET>("POINT_TARGET")
			->Enum<(int)CastingBehavior::TargetingBehavior::UNIT_TARGET>("UNIT_TARGET")
			->Enum<(int)CastingBehavior::TargetingBehavior::DIRECTION_TARGET>("DIRECTION_TARGET")
			->Enum<(int)CastingBehavior::TargetingBehavior::VECTOR_TARGET>("VECTOR_TARGET")

			->Enum<(int)CastingBehavior::TargetTeam::FRIENDLY>("TEAM_FRIENDLY")
			->Enum<(int)CastingBehavior::TargetTeam::ENEMY>("TEAM_ENEMY")
			->Enum<(int)CastingBehavior::TargetTeam::BOTH>("TEAM_BOTH")

			->Enum<(int)CastingBehavior::TargetType::HERO>("HERO")
			->Enum<(int)CastingBehavior::TargetType::CREEP>("CREEP")
			->Enum<(int)CastingBehavior::TargetType::BUILDING>("BUILDING")
			->Enum<(int)CastingBehavior::TargetType::MECHANICAL>("MECHANICAL")
			->Enum<(int)CastingBehavior::TargetType::COURIER>("COURIER")
			->Enum<(int)CastingBehavior::TargetType::TREE>("TREE")
			->Enum<(int)CastingBehavior::TargetType::RUNE>("RUNE")
			->Enum<(int)CastingBehavior::TargetType::NORMAL_UNIT>("NORMAL_UNIT")
			->Enum<(int)CastingBehavior::TargetType::UNIT>("UNIT")
			->Enum<(int)CastingBehavior::TargetType::ALL>("ALL")

			->Method("Contains", &CastingBehavior::Contains);
	}
}

void CastContext::Reflect(AZ::ReflectContext* reflection)
{
	if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
	{
		serializationContext->Class<CastContext>()
			->Version(1)
			->Field("IsItem", &CastContext::isItem)
			->Field("Caster", &CastContext::caster)
			->Field("Target", &CastContext::target)
			->Field("CursorPosition", &CastContext::cursorPosition)
			->Field("Ability", &CastContext::ability)
			->Field("BehaviorUsed", &CastContext::behaviorUsed)
			;

	}
	if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
	{
		behaviorContext->Class<CastContext>("CastContext")
			->Attribute(AZ::Script::Attributes::Storage, AZ::Script::Attributes::StorageType::Value)
			->Property("IsItem",  BehaviorValueProperty(&CastContext::isItem))
			->Property("Caster",  BehaviorValueProperty(&CastContext::caster))
			->Property("Target",  BehaviorValueProperty(&CastContext::target))
			->Property("CursorPosition",  BehaviorValueProperty(&CastContext::cursorPosition))
			->Property("Ability",  BehaviorValueProperty(&CastContext::ability))
			->Property("BehaviorUsed",  BehaviorValueProperty(&CastContext::behaviorUsed))
			;
	}
}


/**
*	Interface to lua scripts
*/
class CooldownNotificationBusHandler
	: public CooldownNotificationBus::Handler
	, public AZ::BehaviorEBusHandler
{
public:
	AZ_EBUS_BEHAVIOR_BINDER(CooldownNotificationBusHandler, "{73DCD507-543D-438F-9508-A1C604CA305A}", AZ::SystemAllocator, OnCooldownStarted,OnCooldownFinished);

	void OnCooldownStarted(float time) override
	{
		Call(FN_OnCooldownStarted,time);
	}

	void OnCooldownFinished() override
	{
		Call(FN_OnCooldownFinished);
	}
};

class AbilityReplicaChunk : public GridMate::ReplicaChunkBase
{
public:
	AZ_CLASS_ALLOCATOR(AbilityReplicaChunk, AZ::SystemAllocator, 0);

	static const char* GetChunkName() { return "AbilityReplicaChunk"; }

	   AbilityReplicaChunk()
		: m_abilityName("AbilityName")
		, m_abilityDescription("AbilityDescription")
		, m_typeId("TypeId")
		, m_iconTexture("IconTexture")
		, m_castingBehavior("CastingBehavior")
		, m_costs("Costs")
		, m_level("Level")
		, m_caster("Caster")
	{
	}

	bool IsReplicaMigratable()
	{
		return true;
	}

	GridMate::DataSet<AZStd::string>::BindInterface<AbilityComponent, &AbilityComponent::OnNewAbilityName> m_abilityName;
	GridMate::DataSet<AZStd::string>::BindInterface<AbilityComponent, &AbilityComponent::OnNewAbilityDescription> m_abilityDescription;
	GridMate::DataSet<AbilityTypeId>::BindInterface<AbilityComponent, &AbilityComponent::OnNewTypeId> m_typeId;
	GridMate::DataSet<AZStd::string>::BindInterface<AbilityComponent, &AbilityComponent::OnNewIconTexture> m_iconTexture;
	GridMate::DataSet<CastingBehavior>::BindInterface<AbilityComponent, &AbilityComponent::OnNewCastingBehavior> m_castingBehavior;
	GridMate::DataSet<Costs, GridMate::ContainerMarshaler<Costs>>::BindInterface<AbilityComponent, &AbilityComponent::OnNewCosts> m_costs;
	GridMate::DataSet<int>::BindInterface<AbilityComponent, &AbilityComponent::OnNewLevel> m_level;
	GridMate::DataSet<UnitId>::BindInterface<AbilityComponent, &AbilityComponent::OnNewCaster> m_caster;
};

void AbilityComponent::Reflect(AZ::ReflectContext * reflection)
{
	if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
	{
		serializationContext->Class<AbilityComponent, AzFramework::NetBindable, AZ::Component>()
			->Version(2)
			->Field("Name", &AbilityComponent::m_abilityName)
			->Field("Description", &AbilityComponent::m_abilityDescription);
			//->Field("Ability Script", &AbilityComponent::m_script);

		if (auto editContext = serializationContext->GetEditContext())
		{
			editContext->Class<AbilityComponent>("AbilityComponent", "to attach to an ability entity")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
				->Attribute(AZ::Edit::Attributes::Category, "spark")
				->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"))
				->DataElement(nullptr, &AbilityComponent::m_abilityName, "Ability Name", "")
				->DataElement(nullptr, &AbilityComponent::m_abilityDescription, "Ability Description", "");
				//->DataElement(nullptr, &AbilityComponent::m_script, "Ability Script", "Lua script describing the ability");
		}
	}
	CastingBehavior::Reflect(reflection);
	CastContext::Reflect(reflection);
	Amount::Reflect(reflection);


	if (auto netContext = azrtti_cast<AzFramework::NetworkContext*>(reflection))
	{
		netContext->Class<AbilityComponent>()
			->Chunk<AbilityReplicaChunk>()
			->Field("AbilityName", &AbilityReplicaChunk::m_abilityName)
			->Field("AbilityDescription", &AbilityReplicaChunk::m_abilityDescription)
			->Field("TypeId", &AbilityReplicaChunk::m_typeId)
			->Field("IconTexture", &AbilityReplicaChunk::m_iconTexture)
			->Field("CastingBehavior", &AbilityReplicaChunk::m_castingBehavior)
			->Field("Costs", &AbilityReplicaChunk::m_costs)
			->Field("Level", &AbilityReplicaChunk::m_level)
			->Field("Caster", &AbilityReplicaChunk::m_caster)
			;
	}

	if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
	{
		
		behaviorContext->EBus<AbilityRequestBus>("AbilityRequestBus")
			->Event("GetAbilityTypeId", &AbilityRequestBus::Events::GetAbilityTypeId)
			->Event("SetAbilityTypeId", &AbilityRequestBus::Events::SetAbilityTypeId)
			->Event("GetAbilityName", &AbilityRequestBus::Events::GetAbilityName)
			->Event("SetAbilityName", &AbilityRequestBus::Events::SetAbilityName)
			->Event("GetAbilityDescription", &AbilityRequestBus::Events::GetAbilityDescription)
			->Event("SetAbilityDescription", &AbilityRequestBus::Events::SetAbilityDescription)
			->Event("GetCastingBehavior", &AbilityRequestBus::Events::GetCastingBehavior)
			->Event("SetCastingBehavior", &AbilityRequestBus::Events::SetCastingBehavior)
			->Event("GetIconTexture", &AbilityRequestBus::Events::GetIconTexture)
			->Event("SetIconTexture", &AbilityRequestBus::Events::SetIconTexture)
			->Event("GetCosts", &AbilityRequestBus::Events::GetCosts)
			->Event("SetCosts", &AbilityRequestBus::Events::SetCosts)
			->Event("SetCooldown", &AbilityRequestBus::Events::SetCooldown)
			->Event("GetLevel", &AbilityRequestBus::Events::GetLevel)
			->Event("SetLevel", &AbilityRequestBus::Events::SetLevel)
			->Event("GetCaster", &AbilityRequestBus::Events::GetCaster)
			;

		behaviorContext->EBus<AbilityNotificationBus>("AbilityNotificationBus")
			->Handler<AbilityNotificationBusHandler>()
			->Event("OnAttached", &AbilityNotificationBus::Events::OnAttached)
			->Event("OnDetached", &AbilityNotificationBus::Events::OnDetached)
			->Event("OnSpellStart", &AbilityNotificationBus::Events::OnSpellStart)
			->Event("OnAbilityPhaseStart", &AbilityNotificationBus::Events::OnAbilityPhaseStart)
			->Event("OnAbilityPhaseInterrupted", &AbilityNotificationBus::Events::OnAbilityPhaseInterrupted)
			->Event("OnChannelFinish", &AbilityNotificationBus::Events::OnChannelFinish)
			->Event("OnUpgrade", &AbilityNotificationBus::Events::OnUpgrade);
		
		behaviorContext->EBus<CooldownNotificationBus>("CooldownNotificationBus")
			->Handler<CooldownNotificationBusHandler>()
			->Event("OnCooldownStarted", &CooldownNotificationBus::Events::OnCooldownStarted)
			->Event("OnCooldownFinished", &CooldownNotificationBus::Events::OnCooldownFinished);


		AZ_Printf(0, "added AbilityBus to behaviorContext");
	}
}

GridMate::ReplicaChunkPtr AbilityComponent::GetNetworkBinding ()
{
	auto replicaChunk = GridMate::CreateReplicaChunk<AbilityReplicaChunk>();
	replicaChunk->SetHandler(this);
	m_replicaChunk = replicaChunk;

	if (AbilityReplicaChunk* abilityChunk = static_cast<AbilityReplicaChunk*>(m_replicaChunk.get()))
	{
		abilityChunk->m_abilityName.Set(m_abilityName);
		abilityChunk->m_abilityDescription.Set(m_abilityDescription);
		abilityChunk->m_typeId.Set(m_typeId);
		abilityChunk->m_iconTexture.Set(m_iconTexture);
		abilityChunk->m_castingBehavior.Set(m_castingBehavior);
		abilityChunk->m_costs.Set(m_costs);
		abilityChunk->m_level.Set(m_level);
		abilityChunk->m_caster.Set(m_caster);
	}

	return m_replicaChunk;
}

void AbilityComponent::SetNetworkBinding (GridMate::ReplicaChunkPtr chunk)
{
	chunk->SetHandler(this);
	m_replicaChunk = chunk;

	AbilityReplicaChunk* abilityChunk = static_cast<AbilityReplicaChunk*>(m_replicaChunk.get());
	m_abilityName = abilityChunk->m_abilityName.Get();
	m_abilityDescription = abilityChunk->m_abilityDescription.Get();
	m_typeId = abilityChunk->m_typeId.Get();
	m_iconTexture = abilityChunk->m_iconTexture.Get();
	m_castingBehavior = abilityChunk->m_castingBehavior.Get();
	m_costs = abilityChunk->m_costs.Get();
	m_level = abilityChunk->m_level.Get();
	m_caster = abilityChunk->m_caster.Get();

	AZ_Printf(0, "Replica ability chunk active now! - %s", m_typeId.c_str());
}

void AbilityComponent::UnbindFromNetwork ()
{
	if (m_replicaChunk)
	{
		m_replicaChunk->SetHandler(nullptr);
		m_replicaChunk = nullptr;
	}
}

void AbilityComponent::OnNewAbilityName (const AZStd::string& abilityName, const GridMate::TimeContext& tc)
{
	m_abilityName = abilityName;
}

void AbilityComponent::OnNewAbilityDescription (const AZStd::string& abilityDescription, const GridMate::TimeContext& tc)
{
	m_abilityDescription = abilityDescription;
}

void AbilityComponent::OnNewTypeId (const AbilityTypeId& typeId, const GridMate::TimeContext& tc)
{
	m_typeId = typeId;
}

void AbilityComponent::OnNewIconTexture (const AZStd::string& iconTexture, const GridMate::TimeContext& tc)
{
	m_iconTexture = iconTexture;
}

void AbilityComponent::OnNewCastingBehavior (const CastingBehavior& castingBehavior, const GridMate::TimeContext& tc)
{
	m_castingBehavior = castingBehavior;
}

void AbilityComponent::OnNewCosts (const Costs& costs, const GridMate::TimeContext& tc)
{
	m_costs = costs;
}

void AbilityComponent::OnNewLevel (const int& level, const GridMate::TimeContext& tc)
{
	m_level = level;
}

void AbilityComponent::OnNewCaster (const UnitId& caster, const GridMate::TimeContext& tc)
{
	m_caster = caster;
}

#define REPLICATE_VALUE(memberVal) \
if (m_replicaChunk) { \
	AbilityReplicaChunk* abilityChunk = static_cast<AbilityReplicaChunk*>(m_replicaChunk.get()); \
	abilityChunk-> memberVal .Set(memberVal); \
	AZ_Printf(0, "Sending an update about %s", #memberVal); \
}

#define UPDATE_VALUE(memberVal, newVal) \
if (memberVal != newVal) {\
	memberVal = newVal; \
	REPLICATE_VALUE(memberVal) \
}

#define UPDATE_STR_VALUE(memberVal, newVal) \
if (memberVal.compare(newVal) != 0) {\
	memberVal = newVal; \
	REPLICATE_VALUE(memberVal) \
}

void AbilityComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
{
	provided.push_back(AZ_CRC("AbilityService"));
}

void AbilityComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
{
	incompatible.push_back(AZ_CRC("AbilityService"));
}

void AbilityComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
{
    required.push_back(AZ_CRC("VariableHolderService"));
}

void AbilityComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
{
	dependent.push_back(AZ_CRC("StaticDataService"));
}

void AbilityComponent::Init()
{
}

void AbilityComponent::Activate()
{
	AbilityRequestBus::Handler::BusConnect(GetEntityId());
	AbilityNotificationBus::Handler::BusConnect(GetEntityId());
	LocalizationNotificationBus::Handler::BusConnect();

	SetLevel(m_level);
}

void AbilityComponent::Deactivate()
{
	if (m_caster.IsValid())
	{
		EBUS_EVENT_ID(m_caster, UnitAbilityRequestBus, DetachAbility, GetEntityId());
	}

	LocalizationNotificationBus::Handler::BusDisconnect();
	AbilityNotificationBus::Handler::BusDisconnect(GetEntityId());
	AbilityRequestBus::Handler::BusDisconnect();
}


CastingBehavior AbilityComponent::GetCastingBehavior() {
	return m_castingBehavior;
}

void AbilityComponent::SetCastingBehavior(CastingBehavior behavior)
{
	UPDATE_VALUE(m_castingBehavior, behavior);
}

void AbilityComponent::SetCosts(Costs costs)
{
	UPDATE_VALUE(m_costs, costs);
}
Costs AbilityComponent::GetCosts()
{
	return m_costs;
}

void AbilityComponent::SetCooldown(float cooldown)
{
	EBUS_EVENT(VariableManagerRequestBus, RegisterVariableAndInizialize, VariableId(GetEntityId(), "cooldown_max"),cooldown);
	
	Value timer=0.0f;
	EBUS_EVENT_RESULT(timer, VariableManagerRequestBus, GetValue, VariableId(GetEntityId(), "cooldown_timer"));

	//check if not already ticking
	if (timer != 0.0f)
	{
		EBUS_EVENT(VariableManagerRequestBus, RegisterVariableAndInizialize, VariableId(GetEntityId(), "cooldown_current"), cooldown);
	}
	
	EBUS_EVENT(VariableManagerRequestBus, RegisterVariable, VariableId(GetEntityId(), "cooldown_timer"));
}


int AbilityComponent::GetLevel()
{
	return m_level;
}

void spark::AbilityComponent::SetAbilityTypeId(AbilityTypeId type)
{
	UPDATE_STR_VALUE(m_typeId, type);
}

AbilityTypeId spark::AbilityComponent::GetAbilityTypeId()
{
	return m_typeId;
}

AZStd::string AbilityComponent::GetAbilityName()
{
	return m_abilityName;
}

AZStd::string AbilityComponent::GetAbilityDescription()
{
	return m_abilityDescription;
}

void AbilityComponent::SetAbilityName(AZStd::string name)
{
	UPDATE_STR_VALUE(m_abilityName, name);
}

void AbilityComponent::SetAbilityDescription(AZStd::string description)
{
	UPDATE_STR_VALUE(m_abilityDescription, description);
}

void AbilityComponent::SetIconTexture(AZStd::string filename)
{
	UPDATE_STR_VALUE(m_iconTexture, filename);
}

AZStd::string AbilityComponent::GetIconTexture()
{
	return m_iconTexture;
}

float GetKvFloatFromJson(const rapidjson::Value &json, int level=1)
{
	if (json.IsNumber())return json.GetDouble();

	if (json.IsString())
	{
		return AZStd::stof(StringUtils::GetKvValue(JsonUtils::ToString(json), level));;
	}

	return 0.0f;
}

Costs GetCostsFromJson(const rapidjson::Value &json, int level=1)
{
	Costs costs;
	if (json.HasMember("costs"))//initialize variables
	{
		AZ_Printf(0,"GetCostsFromJson costs exist");

		auto &costs_json = json["costs"];
		for (auto c = costs_json.MemberBegin(); c != costs_json.MemberEnd(); ++c)
		{
			Amount cost;
			cost.variable = c->name.GetString();

			if (c->value.IsNumber())
			{
				cost.amount = c->value.GetDouble();
			}
			else if (c->value.IsString())
			{
				auto s = JsonUtils::ToString(c->value); // get in the format "123" or "1 2 3 4"
				s = StringUtils::GetKvValue(s, level);	// get the format "123"
				cost.amount = AZStd::stof(s);			// convert to float
			}
			else
			{
				sWARNING("the cost "+ cost.variable +" is not well formatted! -> ignored");
				continue;
			}

			AZ_Printf(0,"GetCostsFromJson added cost   %s=%f",cost.variable.c_str(),cost.amount);
			costs.push_back(cost);
		}
	}
	return costs;
}

void AbilityComponent::SetLevel(int level)
{
	if (SPARK_FILTER_EVENT_PREVENT(AbilitiesNotificationBus, OnUpgradeFilter, GetEntityId())
		|| SPARK_FILTER_EVENT_ID_PREVENT(GetEntityId(), AbilityNotificationBus, OnUpgradeFilter))
	{
		return;
	}

	UPDATE_VALUE(m_level, level);

	const rapidjson::Value *info = nullptr;
	EBUS_EVENT_ID_RESULT(info, GetEntityId(), StaticDataRequestBus, GetJson);

	AZ_Printf(0,"setting level %d", m_level);

	if (info && info->IsObject()) {

		//update costs
		if (info->HasMember("costs"))
		{
			AZ_Printf(0,"setting costs");
			Costs costs = GetCostsFromJson(*info,m_level);
			SetCosts(costs);
		}

		//update cooldown
		if (info->HasMember("cooldown"))
		{
			float cooldown = GetKvFloatFromJson((*info)["cooldown"],m_level);
			SetCooldown(cooldown);
		}
	}

	//notify listeners
	EBUS_EVENT_ID(GetEntityId(),AbilityNotificationBus, OnUpgrade);
	EBUS_EVENT(AbilitiesNotificationBus, OnUpgrade, GetEntityId());
}

void AbilityComponent::OnAttached(UnitId unit)
{
	UPDATE_VALUE(m_caster, unit);
}

void AbilityComponent::OnDetached(UnitId unit)
{
	AZ::EntityId entityId;
	UPDATE_VALUE(m_caster, entityId);
}

UnitId AbilityComponent::GetCaster()
{
	return m_caster;
}

void AbilityComponent::OnLanguageChanged(const AZStd::string &language)
{
	// const rapidjson::Value *pinfo = nullptr;
	// EBUS_EVENT_ID_RESULT(pinfo, GetEntityId(), StaticDataRequestBus, GetJson);

	// if (pinfo && pinfo->IsObject()) {
	// 	auto &info = *pinfo;
	// 	if (info.HasMember("name"))SetAbilityName(info["name"].GetString());
	// 	if (info.HasMember("description"))SetAbilityDescription(info["description"].GetString());
	// }
}
