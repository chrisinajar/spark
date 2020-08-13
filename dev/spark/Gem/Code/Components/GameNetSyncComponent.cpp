#include "spark_precompiled.h"

#include <AzFramework/Network/NetworkContext.h>
#include <AzFramework/Network/NetBindingComponent.h>
#include <AzFramework/Network/NetBindingHandlerBus.h>
#include <GridMate/Replica/ReplicaFunctions.h>

#include <GridMate/Replica/ReplicaChunk.h>
#include <GridMate/Replica/DataSet.h>
#include <GridMate/Serialize/CompressionMarshal.h>
#include <INetwork.h>
#include <ISystem.h>
#include <IConsole.h>

#include <Multiplayer/MultiplayerUtils.h>
#include <Multiplayer/IMultiplayerGem.h>
#include <GameLift/GameLiftBus.h>
#include <GameLift/Session/GameLiftServerService.h>

#include "GameNetSyncComponent.h"
#include "GamePlayerComponent.h"
#include "GamePlayerProxyComponent.h"

#include "Busses/UnitBus.h"
#include "Busses/GameManagerBus.h"

#include "Source/sparkCVars.h"
#include "Utils/Marshaler.h"

#if defined(DEDICATED_SERVER)
#include "equ8/sdk/cxx_bindings/equ8_session_manager/session_manager.h"
#include "equ8/sdk/cxx_bindings/equ8_session_manager/equ8_actions.h"
#endif

using namespace spark;
using namespace GridMate;

class GameNetSyncNotificationBusHandler
	: public GameNetSyncNotificationBus::Handler
	, public AZ::BehaviorEBusHandler
{
public:
	AZ_EBUS_BEHAVIOR_BINDER(GameNetSyncNotificationBusHandler, "{80AB9D40-E294-4427-BB57-0739CC8673A9}", AZ::SystemAllocator,
		OnUIEvent);

	void OnUIEvent(AZ::EntityId playerId, AZStd::string eventName, AZStd::string eventValue)
	{
		Call(FN_OnUIEvent, playerId, eventName, eventValue);
	}
};

class GameNetSyncComponentChunk
	: public GridMate::ReplicaChunkBase
{
public:
	AZ_CLASS_ALLOCATOR(GameNetSyncComponentChunk, AZ::SystemAllocator, 0);

	GameNetSyncComponentChunk() 
		: m_newOrder("NewOrder")
		, m_playersBySteamID("Players By Steam ID")
	{
	}

	bool IsReplicaMigratable() override
	{
		return false;
	}

	static const char* GetChunkName()
	{
		return "GameNetSyncComponentChunk";
	}

	GridMate::Rpc<RpcArg<AZ::EntityId>, RpcArg<UnitOrder>, RpcArg<bool>>::BindInterface<GameNetSyncComponent, &GameNetSyncComponent::OnNewOrder> m_newOrder = { "New Order" };
	GridMate::Rpc<RpcArg<const AZStd::string&>, RpcArg<const AZStd::string&>>::BindInterface<GameNetSyncComponent, &GameNetSyncComponent::OnSendUIEvent> m_sendUIEvent = { "UI Event" };
	GridMate::Rpc
		< RpcArg<AZ::EntityId>
		, RpcArg<AZ::EntityId>
		, RpcArg<const AZStd::vector<char>&>
		, RpcArg<const uint64_t&>
		>::BindInterface<GameNetSyncComponent, &GameNetSyncComponent::OnPlayerReady> m_playerReady = { "Player Ready" };

	GridMate::DataSet<AZStd::unordered_map<CSteamID, AZ::EntityId>, GridMate::ContainerMarshaler<AZStd::unordered_map<CSteamID, AZ::EntityId>>>::BindInterface<GameNetSyncComponent, &GameNetSyncComponent::OnNewPlayersBySteamID>				m_playersBySteamID;
};


