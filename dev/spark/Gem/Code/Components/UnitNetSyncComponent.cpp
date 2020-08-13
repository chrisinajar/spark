#include "spark_precompiled.h"

#include <AzFramework/Network/NetworkContext.h>
#include <AzFramework/Network/NetBindingHandlerBus.h>
#include <GridMate/Replica/ReplicaFunctions.h>
#include <GridMate/Replica/RemoteProcedureCall.h>

#include <GridMate/Replica/ReplicaChunk.h>
#include <GridMate/Replica/DataSet.h>
#include <GridMate/Serialize/CompressionMarshal.h>

#include "UnitNetSyncComponent.h"

#include <Integration/AnimGraphComponentBus.h>

#include "Utils/SparkNavPath.h"
#include "Utils/Marshaler.h"

using namespace spark;
using namespace GridMate;


template<>
class GridMate::Marshaler<SparkNavPath>
{
public:
	typedef SparkNavPath DataType;

    //static const AZStd::size_t MarshalSize = sizeof(DataType);

    void Marshal(WriteBuffer& wb, const DataType& value) const
    {
		Marshaler<int> intMarshaler;
		Marshaler<float> floatMarshaler;

		wb.Write(value.m_valid);
		floatMarshaler.Marshal(wb, value.m_currentFrac);
		intMarshaler.Marshal(wb,value.m_pathPoints.size());
		
		
		for (auto p : value.m_pathPoints)
		{
			floatMarshaler.Marshal(wb, p.GetX());
			floatMarshaler.Marshal(wb, p.GetY());
			floatMarshaler.Marshal(wb, p.GetZ());
		}
    }
    void Unmarshal(DataType& value, ReadBuffer& rb) const
    {
        Marshaler<int> intMarshaler;
		Marshaler<float> floatMarshaler;
		int size = 0;

		rb.Read(value.m_valid);
		floatMarshaler.Unmarshal(value.m_currentFrac, rb);
		intMarshaler.Unmarshal(size,rb);
		
		for(int i=0;i<size;i++)
		{
			float x,y,z;
			floatMarshaler.Unmarshal( x, rb);
			floatMarshaler.Unmarshal( y, rb);
			floatMarshaler.Unmarshal( z, rb);
			value.m_pathPoints.push_back(AZ::Vector3(x,y,z));
		}
    }
};


class UnitNetSyncComponentChunk
	: public GridMate::ReplicaChunkBase
{
public:
	GM_CLASS_ALLOCATOR(UnitNetSyncComponentChunk);

	UnitNetSyncComponentChunk()
		: m_onPathChanged("PathChanged")
	{}

	bool IsReplicaMigratable() override
	{
		return true;
	}

	static const char* GetChunkName()
	{
		return "UnitNetSyncComponentChunk";
	}

	GridMate::Rpc<RpcArg<SparkNavPath>>::BindInterface<UnitNetSyncComponent, &UnitNetSyncComponent::OnNetPathChanged> m_onPathChanged;

	GridMate::Rpc<>::BindInterface<UnitNetSyncComponent, &UnitNetSyncComponent::OnNetSynchronize> m_OnSynchronize = {"Sync"};
	GridMate::Rpc<RpcArg<const AZ::Transform&>>::BindInterface<UnitNetSyncComponent, &UnitNetSyncComponent::OnNetTransformChanged> m_onTransformChanged = { "TransformChanged" };

	GridMate::Rpc<RpcArg<const AZStd::string&>, RpcArg<const bool&>>::BindInterface<UnitNetSyncComponent, &UnitNetSyncComponent::OnNetSetNamedParameterBool> m_SetNamedParameterBool = {"SetNamedParameterBool"};
	GridMate::Rpc<RpcArg<const AZStd::string&>, RpcArg<const float&>>::BindInterface<UnitNetSyncComponent, &UnitNetSyncComponent::OnNetSetNamedParameterFloat> m_SetNamedParameterFloat = {"SetNamedParameterFloat"};

	GridMate::Rpc<RpcArg<UnitOrder>, RpcArg<bool>>::BindInterface<UnitNetSyncComponent, &UnitNetSyncComponent::OnNetOnNewOrder> m_OnNewOrder = {"OnNewOrder"};
	GridMate::Rpc<RpcArg<ModifierId>>::BindInterface<UnitNetSyncComponent, &UnitNetSyncComponent::OnNetOnModifierAttached> m_OnModifierAttached = {"OnModifierAttached"};
	GridMate::Rpc<RpcArg<ModifierId>>::BindInterface<UnitNetSyncComponent, &UnitNetSyncComponent::OnNetOnModifierDetached> m_OnModifierDetached = {"OnModifierDetached"};
	GridMate::Rpc<>::BindInterface<UnitNetSyncComponent, &UnitNetSyncComponent::OnNetOnSpawned> m_OnSpawned = {"OnSpawned"};
	GridMate::Rpc<>::BindInterface<UnitNetSyncComponent, &UnitNetSyncComponent::OnNetOnDeath> m_OnDeath = {"OnDeath"};
};



