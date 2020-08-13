

#include "spark_precompiled.h"

#include "VariableManagerComponent.h"

#include <AzFramework/Network/NetworkContext.h>
#include <AzFramework/Network/NetBindingHandlerBus.h>
#include <GridMate/Replica/ReplicaFunctions.h>

#include <GridMate/Replica/ReplicaChunk.h>
#include <GridMate/Replica/DataSet.h>
#include <GridMate/Serialize/CompressionMarshal.h>
#include <GridMate/Serialize/ContainerMarshal.h>

#include <ISystem.h>
#include <INetwork.h>

#include "Utils/Marshaler.h"
#include "Utils/Log.h"
#include "Utils/Filter.h"
#include "Busses/CooldownBus.h"
#include "Busses/VariableBus.h"


using namespace AzFramework;
using namespace GridMate;
using namespace spark;


//this is just AZ_EBUS_BEHAVIOR_BINDER without the constructor, used for debugging purposes
#define CUSTOM_EBUS_BEHAVIOR_BINDER(_Handler,_Uuid,_Allocator,...)\
	AZ_CLASS_ALLOCATOR(_Handler,_Allocator,0)\
    AZ_RTTI(_Handler,_Uuid,AZ::BehaviorEBusHandler)\
    typedef _Handler ThisType;\
    enum {\
        AZ_SEQ_FOR_EACH(AZ_BEHAVIOR_EBUS_FUNC_ENUM, AZ_EBUS_SEQ(__VA_ARGS__))\
        FN_MAX\
    };\
    int GetFunctionIndex(const char* functionName) const override {\
        AZ_SEQ_FOR_EACH(AZ_BEHAVIOR_EBUS_FUNC_INDEX, AZ_EBUS_SEQ(__VA_ARGS__))\
        return -1;\
    }\
    void Disconnect() override {\
        BusDisconnect();\
    }\
    bool Connect(AZ::BehaviorValueParameter* id = nullptr) override {\
        return AZ::Internal::EBusConnector<_Handler>::Connect(this, id);\
    }


	/**
	*	Interface to lua scripts describing variables
	*/
class VariableRequestBusHandler
	: public VariableRequestBus::Handler
	, public AZ::BehaviorEBusHandler
{
public:
	AZ_EBUS_BEHAVIOR_BINDER(VariableRequestBusHandler, "{37335E0B-8505-4E1F-9CB4-EC66792D954F}", AZ::SystemAllocator, GetValue, OnSet, GetMinValue, GetMaxValue);

	//VariableRequestBusHandler()
	//{
	//	m_events.resize(FN_MAX);
	//	SetEvent(&ThisType::GetValue, "GetValue");
	//	SetEvent(&ThisType::OnSet, "OnSet");
	//	SetEvent(&ThisType::GetMinValue, "GetMinValue");
	//	SetEvent(&ThisType::GetMaxValue, "GetMaxValue");

	//	//AZ_Printf(0, "VariableRequestBusHandler constructed");
	//}
	//~VariableRequestBusHandler()
	//{
	//	//AZ_Assert(false, "here");
	//	//AZ_Printf(0, "VariableRequestBusHandler destructed");
	//}

	Value GetValue(float deltaTime, AZ::ScriptTimePoint time);

	Value OnSet(Value current, Value proposed);

	Value GetMinValue();
	Value GetMaxValue();
};


/**
*	Interface to lua scripts describing (flat)bonus modifiers
*/
class VariableBonusModifierBusHandler
	: public VariableBonusModifierBus::Handler
	, public AZ::BehaviorEBusHandler
{
public:
	AZ_EBUS_BEHAVIOR_BINDER(VariableBonusModifierBusHandler, "{10A5979E-1A04-4393-91DF-95C6C4221F14}", AZ::SystemAllocator, GetModifierBonus);

	Value GetModifierBonus(AZStd::string id) override;
};



class VariableNotificationBusHandler
	: public VariableNotificationBus::Handler
	, public AZ::BehaviorEBusHandler
{
public:
	AZ_EBUS_BEHAVIOR_BINDER(VariableNotificationBusHandler, "{7C5F455D-E761-4DD3-A19B-E17222588385}", AZ::SystemAllocator, OnSetValueFilter, OnSetValue);

	void OnSetValue(VariableId id, Value value) override
	{
		Call(FN_OnSetValue, id, value);
	}
	void OnSetValueFilter(VariableId id, Value value) override
	{
		Call(FN_OnSetValueFilter, id, value);
	}
};