void GameNetSyncComponent::Reflect(AZ::ReflectContext* reflection)
{
	if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
	{
		serializationContext->Class<GameNetSyncComponent, AZ::Component, AzFramework::NetBindable>()
			->Version(1);

		if (auto editContext = serializationContext->GetEditContext())
		{
			editContext->Class<GameNetSyncComponent>("GameNetSyncComponent", "sync the game events")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
				->Attribute(AZ::Edit::Attributes::Category, "spark")
				->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"));
		}
	}

	if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
	{
		behaviorContext->EBus<GameNetSyncRequestBus>("GameNetSyncRequestBus")
			->Event("SendNewOrder", &GameNetSyncRequestBus::Events::SendNewOrder)
			->Event("SendUIEvent", &GameNetSyncRequestBus::Events::SendUIEvent)
			->Event("GetLocalPlayer", &GameNetSyncRequestBus::Events::GetLocalPlayer)
			->Event("GetPlayerList", &GameNetSyncRequestBus::Events::GetPlayerList)
			->Event("CreateFakePlayer", &GameNetSyncRequestBus::Events::CreateFakePlayer)
			;

		behaviorContext->EBus<GameNetSyncNotificationBus>("GameNetSyncNotificationBus")
			->Handler<GameNetSyncNotificationBusHandler>()
			->Event("OnUIEvent", &GameNetSyncNotificationBus::Events::OnUIEvent)
			;
	}

	if (AzFramework::NetworkContext* netContext = azrtti_cast<AzFramework::NetworkContext*>(reflection))
	{
		////register the chunk type
		//auto &descTable = GridMate::ReplicaChunkDescriptorTable::Get();
		//if (!descTable.FindReplicaChunkDescriptor(GridMate::ReplicaChunkClassId(GameNetSyncComponentChunk::GetChunkName())))
		//{
		//	descTable.RegisterChunkType<GameNetSyncComponentChunk>();
		//}	


	//	netContext->Class<GameNetSyncComponent>()
	//		->Chunk<GameNetSyncComponentChunk,GameNetSyncComponentChunk::Descriptor>()
	//		->RPC<GameNetSyncComponentChunk, GameNetSyncComponent>("NewOrder", &GameNetSyncComponentChunk::m_newOrder)
	//		;

		netContext->Class<GameNetSyncComponent>()
			->Chunk<GameNetSyncComponentChunk>()
			->RPC<GameNetSyncComponentChunk, GameNetSyncComponent>("NewOrder", &GameNetSyncComponentChunk::m_newOrder)
			->RPC<GameNetSyncComponentChunk, GameNetSyncComponent>("SendUIEvent", &GameNetSyncComponentChunk::m_sendUIEvent)
			->RPC<GameNetSyncComponentChunk, GameNetSyncComponent>("Player Ready", &GameNetSyncComponentChunk::m_playerReady)
			->Field("Players By Steam ID", &GameNetSyncComponentChunk::m_playersBySteamID)
			//->Field("PlayerEntityId", &ShipComponentReplicaChunk::m_playerEntityId)
			;
	}



}


void GameNetSyncComponent::Init()
{

}
void GameNetSyncComponent::Activate()
{
    m_cvars.RegisterCVars();

	m_isAuthoritative = AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId());

	GameNetSyncRequestBus::Handler::BusConnect();

	GameBus::Handler::BusConnect();
	GamePlayerEventsBus::Handler::BusConnect();

	ISystem* system = nullptr;
	EBUS_EVENT_RESULT(system, CrySystemRequestBus, GetCrySystem);
	if (system)
	{
		SessionEventBus::Handler::BusConnect(system->GetINetwork()->GetGridMate());
	}

#if !defined(DEDICATED_SERVER)
	if (gEnv->IsEditor())
	{
		CreatePlayerEntity(1);
	}
#endif

#if defined(DEDICATED_SERVER)
    // GridMate::GameLiftServerServiceDesc serviceDesc;

    // if (gEnv->pConsole->GetCVar("sv_port"))
    // {
    //     serviceDesc.m_port = gEnv->pConsole->GetCVar("sv_port")->GetIVal();
    // }

    // EBUS_EVENT(GameLift::GameLiftRequestBus, StartServerService, serviceDesc);

 //    GridMate::CarrierDesc carrierDesc;

 //    carrierDesc.m_port = serviceDesc.m_port;
	// carrierDesc.m_driverIsFullPackets = false;
	// carrierDesc.m_driverIsCrossPlatform = true;

	// GridMate::GridSession* session = nullptr;
	// EBUS_EVENT_ID_RESULT(session, m_gridMate, GridMate::GameLiftServerServiceBus, HostSession, sp, carrierDesc);
