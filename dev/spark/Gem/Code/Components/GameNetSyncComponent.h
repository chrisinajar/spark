#pragma once

#include <AzCore/Component/Component.h>
#include <GridMate/Replica/ReplicaChunk.h>
#include <AzFramework/Network/NetBindable.h>
#include <GridMate/Session/Session.h>

#include <AzCore/Component/TransformBus.h>
#include <SteamWorks/SteamWorksBus.h>

#include "Busses/NetSyncBusses.h"
#include "Busses/GamePlayerBus.h"

#include "sparkCVars.h"

namespace spark
{
	class GameNetSyncComponent
		: public AZ::Component
		, protected GameNetSyncRequestBus::Handler
		, private GamePlayerEventsBus::Handler
		, private GameBus::Handler
		, private GridMate::SessionEventBus::Handler
		, public AzFramework::NetBindable
	{
	public:
		AZ_COMPONENT(GameNetSyncComponent, "{F0CA9CF1-556B-42EF-A9CF-A36A451CEEB8}", AzFramework::NetBindable);

		static void Reflect(AZ::ReflectContext* context);

		void Init() override;
		void Activate() override;
		void Deactivate() override;

		GridMate::ReplicaChunkPtr GetNetworkBinding();
		void SetNetworkBinding(GridMate::ReplicaChunkPtr chunk);
		void UnbindFromNetwork();

		// Session events bus
		void OnSessionCreated(GridMate::GridSession* session) override;
		void OnSessionDelete(GridMate::GridSession* session) override;
		void OnMemberJoined(GridMate::GridSession* session, GridMate::GridMember* member) override;
		void OnMemberLeaving(GridMate::GridSession* session, GridMate::GridMember* member) override;

		// Player id bus
		GridMate::PeerId GetPeerId(AZ::EntityId playerEntityId);

		AZStd::vector<AZ::EntityId> GetPlayerList () override;

		// Game player bus
		void OnGamePlayerJoined(AZ::EntityId entityId) override;
		void OnGamePlayerLeft(AZ::EntityId entityId) override;

		// GameBus
		void LeaveSession() override;

		//rpc actions
		bool OnNewOrder(AZ::EntityId unit,UnitOrder, bool queue,const GridMate::RpcContext&);
		bool OnSendUIEvent (const AZStd::string &name, const AZStd::string &value, const GridMate::RpcContext&);
		bool OnPlayerReady (AZ::EntityId playerEntityId, AZ::EntityId playerProxy, const AZStd::vector<char>& token, const uint64_t&, const GridMate::RpcContext&);
		void OnNewPlayersBySteamID (const AZStd::unordered_map<CSteamID, AZ::EntityId> &playersBySteamID, const GridMate::TimeContext& tc);
	private:
		void SetPlayersBySteamID (const AZStd::unordered_map<CSteamID, AZ::EntityId> &playersBySteamID);
		GridMate::PeerId GetLocalPeerId() const;
		void OnPlayerReadyImpl(const AZ::EntityId &playerEntityId, const GridMate::PeerId &playerId, const CSteamID &steamId);
		// ebus handlers
		void SendNewOrder(AZ::EntityId unit, UnitOrder, bool queue) override;
		void SendUIEvent (const AZStd::string &eventName, const AZStd::string &eventValue) override;
		AZ::EntityId GetLocalPlayer() override;
		AZ::EntityId CreateFakePlayer () override;
		void SetPlayerSteamID (const AZ::EntityId playerEntityId, const CSteamID &name) override;

		AZ::EntityId CreatePlayerEntity (const GridMate::PeerId& playerId);
		void SendUIEventImpl (const AZ::EntityId playerEntityId, const AZStd::string &name, const AZStd::string &value);

		bool m_isAuthoritative = false;
		GridMate::ReplicaChunkPtr m_replicaChunk;

		// players that are currently connected
		AZStd::unordered_map<GridMate::PeerId, AZ::EntityId> m_playersByPeerId;
		AZStd::unordered_map<CSteamID, AZ::EntityId> m_playersBySteamID;

		GridMate::GridSession* m_gameSession = nullptr;
		AZ::EntityId m_localPlayer;
		GridMate::PeerId m_localPeerId = GridMate::InvalidReplicaPeerId;
		HAuthTicket m_localAuthTicket;
		SparkCVars m_cvars;
	};
}
