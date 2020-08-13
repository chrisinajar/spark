#include "spark_precompiled.h"
/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#include "Components/GamePlayerComponent.h"

#include <AzCore/Serialization/ObjectStream.h>
#include <AzCore/std/string/conversions.h>
#include <AzFramework/Entity/EntityContextBus.h>
#include <AzCore\RTTI\BehaviorContext.h>

#include <AzFramework/Network/NetworkContext.h>
#include <GridMate/Replica/ReplicaChunk.h>
#include <GridMate/Replica/ReplicaFunctions.h>
#include "Utils/Marshaler.h"
// NetQuery
#include <AzFramework/Network/NetBindingHandlerBus.h>

//#include "CVars/MultiplayerSampleCVars.h"

#include "Utils/GridMate.h"
#include "Busses/GameManagerBus.h"
#include "Busses/NetSyncBusses.h"

//#include "MultiplayerSample/GamePlayerBus.h"

namespace spark
{
	// lua handlers

	class GamePlayerEventsBusHandler
		: public GamePlayerEventsBus::Handler
		, public AZ::BehaviorEBusHandler
	{
	public:
		AZ_EBUS_BEHAVIOR_BINDER(GamePlayerEventsBusHandler, "{D7C6A689-7496-48B8-B453-332C0FA03A7C}", AZ::SystemAllocator,
			OnGamePlayerJoined, OnGamePlayerLeft);

		void OnGamePlayerJoined(AZ::EntityId value)
		{
			Call(FN_OnGamePlayerJoined, value);
		}
		void OnGamePlayerLeft(AZ::EntityId value)
		{
			Call(FN_OnGamePlayerLeft, value);
		}
	};

	class GamePlayerNotificationBusHandler
		: public GamePlayerNotificationBus::Handler
		, public AZ::BehaviorEBusHandler
	{
	public:
		AZ_EBUS_BEHAVIOR_BINDER(GamePlayerNotificationBusHandler, "{D222AF3D-58E0-4C4F-AF23-9F1BF180772F}", AZ::SystemAllocator,
			OnHeroSelected, OnHeroSpawned, OnClientEvent);
		void OnHeroSelected(AZStd::string value)
		{
			Call(FN_OnHeroSelected, value);
		}
		void OnHeroSpawned(AZ::EntityId value)
		{
			Call(FN_OnHeroSpawned, value);
		}
		void OnClientEvent(AZStd::string name, AZStd::string data, AZ::EntityId value)
		{
			Call(FN_OnClientEvent, name, data, value);
		}
	};


	////////////////////
	// GamePlayerReplicaChunk
	////////////////////

	class GamePlayerReplicaChunk
		: public GridMate::ReplicaChunkBase
	{
		friend class GamePlayerComponent;

	public:
		AZ_CLASS_ALLOCATOR(GamePlayerReplicaChunk, AZ::SystemAllocator, 0);

		static const char* GetChunkName() { return "GamePlayerReplicaChunk"; }

		GamePlayerReplicaChunk()
			: m_name("Name")
			, m_playerId("PlayerId")
			, m_peerId("PeerId")
			, m_steamId("SteamId")
			, m_selectedHeroName("SelectedHeroName")
			, m_selectedHeroEntity("SelectedHeroEntity")
			, m_teamId("TeamId")
			, m_connected("Connected")
		{
		}

		bool IsReplicaMigratable() override
		{
			return false;
		}

	protected:
		GridMate::DataSet<AZStd::string>::BindInterface<GamePlayerComponent, &GamePlayerComponent::OnNewPlayerName>		m_name;
		GridMate::DataSet<AZStd::string>::BindInterface<GamePlayerComponent, &GamePlayerComponent::OnNewSelectedHero>	m_selectedHeroName;
		GridMate::DataSet<UnitId>::BindInterface<GamePlayerComponent, &GamePlayerComponent::OnNewSelectedHeroEntity>	m_selectedHeroEntity;
		GridMate::DataSet<SparkPlayerId>::BindInterface<GamePlayerComponent, &GamePlayerComponent::OnNewPlayerId>		m_playerId;
		GridMate::DataSet<GridMate::PeerId>::BindInterface<GamePlayerComponent, &GamePlayerComponent::OnNewPeerId>		m_peerId;
		GridMate::DataSet<CSteamID>::BindInterface<GamePlayerComponent, &GamePlayerComponent::OnNewSteamId>				m_steamId;
		GridMate::DataSet<TeamId>::BindInterface<GamePlayerComponent, &GamePlayerComponent::OnNewTeamId>				m_teamId;
		GridMate::DataSet<bool>::BindInterface<GamePlayerComponent, &GamePlayerComponent::OnNewConnected>				m_connected;
	};