class VariableHolderNotificationBusHandler
	: public VariableHolderNotificationBus::Handler
	, public AZ::BehaviorEBusHandler
{
public:
	AZ_EBUS_BEHAVIOR_BINDER(VariableHolderNotificationBusHandler, "{9147BF96-F474-48A7-A975-10A57624B2E4}", AZ::SystemAllocator, OnSetValueFilter, OnSetValue);

	void OnSetValue(VariableId id, Value value) override
	{
		Call(FN_OnSetValue, id, value);
	}
	void OnSetValueFilter(VariableId id, Value value) override
	{
		Call(FN_OnSetValueFilter, id, value);
	}
};

class VariableManagerNotificationBusHandler
	: public VariableManagerNotificationBus::Handler
	, public AZ::BehaviorEBusHandler
{
public:
	AZ_EBUS_BEHAVIOR_BINDER(VariableManagerNotificationBusHandler, "{6EFDE826-C795-4118-BD4B-3D8A1E4A882A}", AZ::SystemAllocator, OnSetValueFilter, OnSetValue);

	void OnSetValue(VariableId id, Value value) override
	{
		Call(FN_OnSetValue, id, value);
	}
	void OnSetValueFilter(VariableId id, Value value) override
	{
		Call(FN_OnSetValueFilter, id, value);
	}
};


void VariableIdScriptConstructor(VariableId* self, AZ::ScriptDataContext& dc)
{
	if (dc.GetNumArguments() == 0)
	{
		*self = VariableId();
		return;
	} 
	else if (dc.GetNumArguments() == 2)
	{
		if (dc.IsClass<AZ::EntityId>(0) && dc.IsString(1))
		{
			AZ::EntityId entityId;
			const char* variableId = nullptr;
			dc.ReadArg(0, entityId);
			dc.ReadArg(1, variableId);

			new(self) VariableId(entityId, variableId);

			return;
		}
	}

	dc.GetScriptContext()->Error(AZ::ScriptContext::ErrorType::Error, true, "Invalid arguments passed to VariableId().");
	new(self) VariableId();
}

void VariableId::Reflect(AZ::ReflectContext* reflection)
{
	if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
	{
		serializationContext->Class<VariableId>()
			->Version(1)
			->Field("entityId", &VariableId::m_entityId)
			->Field("variableId", &VariableId::m_variableId)
			; 

		if (auto editContext = serializationContext->GetEditContext())
		{
			editContext->Class<VariableId>("VariableId", "identify a unit's variable")
				->DataElement(nullptr, &VariableId::m_entityId, "entityId", "")
				->DataElement(nullptr, &VariableId::m_variableId, "variableId", "");
		}
	}
	   
	if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
	{
		behaviorContext->Class<VariableId>("VariableId")
			->Attribute(AZ::Script::Attributes::Storage, AZ::Script::Attributes::StorageType::RuntimeOwn)
			->Attribute(AZ::Script::Attributes::ConstructorOverride, &VariableIdScriptConstructor)
			->Property("entityId", BehaviorValueProperty(&VariableId::m_entityId))
			->Property("variableId", BehaviorValueProperty(&VariableId::m_variableId))
			->Method("ToString", &VariableId::ToString)
			;
	}
}

