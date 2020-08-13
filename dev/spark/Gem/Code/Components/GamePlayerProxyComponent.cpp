
#include "spark_precompiled.h"

#include "GamePlayerProxyComponent.h"

#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzFramework/Entity/GameEntityContextBus.h>
#include <LmbrCentral/Rendering/ParticleComponentBus.h>

#include <AzFramework/Network/NetworkContext.h>
#include <AzFramework/Network/NetBindingHandlerBus.h>
#include <AzFramework/Network/NetBindingComponent.h>
#include <GridMate/Replica/ReplicaFunctions.h>
#include <GridMate/Replica/ReplicaChunk.h>
#include <GridMate/Replica/DataSet.h>

using namespace spark;
using namespace GridMate;

class GamePlayerProxyReplicaChunk : public GridMate::ReplicaChunkBase
{
public:
    AZ_CLASS_ALLOCATOR(GamePlayerProxyReplicaChunk, AZ::SystemAllocator, 0);

    static const char* GetChunkName() { return "GamePlayerProxyReplicaChunk"; }

       GamePlayerProxyReplicaChunk()
        // : m_particleEffect("ParticleEffect")
    {
    }

    bool IsReplicaMigratable()
    {
        return true;
    }

	GridMate::Rpc<RpcArg<AZStd::string>, RpcArg<AZStd::string>, RpcArg<AZ::EntityId>>::BindInterface<GamePlayerProxyComponent, &GamePlayerProxyComponent::OnClientEvent> m_clientEvent = { "Client Event" };
    // GridMate::DataSet<AZStd::string>::BindInterface<GamePlayerProxyComponent, &GamePlayerProxyComponent::OnNewParticleEffect> m_particleEffect;
};

void GamePlayerProxyComponent::Reflect(AZ::ReflectContext* reflection)
{
	if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
	{
		serializationContext->Class<GamePlayerProxyComponent, AzFramework::NetBindable, AZ::Component>()
			->Version(1)
			;
	}
	if (auto netContext = azrtti_cast<AzFramework::NetworkContext*>(reflection))
	{
	    netContext->Class<GamePlayerProxyComponent>()
			->Chunk<GamePlayerProxyReplicaChunk>()
			->RPC<GamePlayerProxyReplicaChunk, GamePlayerProxyComponent>("Client Event", &GamePlayerProxyReplicaChunk::m_clientEvent)
	        // ->Field("ParticleEffect", &GamePlayerProxyReplicaChunk::m_particleEffect)
	        ;
	}
}

void GamePlayerProxyComponent::Init()
{
}

void GamePlayerProxyComponent::Activate()
{
	GamePlayerProxyRequestBus::Handler::BusConnect(GetEntityId());
}

void GamePlayerProxyComponent::Deactivate()
{
	GamePlayerProxyRequestBus::Handler::BusDisconnect();
}

GridMate::ReplicaChunkPtr GamePlayerProxyComponent::GetNetworkBinding()
{
	AZ_Printf(0, "GamePlayerProxyComponent::GetNetworkBinding");
	m_replicaChunk = GridMate::CreateReplicaChunk<GamePlayerProxyReplicaChunk>();
	m_replicaChunk->SetHandler(this);
	return m_replicaChunk;
}

void GamePlayerProxyComponent::SetNetworkBinding(GridMate::ReplicaChunkPtr chunk)
{
	AZ_Printf(0, "GamePlayerProxyComponent::SetNetworkBinding");

	m_replicaChunk = chunk;
	m_replicaChunk->SetHandler(this);
}

void GamePlayerProxyComponent::UnbindFromNetwork()
{
	AZ_Printf(0, "GamePlayerProxyComponent::UnbindFromNetwork");
	if (m_replicaChunk)
	{
		m_replicaChunk->SetHandler(nullptr);
		m_replicaChunk = nullptr;
	}
}

bool GamePlayerProxyComponent::OnClientEvent (AZStd::string name, AZStd::string data, AZ::EntityId entity, const GridMate::RpcContext &context)
{
	EBUS_EVENT_ID(m_playerEntity, GamePlayerNotificationBus, OnClientEvent, name, data, entity);
	return false;
}

void GamePlayerProxyComponent::SetPlayerEntity(const AZ::EntityId &playerId)
{
	m_playerEntity = playerId;
}

void GamePlayerProxyComponent::SendClientEvent (AZStd::string name, AZStd::string data, AZ::EntityId entity)
{
	// this object is owned by the client
	// only send the event if we're *not* owned by the client
	// also only send the event if we do own the player, meaning we are the server
	if (AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()) || !AzFramework::NetQuery::IsEntityAuthoritative(m_playerEntity))
	{
		return;
	}

	if (m_replicaChunk)
	{
		auto replicaChunk = static_cast<GamePlayerProxyReplicaChunk*>(m_replicaChunk.get());
		replicaChunk->m_clientEvent(name, data, entity);
	}
	else
	{
		// in case we're in-client host
		EBUS_EVENT_ID(GetEntityId(), GamePlayerNotificationBus, OnClientEvent, name, data, entity);
	}
}