void UnitNetSyncComponent::Reflect(AZ::ReflectContext* reflection)
{
	if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
	{
		serializationContext->Class<UnitNetSyncComponent, AZ::Component,AzFramework::NetBindable>()
			->Version(1);

		if (auto editContext = serializationContext->GetEditContext())
		{
			editContext->Class<UnitNetSyncComponent>("UnitNetSyncComponent", "sync the game events")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
				->Attribute(AZ::Edit::Attributes::Category, "spark")
				->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"));
		}
	}



	AzFramework::NetworkContext* netContext = azrtti_cast<AzFramework::NetworkContext*>(reflection);
	if (netContext)
	{
		netContext->Class<UnitNetSyncComponent>()
			->Chunk<UnitNetSyncComponentChunk>()
			->RPC<UnitNetSyncComponentChunk, UnitNetSyncComponent>("PathChanged", &UnitNetSyncComponentChunk::m_onPathChanged)
			->RPC<UnitNetSyncComponentChunk, UnitNetSyncComponent>("Sync", &UnitNetSyncComponentChunk::m_OnSynchronize)
			->RPC<UnitNetSyncComponentChunk, UnitNetSyncComponent>("TransformChanged", &UnitNetSyncComponentChunk::m_onTransformChanged)
			->RPC<UnitNetSyncComponentChunk, UnitNetSyncComponent>("SetNamedParameterBool", &UnitNetSyncComponentChunk::m_SetNamedParameterBool)
			->RPC<UnitNetSyncComponentChunk, UnitNetSyncComponent>("SetNamedParameterFloat", &UnitNetSyncComponentChunk::m_SetNamedParameterFloat)
			->RPC<UnitNetSyncComponentChunk, UnitNetSyncComponent>("OnNewOrder", &UnitNetSyncComponentChunk::m_OnNewOrder)
			->RPC<UnitNetSyncComponentChunk, UnitNetSyncComponent>("OnModifierAttached", &UnitNetSyncComponentChunk::m_OnModifierAttached)
			->RPC<UnitNetSyncComponentChunk, UnitNetSyncComponent>("OnModifierDetached", &UnitNetSyncComponentChunk::m_OnModifierDetached)
			->RPC<UnitNetSyncComponentChunk, UnitNetSyncComponent>("OnSpawned", &UnitNetSyncComponentChunk::m_OnSpawned)
			->RPC<UnitNetSyncComponentChunk, UnitNetSyncComponent>("OnDeath", &UnitNetSyncComponentChunk::m_OnDeath)
		;
	}
}


void UnitNetSyncComponent::Init()
{

}
void UnitNetSyncComponent::Activate()
{
	m_isAuthoritative = AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId());

	UnitNetSyncRequestBus::Handler::BusConnect(GetEntityId());
	if (m_isAuthoritative)
	{
		UnitNavigationNotificationBus::Handler::BusConnect(GetEntityId());
		UnitNotificationBus::Handler::BusConnect(GetEntityId());
	}
}
void UnitNetSyncComponent::Deactivate()
{
	UnitNavigationNotificationBus::Handler::BusDisconnect();
	UnitNetSyncRequestBus::Handler::BusDisconnect();
	UnitNotificationBus::Handler::BusDisconnect();
}
	
GridMate::ReplicaChunkPtr UnitNetSyncComponent::GetNetworkBinding()
{
	AZ_Printf(0, "UnitNetSyncComponent::GetNetworkBinding");
	m_chunk = GridMate::CreateReplicaChunk<UnitNetSyncComponentChunk>();
	m_chunk->SetHandler(this);
	return m_chunk;
}