class VariableManagerReplicaChunk
	: public GridMate::ReplicaChunkBase
{
public:
	GM_CLASS_ALLOCATOR(VariableManagerReplicaChunk);

	VariableManagerReplicaChunk()
		// : m_variables("Variables")
		// , m_variablesByEntity("VariablesByEntityId")
	{
	}

	bool IsReplicaMigratable() override
	{
		return true;
	}

	static const char* GetChunkName()
	{
		return "VariableManagerReplicaChunk";
	}
	// GridMate::Rpc<RpcArg<VariableId>, RpcArg<Value>>::BindInterface<VariableManagerComponent, &VariableManagerComponent::OnNetSetValue> m_onSetValue = { "SetValue" };

	// GridMate::DataSet<VariableManagerComponent::VariableMap,
	// 	GridMate::MapContainerMarshaler<VariableManagerComponent::VariableMap>>
	// 	::BindInterface<VariableManagerComponent, &VariableManagerComponent::OnNewVariables> m_variables;

	// GridMate::DataSet<VariableManagerComponent::VariableIdMap, 
	// 	GridMate::MapContainerMarshaler<VariableManagerComponent::VariableIdMap, GridMate::Marshaler<AZ::EntityId>, GridMate::ContainerMarshaler<AZStd::vector<AZStd::string>>>>
	// 	::BindInterface<VariableManagerComponent, &VariableManagerComponent::OnNewVariablesByEntity> m_variablesByEntity;
};

void VariableManagerComponent::Reflect(AZ::ReflectContext* reflection)
{
	if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
	{
		serializationContext->Class<VariableManagerComponent, AZ::Component, AzFramework::NetBindable>()
			->Version(2);

		if (auto editContext = serializationContext->GetEditContext())
		{
			editContext->Class<VariableManagerComponent>("VariableManagerComponent", "Manage unit's variables(attributes,hp,mana,gold,etc.)")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
				->Attribute(AZ::Edit::Attributes::Category, "spark")
				->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"));
		}
	}

	if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
	{
		VariableId::Reflect(reflection);
		FilterResult::Reflect(reflection);

		behaviorContext->EBus<VariableManagerRequestBus>("VariableManagerRequestBus")
			->Attribute(AZ::Script::Attributes::Category, "Unit")
			->Event("RegisterVariable", &VariableManagerRequestBus::Events::RegisterVariable)
			->Event("RegisterVariableAndInizialize", &VariableManagerRequestBus::Events::RegisterVariableAndInizialize)
			->Event("RegisterDependentVariable", &VariableManagerRequestBus::Events::RegisterDependentVariable)
			->Event("VariableExists", &VariableManagerRequestBus::Events::VariableExists)
			->Event("SetValue", &VariableManagerRequestBus::Events::SetValue)
			->Event("GetValue", &VariableManagerRequestBus::Events::GetValue)
			->Event("GetBaseValue", &VariableManagerRequestBus::Events::GetBaseValue)
			->Event("GetBonusValue", &VariableManagerRequestBus::Events::GetBonusValue)
			->Event("ToString", &VariableManagerRequestBus::Events::ToString);

		behaviorContext->EBus<VariableRequestBus>("VariableRequestBus")
			->Handler<VariableRequestBusHandler>()
			->Event("GetValue", &VariableRequestBus::Events::GetValue)
			->Event("GetMinValue", &VariableRequestBus::Events::GetMinValue)
			->Event("GetMaxValue", &VariableRequestBus::Events::GetMaxValue);

		behaviorContext->EBus<VariableBonusModifierBus>("VariableBonusModifierBus")
			->Handler<VariableBonusModifierBusHandler>()
			->Event("GetModifierBonus", &VariableBonusModifierBus::Events::GetModifierBonus);

		behaviorContext->EBus<VariableNotificationBus>("VariableNotificationBus")
			->Handler<VariableNotificationBusHandler>();

		behaviorContext->EBus<VariableHolderNotificationBus>("VariableHolderNotificationBus")
			->Handler<VariableHolderNotificationBusHandler>();

		behaviorContext->EBus<VariableManagerNotificationBus>("VariableManagerNotificationBus")
			->Handler<VariableManagerNotificationBusHandler>();

		behaviorContext->EBus<VariableHolderRequestBus>("VariableHolderRequestBus")
			->Attribute(AZ::Script::Attributes::Category, "Unit")
			->Event("RegisterVariable", &VariableHolderRequestBus::Events::RegisterVariable)
			->Event("RegisterVariableAndInizialize", &VariableHolderRequestBus::Events::RegisterVariableAndInizialize)
			->Event("RegisterDependentVariable", &VariableHolderRequestBus::Events::RegisterDependentVariable)
			->Event("VariableExists", &VariableHolderRequestBus::Events::VariableExists)
			->Event("SetValue", &VariableHolderRequestBus::Events::SetValue)
			->Event("GetValue", &VariableHolderRequestBus::Events::GetValue)
			->Event("GetBaseValue", &VariableHolderRequestBus::Events::GetBaseValue)
			->Event("GetBonusValue", &VariableHolderRequestBus::Events::GetBonusValue);



		AZ_Printf(0, "added VariableManagerComponent to behaviorContext");
	}

	AzFramework::NetworkContext* netContext = azrtti_cast<AzFramework::NetworkContext*>(reflection);
	if (netContext)
	{

		netContext->Class<VariableManagerComponent>()
			->Chunk<VariableManagerReplicaChunk>()
			// ->Field("Variables", &VariableManagerReplicaChunk::m_variables)
			// ->Field("VariablesByEntityId", &VariableManagerReplicaChunk::m_variablesByEntity)
			;
	}
}