#endif
}


void GameNetSyncComponent::Deactivate()
{
    m_cvars.UnregisterCVars();

	SessionEventBus::Handler::BusDisconnect();
	GamePlayerEventsBus::Handler::BusDisconnect();
	GameBus::Handler::BusDisconnect();

	GameNetSyncRequestBus::Handler::BusDisconnect();
}

GridMate::ReplicaChunkPtr GameNetSyncComponent::GetNetworkBinding()
{
	AZ_Printf(0, "GameNetSyncComponent::GetNetworkBinding");
	auto replicaChunk = GridMate::CreateReplicaChunk<GameNetSyncComponentChunk>();
	m_replicaChunk = replicaChunk;
	m_replicaChunk->SetHandler(this);

	replicaChunk->m_playersBySteamID.Set(m_playersBySteamID);

	return m_replicaChunk;
}

void GameNetSyncComponent::SetNetworkBinding(GridMate::ReplicaChunkPtr chunk)
{
	AZ_Printf(0, "GameNetSyncComponent::SetNetworkBinding");

	m_replicaChunk = chunk;
	m_replicaChunk->SetHandler(this);

    auto replicaChunk = static_cast<GameNetSyncComponentChunk*>(m_replicaChunk.get());
	SetPlayersBySteamID(replicaChunk->m_playersBySteamID.Get());
}

void GameNetSyncComponent::UnbindFromNetwork()
{
	AZ_Printf(0, "GameNetSyncComponent::UnbindFromNetwork");
	if (m_replicaChunk)
	{
		m_replicaChunk->SetHandler(nullptr);
		m_replicaChunk = nullptr;
	}
}

void GameNetSyncComponent::OnNewPlayersBySteamID (const AZStd::unordered_map<CSteamID, AZ::EntityId> &playersBySteamID, const GridMate::TimeContext& tc)
{
	SetPlayersBySteamID(playersBySteamID);
}

void GameNetSyncComponent::SetPlayersBySteamID (const AZStd::unordered_map<CSteamID, AZ::EntityId> &playersBySteamID)
{
	m_playersBySteamID = playersBySteamID;


	GridMate::GridSession* gridSession = nullptr;
	EBUS_EVENT_RESULT(gridSession, Multiplayer::MultiplayerRequestBus, GetSession);
	if (gridSession == nullptr)
	{
		return;
	}
	auto localPeerId = gridSession->GetMyMember()->GetIdCompact();

    for (const auto& i : m_playersBySteamID)
    {
        if (GetPeerId(i.second) == localPeerId)
        {
        	m_localPlayer = i.second;
        }
    }
}

bool spark::GameNetSyncComponent::OnNewOrder(AZ::EntityId unit, UnitOrder order, bool queue, const GridMate::RpcContext&)
{
	AZ_Printf(0, "GameNetSyncComponent::OnNewOrder");
	EBUS_EVENT_ID(unit, UnitRequestBus, UnitRequests::NewOrder, order, queue);

	// orders only *actually* run on the server side
	// clients just get replicated state changes
	return false;
}

void spark::GameNetSyncComponent::SendNewOrder(AZ::EntityId unit, UnitOrder order, bool queue)
{


	//if (AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	//{
	//	EBUS_EVENT_ID(unit, UnitRequestBus, UnitRequests::NewOrder, order, queue);
	//	return;
	//}
	AZ_Printf(0, "GameNetSyncComponent::SendNewOrder");
	if (GameNetSyncComponentChunk* chunk = static_cast<GameNetSyncComponentChunk*>(m_replicaChunk.get()))
	{
		AZ_Printf(0, "GameNetSyncComponent::SendNewOrder  chunk is valid");
		chunk->m_newOrder(unit, order, queue);
	}
	else
	{
		EBUS_EVENT_ID(unit, UnitRequestBus, UnitRequests::NewOrder, order, queue);
	}

	//EBUS_EVENT_ID(unit, UnitRequestBus, UnitRequests::NewOrder, order, queue);
}


