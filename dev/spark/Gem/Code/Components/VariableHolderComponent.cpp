#include "spark_precompiled.h"

#include "VariableHolderComponent.h"

#include <AzFramework/Network/NetworkContext.h>
#include <GridMate/Replica/ReplicaChunk.h>
#include <GridMate/Replica/ReplicaFunctions.h>
#include <GridMate/Serialize/ContainerMarshal.h>
// NetQuery
#include <AzFramework/Network/NetBindingHandlerBus.h>

#include "Busses/VariableBus.h"
#include "Utils/Marshaler.h"
#include "Utils/Filter.h"
#include "Busses/CooldownBus.h"

using namespace spark;

class VariableHolderReplicaChunk
	: public GridMate::ReplicaChunkBase
{
	friend VariableHolderComponent;
public:
	GM_CLASS_ALLOCATOR(VariableHolderReplicaChunk);

#define VH_INITIALIZE_DATA_PAIR(i) \
	, m_dataPair ## i ## ("Data Pair Channel " #i )

	VariableHolderReplicaChunk()
		: m_variables("Variables")
		, m_component(nullptr)
		VH_INITIALIZE_DATA_PAIR(0)
		VH_INITIALIZE_DATA_PAIR(1)
		VH_INITIALIZE_DATA_PAIR(2)
		VH_INITIALIZE_DATA_PAIR(3)
		VH_INITIALIZE_DATA_PAIR(4)
		VH_INITIALIZE_DATA_PAIR(5)
		VH_INITIALIZE_DATA_PAIR(6)
		VH_INITIALIZE_DATA_PAIR(7)
		VH_INITIALIZE_DATA_PAIR(8)
		VH_INITIALIZE_DATA_PAIR(9)
		VH_INITIALIZE_DATA_PAIR(10)
		VH_INITIALIZE_DATA_PAIR(11)
		VH_INITIALIZE_DATA_PAIR(12)
		VH_INITIALIZE_DATA_PAIR(13)
		VH_INITIALIZE_DATA_PAIR(14)
		VH_INITIALIZE_DATA_PAIR(15)
		VH_INITIALIZE_DATA_PAIR(16)
		VH_INITIALIZE_DATA_PAIR(17)
		VH_INITIALIZE_DATA_PAIR(18)
		VH_INITIALIZE_DATA_PAIR(19)
		VH_INITIALIZE_DATA_PAIR(20)
		VH_INITIALIZE_DATA_PAIR(21)
		VH_INITIALIZE_DATA_PAIR(22)
		VH_INITIALIZE_DATA_PAIR(23)
		VH_INITIALIZE_DATA_PAIR(24)
		VH_INITIALIZE_DATA_PAIR(25)
		VH_INITIALIZE_DATA_PAIR(26)
		VH_INITIALIZE_DATA_PAIR(27)
		VH_INITIALIZE_DATA_PAIR(28)
		VH_INITIALIZE_DATA_PAIR(29)
		VH_INITIALIZE_DATA_PAIR(30)
	{
	}
#undef VH_INITIALIZE_DATA_PAIR

	bool IsReplicaMigratable() override
	{
		return true;
	}

	static const char* GetChunkName()
	{
		return "VariableHolderReplicaChunk";
	}
	// GridMate::Rpc<RpcArg<VariableId>, RpcArg<Value>>::BindInterface<VariableHolderComponent, &VariableHolderComponent::OnNetSetValue> m_onSetValue = { "SetValue" };

	GridMate::DataSet<VariableHolderComponent::VariableMap, GridMate::ContainerMarshaler<VariableHolderComponent::VariableMap>>::BindInterface<VariableHolderComponent, &VariableHolderComponent::OnNewVariables> m_variables;

	typedef GridMate::DataSet<VariableHolderComponent::VariableDataPair>::BindInterface<VariableHolderComponent, &VariableHolderComponent::OnNewDataPair> DataPairSet;
	DataPairSet m_dataPair0;
	DataPairSet m_dataPair1;
	DataPairSet m_dataPair2;
	DataPairSet m_dataPair3;
	DataPairSet m_dataPair4;
	DataPairSet m_dataPair5;
	DataPairSet m_dataPair6;
	DataPairSet m_dataPair7;
	DataPairSet m_dataPair8;
	DataPairSet m_dataPair9;
	DataPairSet m_dataPair10;
	DataPairSet m_dataPair11;
	DataPairSet m_dataPair12;
	DataPairSet m_dataPair13;
	DataPairSet m_dataPair14;
	DataPairSet m_dataPair15;
	DataPairSet m_dataPair16;
	DataPairSet m_dataPair17;
	DataPairSet m_dataPair18;
	DataPairSet m_dataPair19;
	DataPairSet m_dataPair20;
	DataPairSet m_dataPair21;
	DataPairSet m_dataPair22;
	DataPairSet m_dataPair23;
	DataPairSet m_dataPair24;
	DataPairSet m_dataPair25;
	DataPairSet m_dataPair26;
	DataPairSet m_dataPair27;
	DataPairSet m_dataPair28;
	DataPairSet m_dataPair29;
	DataPairSet m_dataPair30;

private:
	VariableHolderComponent* m_component = nullptr;
};