	////////////////////////
	// GamePlayerComponent
	////////////////////////

	//void GamePlayerComponent::OnSpawnTagChanged(ICVar* cvar)
	//{
	//    const char* spawnTag = cvar->GetString();
	//    EBUS_EVENT(GamePlayerInterfaceBus, SetSpawnTag, spawnTag);
	//}

	//void GamePlayerComponent::OnUserNameChanged(ICVar* cvar)
	//{
	//    const char* playerName = cvar->GetString();
	//    EBUS_EVENT(GamePlayerInterfaceBus, SetName, playerName);
	//}

	void GamePlayerComponent::Reflect(AZ::ReflectContext* context)
	{
		AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
		if (serializeContext)
		{
			serializeContext->Class<GamePlayerComponent, AZ::Component, NetBindable>()
				->Version(1)
				->Field("Name", &GamePlayerComponent::m_name)
				->Field("SelectedHeroName", &GamePlayerComponent::m_selectedHeroName)
				->Field("PlayerId", &GamePlayerComponent::m_playerId)
				->Field("PeerId", &GamePlayerComponent::m_peerId)
				->Field("SteamId", &GamePlayerComponent::m_steamId)
				->Field("TeamId", &GamePlayerComponent::m_teamId)
				->Field("Connected", &GamePlayerComponent::m_connected)
			;
		}

		if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
		{
			behaviorContext->EBus<GamePlayerInterfaceBus>("GamePlayerInterfaceBus")
				->Event("SetName", &GamePlayerInterfaceBus::Events::SetName)
				->Event("GetName", &GamePlayerInterfaceBus::Events::GetName)
				->Event("GetPlayerId", &GamePlayerInterfaceBus::Events::GetPlayerId)
				->Event("GetPeerId", &GamePlayerInterfaceBus::Events::GetPeerId)
				// ->Event("GetSteamId", &GamePlayerInterfaceBus::Events::GetSteamId) -- CSteamID doesn't work right now
				->Event("GetSelectedHeroEntity", &GamePlayerInterfaceBus::Events::GetSelectedHeroEntity)
				->Event("GetSelectedHero", &GamePlayerInterfaceBus::Events::GetSelectedHero)
				->Event("SetSelectedHero", &GamePlayerInterfaceBus::Events::SetSelectedHero)
				->Event("SpawnHero", &GamePlayerInterfaceBus::Events::SpawnHero)
				->Event("GetTeamId", &GamePlayerInterfaceBus::Events::GetTeamId)
				->Event("SendClientEvent", &GamePlayerInterfaceBus::Events::SendClientEvent)
			;

			behaviorContext->EBus<GamePlayerEventsBus>("GamePlayerEventsBus")
				->Handler<GamePlayerEventsBusHandler>()
				->Event("OnGamePlayerJoined", &GamePlayerEventsBus::Events::OnGamePlayerJoined)
				->Event("OnGamePlayerLeft", &GamePlayerEventsBus::Events::OnGamePlayerLeft)
			;

			behaviorContext->EBus<GamePlayerNotificationBus>("GamePlayerNotificationBus")
				->Handler<GamePlayerNotificationBusHandler>()
				->Event("OnHeroSelected", &GamePlayerNotificationBus::Events::OnHeroSelected)
				->Event("OnHeroSpawned", &GamePlayerNotificationBus::Events::OnHeroSpawned)
				->Event("OnClientEvent", &GamePlayerNotificationBus::Events::OnClientEvent)
			;

			behaviorContext->EBus<GamePlayerIdRequestBus>("GamePlayerIdRequestBus")
				->Event("GetPlayerEntity", &GamePlayerIdRequestBus::Events::GetPlayerEntity)
			;
		}

		AzFramework::NetworkContext* netContext = azrtti_cast<AzFramework::NetworkContext*>(context);
		if (netContext)
		{
			netContext->Class<GamePlayerComponent>()
				->Chunk<GamePlayerReplicaChunk>()
				->Field("Name", &GamePlayerReplicaChunk::m_name)
				->Field("SelectedHeroName", &GamePlayerReplicaChunk::m_selectedHeroName)
				->Field("SelectedHeroEntity", &GamePlayerReplicaChunk::m_selectedHeroEntity)
				->Field("PlayerId", &GamePlayerReplicaChunk::m_playerId)
				->Field("PeerId", &GamePlayerReplicaChunk::m_peerId)
				->Field("SteamId", &GamePlayerReplicaChunk::m_steamId)
				->Field("TeamId", &GamePlayerReplicaChunk::m_teamId)
				->Field("Connected", &GamePlayerReplicaChunk::m_connected)
			;
		}
	}

