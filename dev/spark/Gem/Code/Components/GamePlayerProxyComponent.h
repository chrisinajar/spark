#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/EntityBus.h>
#include <AZCore/Component/TickBus.h>

#include <AzFramework/Network/NetBindable.h>

#include "Busses/GamePlayerBus.h"
#include <AzCore/RTTI/TypeInfo.h>
#include <AzCore/Component/EntityId.h>

namespace spark
{
	// GamePlayerProxyComponent
	// Client side component proxied onto the server for server->client RPC
	class GamePlayerProxyComponent
		: public AZ::Component
		, public AzFramework::NetBindable
		, public GamePlayerProxyRequestBus::Handler
	{
		friend class GamePlayerComponent;
		friend class GameNetSyncComponent;
	public:

		AZ_COMPONENT(GamePlayerProxyComponent, "{34EFC082-B0F8-44F4-939A-BF67F73E2430}", AzFramework::NetBindable);
		static void Reflect(AZ::ReflectContext* context);

		// AZ::Component
		void Init() override;
		void Activate() override;
		void Deactivate() override;

		GridMate::ReplicaChunkPtr GetNetworkBinding();
		void SetNetworkBinding(GridMate::ReplicaChunkPtr chunk);
		void UnbindFromNetwork();

		void SendClientEvent (AZStd::string name, AZStd::string data, AZ::EntityId entity) override;
		
		bool OnClientEvent (AZStd::string name, AZStd::string data, AZ::EntityId entity, const GridMate::RpcContext &context);

	protected:
		void SetPlayerEntity(const AZ::EntityId &playerId);


		AZ::EntityId m_playerEntity;
		GridMate::ReplicaChunkPtr m_replicaChunk;
	};
}