void VariableHolderComponent::Reflect(AZ::ReflectContext* reflection)
{
	if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
	{
		serializationContext->Class<VariableHolderComponent, AZ::Component, AzFramework::NetBindable>()
			->Version(1);

		if (auto editContext = serializationContext->GetEditContext())
		{
			editContext->Class<VariableHolderComponent>("VariableHolderComponent", "Manage unit's variables(attributes,hp,mana,gold,etc.)")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
				->Attribute(AZ::Edit::Attributes::Category, "spark")
				->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"));
		}
	}

	AzFramework::NetworkContext* netContext = azrtti_cast<AzFramework::NetworkContext*>(reflection);
	if (netContext)
	{

#define VH_REFLECT_DATA_PAIR(i) \
		->Field("Data Pair Channel " #i, &VariableHolderReplicaChunk::m_dataPair ## i )

		netContext->Class<VariableHolderComponent>()
			->Chunk<VariableHolderReplicaChunk>()
			->Field("Variables", &VariableHolderReplicaChunk::m_variables)
			VH_REFLECT_DATA_PAIR(0)
			VH_REFLECT_DATA_PAIR(1)
			VH_REFLECT_DATA_PAIR(2)
			VH_REFLECT_DATA_PAIR(3)
			VH_REFLECT_DATA_PAIR(4)
			VH_REFLECT_DATA_PAIR(5)
			VH_REFLECT_DATA_PAIR(6)
			VH_REFLECT_DATA_PAIR(7)
			VH_REFLECT_DATA_PAIR(8)
			VH_REFLECT_DATA_PAIR(9)
			VH_REFLECT_DATA_PAIR(10)
			VH_REFLECT_DATA_PAIR(11)
			VH_REFLECT_DATA_PAIR(12)
			VH_REFLECT_DATA_PAIR(13)
			VH_REFLECT_DATA_PAIR(14)
			VH_REFLECT_DATA_PAIR(15)
			VH_REFLECT_DATA_PAIR(16)
			VH_REFLECT_DATA_PAIR(17)
			VH_REFLECT_DATA_PAIR(18)
			VH_REFLECT_DATA_PAIR(19)
			VH_REFLECT_DATA_PAIR(20)
			VH_REFLECT_DATA_PAIR(21)
			VH_REFLECT_DATA_PAIR(22)
			VH_REFLECT_DATA_PAIR(23)
			VH_REFLECT_DATA_PAIR(24)
			VH_REFLECT_DATA_PAIR(25)
			VH_REFLECT_DATA_PAIR(26)
			VH_REFLECT_DATA_PAIR(27)
			VH_REFLECT_DATA_PAIR(28)
			VH_REFLECT_DATA_PAIR(29)
			VH_REFLECT_DATA_PAIR(30)
			;
	}
#undef VH_REFLECT_DATA_PAIR
}

void VariableHolderComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
{
	provided.push_back(AZ_CRC("VariableHolderService"));
}

void VariableHolderComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
{
	incompatible.push_back(AZ_CRC("VariableHolderService"));
}

void VariableHolderComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
{

}

void VariableHolderComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
{

}


void VariableHolderComponent::Init ()
{
	///@TODO: implement Init
}

void VariableHolderComponent::Activate ()
{
	VariableHolderRequestBus::Handler::BusConnect(GetEntityId());
	if (AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		AZ::TickBus::Handler::BusConnect();
	}
}

void VariableHolderComponent::Deactivate ()
{
	AZ::TickBus::Handler::BusDisconnect();
	VariableHolderRequestBus::Handler::BusDisconnect();
}

void VariableHolderComponent::OnNewVariables (const VariableHolderComponent::VariableMap& variables, const GridMate::TimeContext& tc)
{
	if (!m_replicaChunk || AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return;
	}
	// AZ_Printf(0, "OnNewVariables got new variables!: %d", m_variables.size());
	for (auto &it : variables)
	{
		m_variables[it.first] = it.second;
	}
}

void VariableHolderComponent::OnNewDataPair (const VariableHolderComponent::VariableDataPair& dataPair, const GridMate::TimeContext& tc)
{
	if (!m_replicaChunk || AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return;
	}
	// AZ_Printf(0, "OnNewVariables got new variables!: %d", m_variables.size());
	m_variables[dataPair.first] = dataPair.second;
	auto variableId = VariableId(GetEntityId(), dataPair.first);
	EBUS_EVENT_ID(variableId, VariableNotificationBus, OnSetValue, variableId, dataPair.second.value);
	EBUS_EVENT_ID(GetEntityId(), VariableHolderNotificationBus, OnSetValue, variableId, dataPair.second.value);
	EBUS_EVENT(VariableManagerNotificationBus, OnSetValue, variableId, dataPair.second.value);
}

//NetBindable
GridMate::ReplicaChunkPtr VariableHolderComponent::GetNetworkBinding ()
{
	auto replicaChunk = GridMate::CreateReplicaChunk<VariableHolderReplicaChunk>();
	replicaChunk->SetHandler(this);
	m_replicaChunk = replicaChunk;

	// set all values
	if (AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		UpdateValues();
	}
	else
	{
		m_variables = replicaChunk->m_variables.Get();
	}

	return m_replicaChunk;
}

void VariableHolderComponent::SetNetworkBinding (GridMate::ReplicaChunkPtr chunk)
{
	chunk->SetHandler(this);
	m_replicaChunk = chunk;

	if (AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		UpdateValues();
	}
	else
	{
		VariableHolderReplicaChunk* variableHolderChunk = static_cast<VariableHolderReplicaChunk*>(m_replicaChunk.get());
		m_variables = variableHolderChunk->m_variables.Get();
	}
}

void VariableHolderComponent::UnbindFromNetwork ()
{
	if (m_replicaChunk)
	{
		if (VariableHolderReplicaChunk* variableHolderChunk = static_cast<VariableHolderReplicaChunk*>(m_replicaChunk.get()))
		{
			variableHolderChunk->m_component = nullptr;
		}
		m_replicaChunk->SetHandler(nullptr);
		m_replicaChunk = nullptr;
	}
}

// variable holder request bus
bool VariableHolderComponent::RegisterDependentVariable (VariableId variableId)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return true;
	}
	auto variableName = variableId.m_variableId;
	// AZ_Printf(0, "VariableHolderComponent::RegisterDependentVariable %s", variableName.c_str());
	//if (VariableExists(variableId))return false;
	auto it = m_variables.find(variableName);
	if (it != m_variables.end())
	{
		it->second.isDependent = true;
		return true;
	}

	VariableData data;
	data.isDependent = true;
	m_variables[variableName] = data;

	m_hasChanged = true;
	UpdateValue(variableName);

	return true;
}