	void GamePlayerComponent::Init()
	{
	}

	void GamePlayerComponent::Activate()
	{
		GamePlayerInterfaceBus::Handler::BusConnect(GetEntityId());
		AZ::TickBus::Handler::BusConnect();
	}

	void GamePlayerComponent::Deactivate()
	{
		EBUS_EVENT(GamePlayerEventsBus, OnGamePlayerLeft, GetEntityId());
		GamePlayerInterfaceBus::Handler::BusDisconnect();
		AZ::TickBus::Handler::BusDisconnect();
		GamePlayerPeerRequestBus::Handler::BusDisconnect();
	}

	void GamePlayerComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time)
	{
		if (m_peerId != GridMate::InvalidReplicaPeerId)
		{
			AZ::TickBus::Handler::BusDisconnect();
			EBUS_EVENT(GamePlayerEventsBus, OnGamePlayerJoined, GetEntityId());
		}
		else
		{
			AZ_Printf(0, "GamePlayerComponent still invalid")
		}
	}

	void GamePlayerComponent::SetName (const AZStd::string &name)
	{
		SPARK_UPDATE_VALUE(GamePlayer, name);
	}

	AZStd::string GamePlayerComponent::GetName() const
	{
		return m_name.c_str();
	}

	void GamePlayerComponent::Enumerate(PlayersEnumerateCallback callback)
	{
		callback(GetEntityId());
	}

	GridMate::ReplicaChunkPtr GamePlayerComponent::GetNetworkBinding()
	{
		GamePlayerReplicaChunk* chunk = GridMate::CreateReplicaChunk<GamePlayerReplicaChunk>();
		chunk->SetHandler(this);
		m_replicaChunk = chunk;

		auto replicaChunk = static_cast<GamePlayerReplicaChunk*>(m_replicaChunk.get());
		replicaChunk->m_playerId.Set(m_playerId);
		replicaChunk->m_peerId.Set(m_peerId);
		replicaChunk->m_steamId.Set(m_steamId);
		replicaChunk->m_name.Set(m_name);
		replicaChunk->m_selectedHeroName.Set(m_selectedHeroName);
		replicaChunk->m_selectedHeroEntity.Set(m_selectedHeroEntity);
		replicaChunk->m_teamId.Set(m_teamId);
		replicaChunk->m_connected.Set(m_connected);

		return m_replicaChunk;
	}

	void GamePlayerComponent::SetNetworkBinding(GridMate::ReplicaChunkPtr chunk)
	{
		AZ_Assert(m_replicaChunk == nullptr, "Being bound to two ReplicaChunks");
		if (chunk)
		{
			chunk->SetHandler(this);
			m_replicaChunk = chunk;

			m_name = static_cast<GamePlayerReplicaChunk*>(m_replicaChunk.get())->m_name.Get().c_str();
			SetPlayerId(static_cast<GamePlayerReplicaChunk*>(m_replicaChunk.get())->m_playerId.Get());
			SetPeerId(static_cast<GamePlayerReplicaChunk*>(m_replicaChunk.get())->m_peerId.Get());
			SetSteamId(static_cast<GamePlayerReplicaChunk*>(m_replicaChunk.get())->m_steamId.Get());
			SetSelectedHero(static_cast<GamePlayerReplicaChunk*>(m_replicaChunk.get())->m_selectedHeroName.Get());
			SetSelectedHeroEntity(static_cast<GamePlayerReplicaChunk*>(m_replicaChunk.get())->m_selectedHeroEntity.Get());
			SetTeamId(static_cast<GamePlayerReplicaChunk*>(m_replicaChunk.get())->m_teamId.Get());
			SetConnected(static_cast<GamePlayerReplicaChunk*>(m_replicaChunk.get())->m_connected.Get());
		}
	}

	void GamePlayerComponent::UnbindFromNetwork()
	{
		m_replicaChunk->SetHandler(nullptr);
		m_replicaChunk = nullptr;
	}

	void GamePlayerComponent::OnNewPlayerName(const AZStd::string& name, const GridMate::TimeContext& tc)
	{
		(void)tc;
		if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
		{
			SetName(name);
		}
	}

	void GamePlayerComponent::OnNewPlayerId(const SparkPlayerId& playerId, const GridMate::TimeContext& tc)
	{
		(void)tc;
		if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
		{
			SetPlayerId(playerId);
		}
	}

	void GamePlayerComponent::OnNewPeerId(const GridMate::PeerId& peerId, const GridMate::TimeContext& tc)
	{
		(void)tc;
		if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
		{
			SetPeerId(peerId);
		}
	}

	void GamePlayerComponent::OnNewSteamId(const CSteamID& steamId, const GridMate::TimeContext& tc)
	{
		(void)tc;
		if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
		{
			SetSteamId(steamId);
		}
	}

	void GamePlayerComponent::OnNewSelectedHero(const AZStd::string& selectedHeroName, const GridMate::TimeContext& tc)
	{
		(void)tc;
		if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
		{
			SetSelectedHero(selectedHeroName);
		}
	}

	void GamePlayerComponent::OnNewSelectedHeroEntity(const AZ::EntityId& selectedHeroEntity, const GridMate::TimeContext& tc)
	{
		(void)tc;
		if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
		{
			SetSelectedHeroEntity(selectedHeroEntity);
		}
	}
	
	void GamePlayerComponent::OnNewTeamId(const TeamId &teamId, const GridMate::TimeContext& tc)
	{
		if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
		{
			SetTeamId(teamId);
		}
	}
	
	void GamePlayerComponent::OnNewConnected(const bool &connected, const GridMate::TimeContext& tc)
	{
		if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
		{
			SetConnected(connected);
		}
	}

	AZStd::string GamePlayerComponent::GetSelectedHero() const
	{
		return m_selectedHeroName;
	}

	UnitId GamePlayerComponent::GetSelectedHeroEntity() const
	{
		return m_selectedHeroEntity;
	}

	void GamePlayerComponent::SetSelectedHeroEntity(AZ::EntityId selectedHeroEntity)
	{
		SPARK_UPDATE_VALUE(GamePlayer, selectedHeroEntity);
	}

	// this should be replaced by lua
	void GamePlayerComponent::SetSelectedHero(const AZStd::string& selectedHeroName)
	{
		SPARK_UPDATE_VALUE(GamePlayer, selectedHeroName);
		// don't do anything if we got our hero name cleared out for some reason
		if (m_selectedHeroName.empty())
		{
			return;
		}

		AZ_Printf(0, "Got a selected hero! %s", selectedHeroName.c_str());
		EBUS_EVENT_ID(GetEntityId(), GamePlayerNotificationBus, OnHeroSelected, selectedHeroName);

		///@TODO probably don't do this automatically?
		SetSelectedHeroEntity(SpawnHero());
	}

	void GamePlayerComponent::SetPlayerId(const SparkPlayerId& playerId)
	{
		if (playerId == m_playerId)
		{
			return;
		}

		// disconnect to old player id bus
		if (m_playerId != SparkInvalidPlayerId)
		{
			GamePlayerIdRequestBus::Handler::BusDisconnect();
		}

		m_playerId = playerId;

		// connect to new player id bus
		if (m_playerId != SparkInvalidPlayerId)
		{
			GamePlayerIdRequestBus::Handler::BusConnect(m_playerId);
		}

		if (m_replicaChunk && AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
		{
			auto replicaChunk = static_cast<GamePlayerReplicaChunk*>(m_replicaChunk.get());
			replicaChunk->m_playerId.Set(m_playerId);
		}
	}

	void GamePlayerComponent::SetPeerId(const GridMate::PeerId& peerId)
	{
		if (peerId == m_peerId)
		{
			return;
		}

		// disconnect to old peer id bus
		GamePlayerPeerRequestBus::Handler::BusDisconnect();

		m_peerId = peerId;

		// connect to new peer id bus
		if (m_peerId != GridMate::InvalidReplicaPeerId)
		{
			GamePlayerPeerRequestBus::Handler::BusConnect(m_peerId);
		}


		if (m_replicaChunk && AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
		{
			auto replicaChunk = static_cast<GamePlayerReplicaChunk*>(m_replicaChunk.get());
			replicaChunk->m_peerId.Set(m_peerId);
		}
	}

	void GamePlayerComponent::SetSteamId(const CSteamID &steamId)
	{
		SPARK_UPDATE_VALUE(GamePlayer, steamId);

		if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
		{
			EBUS_EVENT(GameNetSyncRequestBus, SetPlayerSteamID, GetEntityId(), steamId);
		}
	}

	SparkPlayerId GamePlayerComponent::GetPlayerId() const
	{
		return m_playerId;
	}

	GridMate::PeerId GamePlayerComponent::GetPeerId() const
	{
		return m_peerId;
	}

	CSteamID GamePlayerComponent::GetSteamId() const
	{
		return m_steamId;
	}

	void GamePlayerComponent::SetClientProxy(const AZ::EntityId &playerProxy)
	{
		m_playerProxyEntity = playerProxy;
	}

	AZ::EntityId GamePlayerComponent::GetPlayerEntity ()
	{
		return GetEntityId();
	}

	TeamId GamePlayerComponent::GetTeamId() const
	{
		return m_teamId;
	}

	void GamePlayerComponent::SetTeamId(const TeamId &teamId)
	{
		SPARK_UPDATE_VALUE(GamePlayer, teamId);
	}

	bool GamePlayerComponent::GetConnected() const
	{
		return m_connected;
	}

	void GamePlayerComponent::SetConnected(const bool &connected)
	{
		SPARK_UPDATE_VALUE(GamePlayer, connected);
		AZ_Printf(0, "Player connected state changed!");
	}

	// this should be replaced by lua
	UnitId GamePlayerComponent::SpawnHero()
	{
		if (m_selectedHeroName.empty() || !AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
		{
			return UnitId();
		}

		AZ_Printf(0, "Spawning hero entity for this player!")

		auto jsonStr = AZStd::string::format(R"({"team":"%s","playerOwner":%d,"tags":["hero"]})", GetTeamId() == 1 ? "right" : "left", GetPlayerId());

		//AzFramework::SliceInstantiationTicket ticket;
		UnitId unitId;
		EBUS_EVENT_RESULT(unitId, GameManagerRequestBus, CreateUnitWithJson, m_selectedHeroName, jsonStr);

		return unitId;
	}

    void GamePlayerComponent::LookAtHero ()
    {
    	
    }
    void GamePlayerComponent::LookAtPosition (AZ::Vector3)
    {
    	
    }

    void GamePlayerComponent::SendClientEvent (AZStd::string name, AZStd::string data, AZ::EntityId entity)
    {
    	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
    	{
    		// only send client events from the server
    		return;
    	}
    	if (IsLocalPlayer())
    	{
    		// if we're local and server then we're hosting locally or in editor
			EBUS_EVENT_ID(GetEntityId(), GamePlayerNotificationBus, OnClientEvent, name, data, entity);
			return;
    	}
    	if (m_playerProxyEntity.IsValid())
    	{
			EBUS_EVENT_ID(m_playerProxyEntity, GamePlayerProxyRequestBus, SendClientEvent, name, data, entity);
		}
    }

    void GamePlayerComponent::SetIsLocalPlayer(const bool &isLocalPlayer)
    {
	#if !defined(DEDICATED_SERVER)
    	m_isLocalPlayer = isLocalPlayer;
	#endif
    }

    bool GamePlayerComponent::IsLocalPlayer()
    {
	#if defined(DEDICATED_SERVER)
    	return false;
	#else
    	return m_isLocalPlayer;
	#endif
    }
}