void GameNetSyncComponent::OnMemberJoined(GridMate::GridSession* session, GridMate::GridMember* member)
{
#if !defined(DEDICATED_SERVER)
	if (m_localPeerId == GridMate::InvalidReplicaPeerId)
	{
		m_localPeerId = session->GetMyMember()->GetIdCompact();
	}
#endif

	auto userId = member->GetIdCompact();
	AZ_Printf(0, "GameNetSyncComponent::OnMemberJoined %u", userId);
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return;
	}

#if defined(DEDICATED_SERVER)
	if (member->IsLocal())
	{
		// do not do any of the player management for the dedicated server "player"
		return;
	}
	// anti-cheat is only enabled in dedicated servers
	auto status = equ8_sm_user_status(AZStd::to_string(userId).c_str());
	switch (status)
	{
		case equ8_action_ban:
		case equ8_action_timeout_1m:
		case equ8_action_timeout_15m:
			session->KickMember(member, status);
			return;
			break;
		default:
			break;
	}

#endif

	CreatePlayerEntity(member->GetIdCompact());
}

void GameNetSyncComponent::OnMemberLeaving(GridMate::GridSession* session, GridMate::GridMember* member)
{
#if !defined(DEDICATED_SERVER)
	if (m_localPeerId == GridMate::InvalidReplicaPeerId)
	{
		m_localPeerId = session->GetMyMember()->GetIdCompact();
	}
#endif
	AZ_Printf(0, "GameNetSyncComponent::OnMemberLeaving");
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return;
	}
	auto peerId = member->GetIdCompact();
	AZ::EntityId playerEntity;
	EBUS_EVENT_ID_RESULT(playerEntity, peerId, GamePlayerPeerRequestBus, GetPlayerEntity);

	if (playerEntity.IsValid())
	{
		EBUS_EVENT_ID(playerEntity, GamePlayerInterfaceBus, SetConnected, false);
	}
}

AZ::EntityId GameNetSyncComponent::CreateFakePlayer ()
{
	AZ_Printf(0, "Creating fake player!");
	auto botId = 2 + m_playersByPeerId.size();
	auto fakePlayer = CreatePlayerEntity(botId);
	m_playersByPeerId[botId] = fakePlayer;

	return fakePlayer;
}

AZ::EntityId GameNetSyncComponent::CreatePlayerEntity (const GridMate::PeerId& peerId)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return AZ::EntityId();
	}
	AZ::Entity* entity = nullptr;
	EBUS_EVENT_RESULT(entity, AzFramework::GameEntityContextRequestBus, CreateGameEntity, "player tracker");
	auto gamePlayer = entity->CreateComponent<GamePlayerComponent>();
	entity->CreateComponent<AzFramework::NetBindingComponent>();
	gamePlayer->SetPeerId(peerId);

	if (peerId == 1)
	{
		m_localPlayer = entity->GetId();
	}

	entity->Activate();

	return entity->GetId();
}