bool VariableHolderComponent::RegisterVariable (VariableId variableId)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return true;
	}
	auto variableName = variableId.m_variableId;
	// AZ_Printf(0, "VariableHolderComponent::RegisterVariable %s", variableName.c_str());

	if (VariableExists(variableId))
	{
		return false;
	}

	VariableData data;
	m_variables[variableName] = data;

	m_hasChanged = true;
	UpdateValue(variableName);

	return true;
}

bool VariableHolderComponent::RegisterVariableAndInizialize (VariableId variableId, Value value)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return true;
	}
	auto variableName = variableId.m_variableId;
	// AZ_Printf(0, "VariableHolderComponent::RegisterVariableAndInizialize %s", variableName.c_str());

	//if (VariableExists(variableId))return false;
	auto it = m_variables.find(variableName);
	if (it != m_variables.end()) {
		it->second.value = value;
		it->second.isDependent = false;
		return true;
	}

	VariableData data;
	data.value = value;
	m_variables[variableName] = data;

	m_hasChanged = true;
	UpdateValue(variableName);

	return true;
}

bool VariableHolderComponent::SetValue (VariableId variableId, Value value)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return true;
	}
	auto variableName = variableId.m_variableId;


	auto it = m_variables.find(variableName);
	if (it == m_variables.end())return false;

	VariableId id = variableId;
	if (SPARK_FILTER_EVENT_PREVENT(VariableManagerNotificationBus, OnSetValueFilter, id, value)
		|| SPARK_FILTER_EVENT_ID_PREVENT(GetEntityId(), VariableHolderNotificationBus, OnSetValueFilter, id, value)
		|| SPARK_FILTER_EVENT_ID_PREVENT(variableId, VariableNotificationBus, OnSetValueFilter, id, value))
	{
		return false;
	}

	//update locally
	Value modifiedValue = value;
	EBUS_EVENT_ID_RESULT(modifiedValue, variableId, VariableRequestBus, OnSet, it->second.value, value);

	if (fabs(it->second.value - modifiedValue) > 0.00001f)
	{
		it->second.value = modifiedValue;
		UpdateValue(variableName);
	}
	else
	{
		it->second.value = modifiedValue;
	}
	
	EBUS_EVENT_ID(variableId, VariableNotificationBus, OnSetValue, id, value);
	EBUS_EVENT_ID(GetEntityId(), VariableHolderNotificationBus, OnSetValue, id, value);
	EBUS_EVENT(VariableManagerNotificationBus, OnSetValue, id, value);

	return true;
}

void VariableHolderComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time)
{
	// m_log = AZStd::string::format("[%s]VariableHolderComponent variables:\n", GetEntityId().ToString().c_str());

	//if some variable is set while updating, it doesn't need to be passed to the network
	m_updating = true;
	auto entityId = GetEntityId();

	for (auto &it : m_variables)
	{
		VariableId variableId = VariableId(entityId, it.first);
		if (it.second.isDependent)
		{
			//Update all dependent variables
			Value value = 0;
			EBUS_EVENT_ID_RESULT(value, variableId, VariableRequestBus, GetValue, deltaTime, time);
			if (fabs(it.second.value - value) > (0.0001f * deltaTime))
			{
				// AZ_Printf(0, "Dependant variable is changing: %s = %f", it.first.c_str(), value);
				it.second.value = value;
				UpdateValue(it.first);
			}
		}
		else
		{
			if (it.first == "cooldown_timer" && it.second.value>0.0f)
			{
				it.second.value = max(0.0f, it.second.value - deltaTime);
				UpdateValue(it.first);

				if (it.second.value == 0.0f)
				{
					//when the cooldown finish, update the "cooldown_current" variable for the hud
					SetValue(VariableId(entityId, "cooldown_current"), GetValue(VariableId(entityId, "cooldown")));
					EBUS_EVENT_ID(entityId, CooldownNotificationBus, OnCooldownFinished);
				}
			}
		}

		//get and sum all (flat) modifiers
		//AZ::EBusAggregateResults<bool> results;
		AZ::EBusReduceResult<Value, AZStd::plus<Value>> result(0);
		//VariableBonusModifierBus::EventResult(result, variableId, &VariableBonusModifierBus::Events::GetModifierBonus, it.first);
		EBUS_EVENT_ID_RESULT(result, variableId, VariableBonusModifierBus, GetModifierBonus, it.first);
		if (fabs(it.second.bonusValue - result.value) > (0.0001f * deltaTime))
		{
			it.second.bonusValue = result.value;
			UpdateValue(it.first);
		}

		//todo check if inside min-max range
	}

	//create the log string	
	// for (auto entity_variables : m_variablesByEntity)
	// {
	// 	m_log += AZStd::string::format("%s:", entity_variables.first.ToString().c_str());
	// 	for (auto variableId : entity_variables.second)
	// 	{
	// 		VariableData &data = m_variables[VariableId(entity_variables.first, variableId)];


	// 		m_log += AZStd::string::format("\"%s\":%f+%f, ", variableId.c_str(), data.value, data.bonusValue);
	// 	}
	// 	m_log += "\n";
	// }

	m_updating = false;
	if (m_hasChanged)
	{
		m_hasChanged = false;
		UpdateValues();
	}

	m_replicaValueCount = 0;
}