void VariableManagerComponent::Init()
{
}
void VariableManagerComponent::Activate()
{
	AZ_Printf(0, "VariableManager activated!");
	VariableManagerRequestBus::Handler::BusConnect();

#if defined(DEDICATED_SERVER)
	ISystem* system = nullptr;
	EBUS_EVENT_RESULT(system, CrySystemRequestBus, GetCrySystem);
	if (system)
	{
		SessionEventBus::Handler::BusConnect(system->GetINetwork()->GetGridMate());
	}
#endif
}
void VariableManagerComponent::Deactivate()
{
#if defined(DEDICATED_SERVER)
	SessionEventBus::Handler::BusDisconnect();
#endif
	VariableManagerRequestBus::Handler::BusDisconnect();
}


GridMate::ReplicaChunkPtr VariableManagerComponent::GetNetworkBinding()
{
	auto replicaChunk = GridMate::CreateReplicaChunk<VariableManagerReplicaChunk>();
	replicaChunk->SetHandler(this);
	m_replicaChunk = replicaChunk;

	UpdateValues();

	return m_replicaChunk;
}

void VariableManagerComponent::SetNetworkBinding(GridMate::ReplicaChunkPtr chunk)
{
	chunk->SetHandler(this);
	m_replicaChunk = chunk;

	if (VariableManagerReplicaChunk* variableManagerChunk = static_cast<VariableManagerReplicaChunk*>(m_replicaChunk.get()))
	{
		// m_variables = variableManagerChunk->m_variables.Get();
		// m_variablesByEntity = variableManagerChunk->m_variablesByEntity.Get();
	}
}

void VariableManagerComponent::UnbindFromNetwork()
{
	if (m_replicaChunk)
	{
		m_replicaChunk->SetHandler(nullptr);
		m_replicaChunk = nullptr;
	}
}

void VariableManagerComponent::OnMemberJoined(GridSession * session, GridMember * member)
{
	// const MemberIDCompact playerId = member->GetIdCompact();

	// if (session->GetMyMember()->GetIdCompact() == playerId) return; //ignore ourselves, the server

	// Synchronize();
	// this is handled by gridmate
}

void VariableManagerComponent::OnMemberLeaving(GridSession * session, GridMember * member)
{
}

void VariableManagerComponent::UpdateValues ()
{
	m_hasChanged = true;

	// don't bother net syncing mid-update, we're going to change this again synchronously
	if (m_updating || !m_replicaChunk)
	{
		return;
	}
	if (VariableManagerReplicaChunk* variableManagerChunk = static_cast<VariableManagerReplicaChunk*>(m_replicaChunk.get()))
	{
		// variableManagerChunk->m_variables.Set(m_variables);
	}
}

bool VariableManagerComponent::RegisterDependentVariable(VariableId variableId)
{
	bool result = false;
	EBUS_EVENT_ID_RESULT(result, variableId.GetEntityId(), VariableHolderRequestBus, RegisterDependentVariable, variableId);
	return result;
}

bool VariableManagerComponent::RegisterVariable(VariableId variableId)
{
	bool result = false;
	EBUS_EVENT_ID_RESULT(result, variableId.GetEntityId(), VariableHolderRequestBus, RegisterVariable, variableId);
	return result;
}