void UnitNetSyncComponent::SetNetworkBinding(GridMate::ReplicaChunkPtr chunk)
{
	AZ_Printf(0, "UnitNetSyncComponent::SetNetworkBinding");
		
	m_chunk = chunk;
	m_chunk->SetHandler(this);

	
	Synchronize();
}

void UnitNetSyncComponent::UnbindFromNetwork()
{
	AZ_Printf(0, "UnitNetSyncComponent::UnbindFromNetwork");
	if (m_chunk)
	{
		m_chunk->SetHandler(nullptr);
		m_chunk = nullptr;
	}
}

void UnitNetSyncComponent::SetNamedParameterBool (const AZStd::string &valueName, const bool &value)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return;
	}

	if (UnitNetSyncComponentChunk* chunk = static_cast<UnitNetSyncComponentChunk*>(m_chunk.get()))
	{
		chunk->m_SetNamedParameterBool(valueName, value);
	}
	else
	{
		AZStd::vector<AZ::EntityId> children;
		EBUS_EVENT_ID_RESULT(children, GetEntityId(), AZ::TransformBus, GetChildren);

		for (auto c : children)
		{
			EBUS_EVENT_ID(c, EMotionFX::Integration::AnimGraphComponentRequestBus, SetNamedParameterBool, valueName.c_str(), value);
		}
	}
}

bool UnitNetSyncComponent::OnNetSetNamedParameterBool (const AZStd::string &valueName, const bool &value, const GridMate::RpcContext &context)
{
	AZStd::vector<AZ::EntityId> children;
	EBUS_EVENT_ID_RESULT(children, GetEntityId(), AZ::TransformBus, GetChildren);

	for (auto c : children)
	{
		EBUS_EVENT_ID(c, EMotionFX::Integration::AnimGraphComponentRequestBus, SetNamedParameterBool, valueName.c_str(), value);
	}

	return true;
}

void UnitNetSyncComponent::SetNamedParameterFloat (const AZStd::string &valueName, const float &value)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return;
	}

	if (UnitNetSyncComponentChunk* chunk = static_cast<UnitNetSyncComponentChunk*>(m_chunk.get()))
	{
		chunk->m_SetNamedParameterFloat(valueName, value);
	}
	else
	{
		AZStd::vector<AZ::EntityId> children;
		EBUS_EVENT_ID_RESULT(children, GetEntityId(), AZ::TransformBus, GetChildren);

		for (auto c : children)
		{
			EBUS_EVENT_ID(c, EMotionFX::Integration::AnimGraphComponentRequestBus, SetNamedParameterFloat, valueName.c_str(), value);
		}
	}
}

bool UnitNetSyncComponent::OnNetSetNamedParameterFloat (const AZStd::string &valueName, const float &value, const GridMate::RpcContext &context)
{
	AZStd::vector<AZ::EntityId> children;
	EBUS_EVENT_ID_RESULT(children, GetEntityId(), AZ::TransformBus, GetChildren);

	for (auto c : children)
	{
		EBUS_EVENT_ID(c, EMotionFX::Integration::AnimGraphComponentRequestBus, SetNamedParameterFloat, valueName.c_str(), value);
	}
	return true;
}

void UnitNetSyncComponent::OnNewOrder(UnitOrder order, bool queue)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return;
	}
	if (UnitNetSyncComponentChunk* chunk = static_cast<UnitNetSyncComponentChunk*>(m_chunk.get()))
	{
		chunk->m_OnNewOrder(order, queue);
	}
}

bool UnitNetSyncComponent::OnNetOnNewOrder(UnitOrder order, bool queue, const GridMate::RpcContext&)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		AZ_Printf(0, "Proxying unit notification event: OnNewOrder");
		EBUS_EVENT_ID(GetEntityId(), UnitNotificationBus, OnNewOrder, order, queue);
	}
	return true;
}

void UnitNetSyncComponent::OnModifierAttached(ModifierId id)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return;
	}
	if (UnitNetSyncComponentChunk* chunk = static_cast<UnitNetSyncComponentChunk*>(m_chunk.get()))
	{
		chunk->m_OnModifierAttached(id);
	}
}