void VariableHolderComponent::UpdateValues ()
{
	// don't bother net syncing mid-update, we're going to change this again synchronously
	if (m_updating || !m_replicaChunk)
	{
		m_hasChanged = true;
		return;
	}
	if (VariableHolderReplicaChunk* variableHolderChunk = static_cast<VariableHolderReplicaChunk*>(m_replicaChunk.get()))
	{
		// AZ_Printf(0, "Updating variable holder values: %d", m_variables.size());
		variableHolderChunk->m_variables.Set(m_variables);
	}
}

void VariableHolderComponent::UpdateValue (const AZStd::string& valueName)
{
	if (!m_replicaChunk || !AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return;
	}
	if (VariableHolderReplicaChunk* variableHolderChunk = static_cast<VariableHolderReplicaChunk*>(m_replicaChunk.get()))
	{
		auto it = m_valueIndexMap.find(valueName);
		int nextIndex = -1;
		if (it != m_valueIndexMap.end())
		{
			nextIndex = it->second;
			// we're already syncing this variable, use the existing index...
		}
		else
		{
			nextIndex = ++m_replicaValueIndex % 31;
			m_replicaValueCount++;

			if (m_replicaValueCount > 16)
			{
				UpdateValues();
			}
			else
			{
				for (auto &it : m_valueIndexMap)
				{
					if (it.second == nextIndex)
					{
						AZ_Printf(0, "Replacing the sync of value %s with %s", it.first.c_str(), valueName.c_str());
						m_valueIndexMap.erase(it.first);
						break;
					}
				}
				m_valueIndexMap[valueName] = nextIndex;
			}
		}

	// AZ_Printf(0, "Sending %s over channel %d", valueName.c_str(), i ); \

#define VH_DATA_SET_INDEX(i) \
case i : \
	variableHolderChunk->m_dataPair ## i ## .Set(VariableHolderComponent::VariableDataPair(valueName, m_variables[valueName])); \
	break;

		// sync now using nextIndex
		switch (nextIndex)
		{
			VH_DATA_SET_INDEX(0)
			VH_DATA_SET_INDEX(1)
			VH_DATA_SET_INDEX(2)
			VH_DATA_SET_INDEX(3)
			VH_DATA_SET_INDEX(4)
			VH_DATA_SET_INDEX(5)
			VH_DATA_SET_INDEX(6)
			VH_DATA_SET_INDEX(7)
			VH_DATA_SET_INDEX(8)
			VH_DATA_SET_INDEX(9)
			VH_DATA_SET_INDEX(10)
			VH_DATA_SET_INDEX(11)
			VH_DATA_SET_INDEX(12)
			VH_DATA_SET_INDEX(13)
			VH_DATA_SET_INDEX(14)
			VH_DATA_SET_INDEX(15)
			VH_DATA_SET_INDEX(16)
			VH_DATA_SET_INDEX(17)
			VH_DATA_SET_INDEX(18)
			VH_DATA_SET_INDEX(19)
			VH_DATA_SET_INDEX(20)
			VH_DATA_SET_INDEX(21)
			VH_DATA_SET_INDEX(22)
			VH_DATA_SET_INDEX(23)
			VH_DATA_SET_INDEX(24)
			VH_DATA_SET_INDEX(25)
			VH_DATA_SET_INDEX(26)
			VH_DATA_SET_INDEX(27)
			VH_DATA_SET_INDEX(28)
			VH_DATA_SET_INDEX(29)
			VH_DATA_SET_INDEX(30)
		}
	}
#undef VH_DATA_SET_INDEX
}

Value VariableHolderComponent::GetValue (VariableId variableId)
{
	auto it = m_variables.find(variableId.m_variableId);
	if (it != m_variables.end())
	{
		return  it->second.value + it->second.bonusValue;
	}
	// AZ_Printf(0, "Called VariableHolderComponent::GetValue() with invalid id : %s", variableId.m_variableId.c_str());
	return 0;
}

Value VariableHolderComponent::GetBaseValue (VariableId variableId)
{
	auto it = m_variables.find(variableId.m_variableId);
	if (it != m_variables.end()) {
		return  it->second.value;
	}
	return 0;
}

Value VariableHolderComponent::GetBonusValue (VariableId variableId)
{
	auto it = m_variables.find(variableId.m_variableId);
	if (it != m_variables.end()) {
		return  it->second.bonusValue;
	}
	return 0;
}

bool VariableHolderComponent::VariableExists (VariableId variableId)
{
	auto it = m_variables.find(variableId.m_variableId);
	return it != m_variables.end();
}