bool VariableManagerComponent::RegisterVariableAndInizialize(VariableId variableId, Value value)
{
	bool result = false;
	EBUS_EVENT_ID_RESULT(result, variableId.GetEntityId(), VariableHolderRequestBus, RegisterVariableAndInizialize, variableId, value);
	return result;
}

bool VariableManagerComponent::VariableExists(VariableId variableId)
{
	bool result = false;
	EBUS_EVENT_ID_RESULT(result, variableId.GetEntityId(), VariableHolderRequestBus, VariableExists, variableId);
	return result;
}

Value VariableManagerComponent::GetBaseValue(VariableId variableId)
{
	Value result = 0.0f;
	EBUS_EVENT_ID_RESULT(result, variableId.GetEntityId(), VariableHolderRequestBus, GetBaseValue, variableId);
	return result;
}
Value VariableManagerComponent::GetBonusValue(VariableId variableId)
{
	Value result = 0.0f;
	EBUS_EVENT_ID_RESULT(result, variableId.GetEntityId(), VariableHolderRequestBus, GetBonusValue, variableId);
	return result;
}

Value VariableManagerComponent::GetValue(VariableId variableId)
{
	Value result = 0.0f;
	EBUS_EVENT_ID_RESULT(result, variableId.GetEntityId(), VariableHolderRequestBus, GetValue, variableId);
	return result;
}

bool  VariableManagerComponent::SetValue(VariableId variableId, Value value)
{
	bool result = false;
	EBUS_EVENT_ID_RESULT(result, variableId.GetEntityId(), VariableHolderRequestBus, SetValue, variableId, value);
	return result;
}

bool VariableManagerComponent::OnNetSetValue(VariableId variableId, Value value, const GridMate::RpcContext &)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		AZ_Printf(0, "Called VariableManagerComponent::OnNetSetValue(%s,%f)", variableId.ToString().c_str(), value);
		if (!SetValue(variableId, value))AZ_Printf(0, "Called VariableManagerComponent::OnNetSetValue variable not found");

		AZ_Printf(0, "Called VariableManagerComponent::OnNetSetValue result:%f", m_variables[variableId].value);

	}

	return true;
}

AZStd::string VariableManagerComponent::ToString()
{
	return m_log;
}

void VariableManagerComponent::OnNewVariables (const VariableManagerComponent::VariableMap& variables, const GridMate::TimeContext& tc)
{
	if (!m_replicaChunk)
	{
		return;
	}
	if (AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return;
	}
	// AZ_Printf(0, "OnNewVariables variable length: %d vs %d", m_variables.size(), variables.size());
	m_variables = variables;
}

void VariableManagerComponent::OnNewVariablesByEntity (const VariableManagerComponent::VariableIdMap& variablesByEntity, const GridMate::TimeContext& tc)
{
	if (!m_replicaChunk)
	{
		return;
	}
	if (AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return;
	}
	m_variablesByEntity = variablesByEntity;
	// m_variablesByEntity.clear();
	for (auto &it : variablesByEntity)
	{
		AZ_Printf(0, "%s has %d variables", it.first.ToString().c_str(), it.second.size());
		// m_variablesByEntity[it.first] = it.second;
	}
	// m_variablesByEntity = variablesByEntity;
}

//####// LUA HANDLERS //####//

//VariableRequestBusHandler
Value VariableRequestBusHandler::GetValue(float deltaTime, AZ::ScriptTimePoint time)
{
	Value result = 0;
	CallResult(result, FN_GetValue, deltaTime, time);
	return result;
}

Value VariableRequestBusHandler::OnSet(Value current, Value proposed)
{
	Value result = proposed;
	CallResult(result, FN_OnSet, current, proposed);
	return result;
}

Value VariableRequestBusHandler::GetMinValue()
{
	Value result = 0;
	CallResult(result, FN_GetMinValue);
	return result;
}
Value VariableRequestBusHandler::GetMaxValue()
{
	Value result = -1;
	CallResult(result, FN_GetMaxValue);
	return result;
}


//VariableBonusModifierBusHandler
Value VariableBonusModifierBusHandler::GetModifierBonus(AZStd::string id)
{
	Value result = 0;
	CallResult(result, FN_GetModifierBonus, id);
	return result;
}

