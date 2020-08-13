
#pragma once

#include <AzCore/Component/Component.h>
#include <AzFramework/Network/NetBindable.h>
#include <GridMate/Session/Session.h>

#include <AzCore/Component/TickBus.h>
#include <SteamWorks/SteamWorksBus.h>

#include <AzCore/std/containers/unordered_map.h>

namespace SteamWorks
{
	class SteamFriendsEvents;
	class SteamWorksSystemReplicaChunk;

	class SteamWorksSystemComponent
		: public AZ::Component
		, public AzFramework::NetBindable
		, protected GridMate::SessionEventBus::Handler
		, protected AZ::TickBus::Handler
		, protected SteamWorksRequestBus::Handler
	{
		friend SteamWorksSystemReplicaChunk;
	public:
		AZ_COMPONENT(SteamWorksSystemComponent, "{EB8E4C55-A94A-46D1-89AF-CDA15FB769ED}", AzFramework::NetBindable);

		static void Reflect(AZ::ReflectContext* context);

		static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
		static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
		static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
		static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

		//NetBindable
		GridMate::ReplicaChunkPtr GetNetworkBinding ();
		void SetNetworkBinding (GridMate::ReplicaChunkPtr chunk);
		void UnbindFromNetwork ();

	protected:
		void CancelAuthTicket(const HAuthTicket &ticket);

		////////////////////////////////////////////////////////////////////////
		// SteamWorksRequestBus interface implementation
		CSteamID GetSteamID() const override;
		AZStd::string GetSteamName() const override;
		AuthTokenResult GetAuthToken() override;
		AuthTokenResult GetServerAuthToken() override;
		EBeginAuthSessionResult ConfirmAuthToken(const AZStd::vector<char> &, const CSteamID &steamID) override;
		////////////////////////////////////////////////////////////////////////
		
		// Session events bus
		void OnSessionCreated(GridMate::GridSession* session) override;
		void OnSessionDelete(GridMate::GridSession* session) override;
		void OnMemberJoined(GridMate::GridSession* session, GridMate::GridMember* member) override;
		void OnMemberLeaving(GridMate::GridSession* session, GridMate::GridMember* member) override;

		////////////////////////////////////////////////////////////////////////
		// AZ::Component interface implementation
		void Init() override;
		void Activate() override;
		void Deactivate() override;
		////////////////////////////////////////////////////////////////////////
		void OnTick (float deltaTime, AZ::ScriptTimePoint time) override;

		bool OnAuthToken(const AZStd::vector<char>& token, const GridMate::RpcContext&);

		bool InitServer();

		GridMate::ReplicaChunkPtr m_replicaChunk;
		SteamFriendsEvents* m_friendsEvents = nullptr;
		bool m_hasInit = false;
		bool m_isServer = false;

		HAuthTicket m_localTicket = k_HAuthTicketInvalid;
		AZStd::unordered_map<GridMate::MemberIDCompact, HAuthTicket> m_peerAuthTickets;
	};
}