void GameNetSyncComponent::OnSessionCreated(GridMate::GridSession* session)
{
#if !defined(DEDICATED_SERVER)
	if (m_localPeerId == GridMate::InvalidReplicaPeerId)
	{
		m_localPeerId = session->GetMyMember()->GetIdCompact();
	}
#endif
#if defined(DEDICATED_SERVER)
	AZ_Assert(m_gameSession == nullptr, "Game session already exists.");
#endif

	m_gameSession = session;
	AZ_Printf(0, "GameNetSyncComponent::OnSessionCreated");

	// i don't think any of this is needed
	// +map launch params can just set map :thinking:
	// // Synchronize session state and load map if sv_map is passed via parameters
	//static void SynchronizeSessionState(const GridMate::GridSession* session)
	//{
	if (session && session->IsHost())
	{
		GridMate::ReplicationSecurityOptions securityOptions = m_gameSession->GetReplicaMgr()->GetSecurityOptions();
		securityOptions.m_enableStrictSourceValidation = true;
		m_gameSession->GetReplicaMgr()->SetSecurityOptions(securityOptions);

		AZStd::string mapName;

		// Push any session parameters we may have received back to any matching CVars.
		for (unsigned int i = 0; i < session->GetNumParams(); ++i)
		{
			if (strcmp(session->GetParam(i).m_id.c_str(),"sv_map") == 0)
			{
				mapName = session->GetParam(i).m_value.c_str();
			}
			else
			{
				ICVar* var = gEnv->pConsole->GetCVar(session->GetParam(i).m_id.c_str());
				if (var)
				{
					var->Set(session->GetParam(i).m_value.c_str());
				}
				else
				{
					CryLogAlways("Unable to bind session property '%s:%s' to CVar. CVar does not exist.", session->GetParam(i).m_id.c_str(), session->GetParam(i).m_value.c_str());
				}
			}
		}

		if (!mapName.empty())
		{
			// If we have an actual level to load, load it.                    
			AZStd::string loadCommand = "map ";
			loadCommand += mapName;
			gEnv->pConsole->ExecuteString(loadCommand.c_str(), false, true);
		}
#if defined(DEDICATED_SERVER)
		// m_gameSession->GetReplicaMgr()->SetLocalLagAmt(100);

		// When we are a GameLiftServer. We need to synchronize our session state, to whatever the gamelift session is.
		// Otherwise, we can assume we created the session, and are already synchronized.
		if (gEnv->IsDedicated())
		{
			bool isGameLiftServer = false;
			EBUS_EVENT_RESULT(isGameLiftServer, GameLift::GameLiftRequestBus, IsGameLiftServer);

			if (isGameLiftServer)
			{
				Multiplayer::Utils::SynchronizeSessionState(m_gameSession);
			}
		}
#endif
	}
}

void GameNetSyncComponent::OnSessionDelete(GridMate::GridSession* session)
{
	bool reloadMap = true;
	if (m_gameSession)
	{
		if (session == m_gameSession)
		{
			reloadMap = true;
			m_gameSession = nullptr;
		}
		else
		{
			reloadMap = false;
		}
	}
	else
	{
		reloadMap = false;
	}

	//if (reloadMap)
	//{
	//	// session ended -> going back to initial map
	//	if (!gEnv->IsDedicated())
	//	{
	//		gEnv->pConsole->ExecuteString("exec initialmap.cfg", false, true);
	//	}
	//	else
	//	{
	//		gEnv->pConsole->ExecuteString("quit");
	//	}
	//}
}

GridMate::PeerId GameNetSyncComponent::GetLocalPeerId() const
{
	GridMate::GridSession* gridSession = nullptr;
	EBUS_EVENT_RESULT(gridSession, Multiplayer::MultiplayerRequestBus, GetSession);
	if (gridSession == nullptr)
	{
		return 1;
	}
	return gridSession->GetMyMember()->GetIdCompact();
}

GridMate::PeerId GameNetSyncComponent::GetPeerId(AZ::EntityId playerEntityId)
{
	GridMate::PeerId playerId = GridMate::InvalidReplicaPeerId;
	
	EBUS_EVENT_ID_RESULT(playerId, playerEntityId, GamePlayerInterfaceBus, GetPeerId);

	return playerId;
}

