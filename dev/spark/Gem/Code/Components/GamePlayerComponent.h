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
#pragma once

#ifndef MULTIPLAYERSAMPLE_COMPONENTS_GAMEPLAYERCOMPONENT_H
#define MULTIPLAYERSAMPLE_COMPONENTS_GAMEPLAYERCOMPONENT_H

#include <AzCore/Component/Component.h>
#include <AzCore/Component/EntityBus.h>
#include <AZCore/Component/TickBus.h>

#include <AzFramework/Network/NetBindable.h>

#include "Busses/GamePlayerBus.h"
#include <AzCore/RTTI/TypeInfo.h>
#include <AzCore/Component/EntityId.h>
//#include "Busses/GameTypes.h"

namespace spark
{
	// GamePlayerComponent
	// This component acts as the player's identifier on the server
	class GamePlayerComponent
		: public AZ::Component
		, public AzFramework::NetBindable
		, public AZ::TickBus::Handler
		, public GamePlayerInterfaceBus::Handler
		, public GamePlayerIdRequestBus::Handler
		, public GamePlayerPeerRequestBus::Handler
	{
		friend class GamePlayerReplicaChunk;
		friend class GameNetSyncComponent;
	public:
		// CVar
		//static void OnSpawnTagChanged(ICVar* cvar);
		//static void OnUserNameChanged(ICVar* cvar);

		AZ_COMPONENT(GamePlayerComponent, "{25DE1395-B750-4E8C-ADB4-F31916C7C7E3}", AzFramework::NetBindable);
		static void Reflect(AZ::ReflectContext* context);

		// AZ::Component
		void Init() override;
		void Activate() override;
		void Deactivate() override;

	protected:
        bool IsLocalPlayer () override;
        void SetIsLocalPlayer (const bool&) override;

		// NetBindable
		GridMate::ReplicaChunkPtr GetNetworkBinding() override;
		void SetNetworkBinding(GridMate::ReplicaChunkPtr chunk) override;
		void UnbindFromNetwork() override;

		// Events coming from network
		void OnNewPlayerName(const AZStd::string& name, const GridMate::TimeContext& tc);
		void OnNewPlayerId(const SparkPlayerId&, const GridMate::TimeContext& tc);
		void OnNewPeerId(const GridMate::PeerId&, const GridMate::TimeContext& tc);
		void OnNewSteamId(const CSteamID&, const GridMate::TimeContext& tc);
		void OnNewSelectedHero(const AZStd::string&, const GridMate::TimeContext& tc);
		void OnNewSelectedHeroEntity(const AZ::EntityId&, const GridMate::TimeContext&);
		void OnNewTeamId(const TeamId&, const GridMate::TimeContext& tc);
		void OnNewConnected(const bool&, const GridMate::TimeContext& tc);
		void OnNewSteamId(const bool&, const CSteamID& tc);
		
		// GamePlayerInterfaceBus
		void SetName(const AZStd::string &name) override;
		AZStd::string GetName() const override;
		void Enumerate(PlayersEnumerateCallback callback) override;
		SparkPlayerId GetPlayerId() const override;
		void SetPlayerId(const SparkPlayerId &) override;
		GridMate::PeerId GetPeerId() const override;
		void SetPeerId(const GridMate::PeerId &) override;
		CSteamID GetSteamId() const override;
		void SetSteamId(const CSteamID &) override;
		void SetClientProxy(const AZ::EntityId &playerProxy) override;
		AZStd::string GetSelectedHero() const override;
		UnitId GetSelectedHeroEntity() const override;
		void SetSelectedHeroEntity(AZ::EntityId selectedHeroEntity);
		void SetSelectedHero(const AZStd::string&) override;
		TeamId GetTeamId() const override;
		void SetTeamId(const TeamId&) override;
		bool GetConnected() const override;
		void SetConnected(const bool&) override;
		UnitId SpawnHero() override;
        void LookAtHero () override;
        void LookAtPosition (AZ::Vector3) override;
        void SendClientEvent (AZStd::string name, AZStd::string data, AZ::EntityId entity) override;

		// timer
		void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

		// peer id request bus, requests, by network peerid
		AZ::EntityId GetPlayerEntity () override;


		GridMate::ReplicaChunkPtr   m_replicaChunk;
		AZStd::string               m_name;
		AZStd::string               m_selectedHeroName;
		AZ::EntityId                m_selectedHeroEntity;
		AZ::EntityId                m_playerProxyEntity;
		SparkPlayerId               m_playerId = SparkInvalidPlayerId;
		GridMate::PeerId            m_peerId = GridMate::InvalidReplicaPeerId;
		CSteamID            		m_steamId = CSteamID();
		TeamId 						m_teamId;
		bool 						m_connected = false;
		bool						m_isLocalPlayer = false;
	};
}

#endif