bool UnitNetSyncComponent::OnNetOnModifierAttached(ModifierId id, const GridMate::RpcContext&)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		// AZ_Printf(0, "Proxying unit notification event: OnModifierAttached");
		// EBUS_EVENT_ID(GetEntityId(), UnitNotificationBus, OnModifierAttached, id);
	}
	return true;
}

void UnitNetSyncComponent::OnModifierDetached(ModifierId id)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return;
	}
	if (UnitNetSyncComponentChunk* chunk = static_cast<UnitNetSyncComponentChunk*>(m_chunk.get()))
	{
		chunk->m_OnModifierDetached(id);
	}
}

bool UnitNetSyncComponent::OnNetOnModifierDetached(ModifierId id, const GridMate::RpcContext&)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		// AZ_Printf(0, "Proxying unit notification event: OnModifierDetached");
		// EBUS_EVENT_ID(GetEntityId(), UnitNotificationBus, OnModifierDetached, id);
	}
	return true;
}

void UnitNetSyncComponent::OnSpawned()
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return;
	}
	if (UnitNetSyncComponentChunk* chunk = static_cast<UnitNetSyncComponentChunk*>(m_chunk.get()))
	{
		chunk->m_OnSpawned();
	}
}

bool UnitNetSyncComponent::OnNetOnSpawned(const GridMate::RpcContext&)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		AZ_Printf(0, "Proxying unit notification event: OnSpawned");
		EBUS_EVENT_ID(GetEntityId(), UnitNotificationBus, OnSpawned);
	}
	return true;
}

void UnitNetSyncComponent::OnDeath()
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return;
	}
	if (UnitNetSyncComponentChunk* chunk = static_cast<UnitNetSyncComponentChunk*>(m_chunk.get()))
	{
		chunk->m_OnDeath();
	}
}

bool UnitNetSyncComponent::OnNetOnDeath(const GridMate::RpcContext&)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		AZ_Printf(0, "Proxying unit notification event: OnDeath");
		EBUS_EVENT_ID(GetEntityId(), UnitNotificationBus, OnDeath);
	}
	return true;
}







// old unused RPCs, new better stuff above this

bool UnitNetSyncComponent::OnNetPathChanged(SparkNavPath path, const GridMate::RpcContext&)
{
	AZ_Printf(0, "UnitNetSyncComponent::OnNetPathChanged (external)");
	EBUS_EVENT_ID(GetEntityId(), UnitNavigationRequestBus, SetPath, path);


	return true;//todo should be true, but crash
}

bool UnitNetSyncComponent::OnNetTransformChanged(const AZ::Transform & transform, const GridMate::RpcContext&)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus, SetWorldTM, transform);
	}
	return true;
}

void UnitNetSyncComponent::Synchronize()
{
	// position related stuff is automatically replicaed, so do nothing now...
	return;
	// UnitNetSyncComponentChunk* chunk = static_cast<UnitNetSyncComponentChunk*>(m_chunk.get());

	// if (!chunk)return;


	// if (AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	// {
	// 	AZ::Transform entityTransform;
	// 	EBUS_EVENT_ID_RESULT(entityTransform, GetEntityId(), AZ::TransformBus, GetWorldTM);
	// 	chunk->m_onTransformChanged(entityTransform);
	// }
	// else
	// {
	// 	chunk->m_OnSynchronize();
	// }
}

bool UnitNetSyncComponent::OnNetSynchronize(const GridMate::RpcContext &)
{
	UnitNetSyncComponentChunk* chunk = static_cast<UnitNetSyncComponentChunk*>(m_chunk.get());

	if (!chunk || !AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))return false;
	
	Synchronize();
	return true;	
}

void UnitNetSyncComponent::OnPathChanged(const SparkNavPath &path)
{
	// AZ_Printf(0, "UnitNetSyncComponent::OnNetPathChanged");
	// if (UnitNetSyncComponentChunk* chunk = static_cast<UnitNetSyncComponentChunk*>(m_chunk.get()))
	// {
	// 	// AZ_Printf(0, "UnitNetSyncComponent::OnNetPathChanged  chunk is valid  : %f",path.m_currentFrac);
	// 	// chunk->m_onPathChanged(path);
	// 	// AZ_Printf(0, "UnitNetSyncComponent::OnNetPathChanged  chunk is valid  : %f",path.m_currentFrac);
	// }
}