void GameNetSyncComponent::OnGamePlayerJoined(AZ::EntityId playerEntityId)
{
	// a game player entity was created
	AZ_Error("GameNetSyncComponent", playerEntityId.IsValid(), "GamePlayer did not provide playerId");
	auto peerId = GetPeerId(playerEntityId);
	AZ_Error("GameNetSyncComponent", peerId != GridMate::InvalidReplicaPeerId, "GamePlayer did not provide playerId");
	AZStd::string name = "";
	EBUS_EVENT_ID_RESULT(name, playerEntityId, GamePlayerInterfaceBus, GetName);

	CSteamID steamID;
	EBUS_EVENT_RESULT(steamID, SteamWorks::SteamWorksRequestBus, GetSteamID);

	AZ_Printf(0, "GameNetSyncComponent::OnGamePlayerJoined %s, users: %d", name.c_str(), m_playersByPeerId.size());

#if !defined(DEDICATED_SERVER)
	if (m_replicaChunk && m_replicaChunk->GetReplica())
	{
		auto myPeerId = GetLocalPeerId();
		AZ_Printf(0, "-------------------------------");
		AZ_Printf(0, "-------------------------------");
		AZ_Printf(0, "Activating player with peerId %u vs my %u", peerId, myPeerId);
		if (peerId == myPeerId)
		{
			AZ_Printf(0, "Found my local player, sending ready signal!");
			m_localPlayer = playerEntityId;
			EBUS_EVENT_ID(playerEntityId, GamePlayerInterfaceBus, SetIsLocalPlayer, true);

			// if this is us then send the ready RPC to the server
			// being able to get all of these values above while on the replica means that all of those values have been replicated
			// that's what the ready signal is waiting for, just the player entities to be fully configured and functional
			if (AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
			{
				m_playersBySteamID[steamID] = playerEntityId;
				OnPlayerReadyImpl(playerEntityId, peerId, steamID);
			}
			else
			{
				// 
				auto chunk = static_cast<GameNetSyncComponentChunk*>(m_replicaChunk.get());
				SteamWorks::AuthTokenResult steamResult;
				EBUS_EVENT_RESULT(steamResult, SteamWorks::SteamWorksRequestBus, GetAuthToken);
				m_localAuthTicket = steamResult.ticket;

				// create client proxy object
				AZ::Entity* entity = nullptr;
				EBUS_EVENT_RESULT(entity, AzFramework::GameEntityContextRequestBus, CreateGameEntity, "player proxy object");
				auto proxy = entity->CreateComponent<GamePlayerProxyComponent>();
				entity->CreateComponent<AzFramework::NetBindingComponent>();
				proxy->SetPlayerEntity(m_localPlayer);

				entity->Activate();

				AZ_Printf(0, "sending ready signal rpc");
				chunk->m_playerReady(playerEntityId, entity->GetId(), steamResult.token, steamID.ConvertToUint64());
			}
		}
		else
#endif
		if (peerId > 2 && peerId < 12)
		{
			// it's a bot! also on player ready chceks if it's auth so don't worry about it here
			AZ_Printf(0, "Forcing bot to be ready");
			auto steamId = CSteamID(peerId, k_EUniverseDev, k_EAccountTypeIndividual);
			m_playersBySteamID[steamId] = playerEntityId;
			OnPlayerReadyImpl(playerEntityId, peerId, steamId);
		}
#if !defined(DEDICATED_SERVER)
		AZ_Printf(0, "-------------------------------");
		AZ_Printf(0, "-------------------------------");
	}
	// else if (gEnv->IsEditor())
	// not on network but the player component was created and we're authoritative (but not on the network)
	else if (AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		// there should have already been a local player object created with peer id 1 for this user
		// try telling it that it is local...
		EBUS_EVENT_ID(m_localPlayer, GamePlayerInterfaceBus, SetIsLocalPlayer, true);

		AZ_Printf(0, "Forcing ready because we're local");
		m_playersBySteamID[steamID] = playerEntityId;
		OnPlayerReadyImpl(playerEntityId, peerId, steamID);
	}
#endif
}

void GameNetSyncComponent::OnGamePlayerLeft(AZ::EntityId playerEntityId)
{
	// this doesn't happen anymore because we don't delete player entities when they disconnect anymore
	// we keep the entity around to track their shit while they're disconnected and instead maintain a connected state on it
	AZ_Printf(0, "GameNetSyncComponent::OnGamePlayerLeft - game player was destroyed?");
	AZ_Error("GameNetSyncComponent", playerEntityId.IsValid(), "GamePlayer did not provide playerId");
	auto peerId = GetPeerId(playerEntityId);
	AZ_Error("GameNetSyncComponent", peerId != GridMate::InvalidReplicaPeerId, "GamePlayer did not provide peer id");
	AZ_Error("GameNetSyncComponent", m_playersByPeerId.find(peerId) != m_playersByPeerId.end(), "Unknown peer id");
	m_playersByPeerId.erase(peerId);
	AZ_Printf(0, "GameNetSyncComponent::OnGamePlayerLeft, users: %d", m_playersByPeerId.size());

	CSteamID steamId;
	EBUS_EVENT_ID_RESULT(steamId, playerEntityId, GamePlayerInterfaceBus, GetSteamId);
	auto it = m_playersBySteamID.find(steamId);
	if (it != m_playersBySteamID.end() && it->second == playerEntityId)
	{
		m_playersBySteamID.erase(steamId);

		if (m_replicaChunk && m_isAuthoritative)
		{
			auto replicaChunk = static_cast<GameNetSyncComponentChunk*>(m_replicaChunk.get());
			replicaChunk->m_playersBySteamID.Set(m_playersBySteamID);
		}
	}
}

bool GameNetSyncComponent::OnPlayerReady (AZ::EntityId playerEntityId, AZ::EntityId proxyEntityId, const AZStd::vector<char>& token, const uint64_t &steamIdUint, const GridMate::RpcContext &context)
{
	AZ_Error("GameNetSyncComponent", playerEntityId.IsValid(), "GamePlayer did not provide playerId");
	auto peerId = GetPeerId(playerEntityId);
	AZ_Error("GameNetSyncComponent", peerId != GridMate::InvalidReplicaPeerId, "GamePlayer did not provide playerId");
	auto rpcPeerId = context.m_sourcePeer;
	AZ_Printf(0, "This peer connected claiming this entity %u vs %u", peerId, rpcPeerId);
	AZ_Error("GameNetSyncComponent", peerId == rpcPeerId, "PeerIDs are inconsistent between entity and rpc context");

	CSteamID steamId(steamIdUint);
	EBeginAuthSessionResult steamResult;
	EBUS_EVENT_RESULT(steamResult, SteamWorks::SteamWorksRequestBus, ConfirmAuthToken, token, steamId);

	if (steamResult != k_EBeginAuthSessionResultOK)
	{
		AZ_Error("GameNetSyncComponent", steamResult == k_EBeginAuthSessionResultOK, "Failed to authenticate user!");
		GridMate::GridSession* gridSession = nullptr;
		EBUS_EVENT_RESULT(gridSession, Multiplayer::MultiplayerRequestBus, GetSession);

		bool isGameLiftServer = false;
		EBUS_EVENT_RESULT(isGameLiftServer, GameLift::GameLiftRequestBus, IsGameLiftServer);

		if (gridSession && isGameLiftServer)
		{
			gridSession->KickMember(gridSession->GetMemberById(peerId));
			EBUS_EVENT(AzFramework::GameEntityContextRequestBus, DestroyGameEntity, playerEntityId);
		}
	}
	else
	{
		AZ_Printf(0, "GameNetSyncComponent::OnPlayerReady, player is ready!!! Woohoo!!!");

		if (m_playersBySteamID.find(steamId) == m_playersBySteamID.end())
		{
			m_playersBySteamID[steamId] = playerEntityId;
			OnPlayerReadyImpl(playerEntityId, peerId, steamId);
		}
		else
		{
			EBUS_EVENT(AzFramework::GameEntityContextRequestBus, DestroyGameEntity, playerEntityId);
			playerEntityId = m_playersBySteamID[steamId];
			auto oldPeerId = GetPeerId(playerEntityId);
			m_playersByPeerId.erase(oldPeerId);
			EBUS_EVENT_ID(playerEntityId, GamePlayerInterfaceBus, SetPeerId, peerId);
			m_playersByPeerId[peerId] = playerEntityId;
			EBUS_EVENT_ID(playerEntityId, GamePlayerInterfaceBus, SetConnected, true);
			AZ_Printf(0, "Player has reconnected probably maybe!");
			if (m_replicaChunk && m_isAuthoritative)
			{
    			auto replicaChunk = static_cast<GameNetSyncComponentChunk*>(m_replicaChunk.get());
    			replicaChunk->m_playersBySteamID.Set(m_playersBySteamID);
			}
		}

		EBUS_EVENT_ID(playerEntityId, GamePlayerInterfaceBus, SetClientProxy, proxyEntityId);
	}

	return false;
}

void GameNetSyncComponent::OnPlayerReadyImpl(const AZ::EntityId &playerEntityId, const GridMate::PeerId &peerId, const CSteamID &steamId)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		return;
	}
	// add player by peer and set the default team
	m_playersByPeerId[peerId] = playerEntityId;
	if (AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		EBUS_EVENT_ID(playerEntityId, GamePlayerInterfaceBus, SetTeamId, (m_playersByPeerId.size() % 2) + 1);
	}

	EBUS_EVENT_ID(playerEntityId, GamePlayerInterfaceBus, SetPlayerId, m_playersByPeerId.size());
	EBUS_EVENT_ID(playerEntityId, GamePlayerInterfaceBus, SetSteamId, steamId);
	EBUS_EVENT_ID(playerEntityId, GamePlayerInterfaceBus, SetConnected, true);

	GamePhase::Type gamePhase = GamePhase::INIT;
	EBUS_EVENT_RESULT(gamePhase, GameManagerRequestBus, GetGamePhase);

	AZ_Printf(0, "Added player ID %d to the game!", m_playersByPeerId.size());

	if (gamePhase == GamePhase::WAITING_FOR_PLAYERS && m_playersByPeerId.size() >= 0)
	{
		AZ_Printf(0, "All %d players are connected, moving on to configure phase...", m_playersByPeerId.size());
		EBUS_EVENT(GameManagerRequestBus, SetGamePhase, GamePhase::CONFIGURE_GAME);
	}

	if (m_replicaChunk && m_isAuthoritative)
	{
		auto replicaChunk = static_cast<GameNetSyncComponentChunk*>(m_replicaChunk.get());
		replicaChunk->m_playersBySteamID.Set(m_playersBySteamID);
	}
}

void GameNetSyncComponent::LeaveSession()
{
	if (m_gameSession)
	{
		const bool migrateHost = false;
		m_gameSession->Leave(migrateHost);
	}
}

void GameNetSyncComponent::SendUIEvent (const AZStd::string &name, const AZStd::string &value)
{
	if (!m_replicaChunk)
	{
		AZ::EntityId playerEntityId;
		EBUS_EVENT_ID_RESULT(playerEntityId, 1, GamePlayerPeerRequestBus, GetPlayerEntity);
		SendUIEventImpl(playerEntityId, name, value);
		return;
	}
	else if (m_isAuthoritative)
	{
		auto myPeerId = GetLocalPeerId();
		AZ::EntityId playerEntityId;
		EBUS_EVENT_ID_RESULT(playerEntityId, myPeerId, GamePlayerPeerRequestBus, GetPlayerEntity);
		SendUIEventImpl(playerEntityId, name, value);
		return;
	}

	GameNetSyncComponentChunk* chunk = static_cast<GameNetSyncComponentChunk*>(m_replicaChunk.get());
	chunk->m_sendUIEvent(name, value);
}

bool GameNetSyncComponent::OnSendUIEvent (const AZStd::string &name, const AZStd::string &value, const GridMate::RpcContext &context)
{
	auto rpcPeerId = context.m_sourcePeer;
	AZ::EntityId playerEntityId;
	EBUS_EVENT_ID_RESULT(playerEntityId, context.m_sourcePeer, GamePlayerPeerRequestBus, GetPlayerEntity);

	SendUIEventImpl(playerEntityId, name, value);
	return false;
}

void GameNetSyncComponent::SendUIEventImpl (const AZ::EntityId playerEntityId, const AZStd::string &name, const AZStd::string &value)
{
	if (!m_isAuthoritative)
	{
		return;
	}

	EBUS_EVENT(GameNetSyncNotificationBus, OnUIEvent, playerEntityId, name, value);
}

AZ::EntityId GameNetSyncComponent::GetLocalPlayer()
{
	return m_localPlayer;
}

AZStd::vector<AZ::EntityId> GameNetSyncComponent::GetPlayerList ()
{
	AZStd::vector<AZ::EntityId> result;

    for (const auto& i : m_playersBySteamID)
    {
        result.push_back(i.second);
    }

	return result;
}

void GameNetSyncComponent::SetPlayerSteamID (const AZ::EntityId playerEntityId, const CSteamID &steamId)
{
	if (m_isAuthoritative)
	{
		return;
	}

	m_playersBySteamID[steamId] = playerEntityId;


	if (m_replicaChunk && m_isAuthoritative)
	{
		auto replicaChunk = static_cast<GameNetSyncComponentChunk*>(m_replicaChunk.get());
		replicaChunk->m_playersBySteamID.Set(m_playersBySteamID);
	}
}
