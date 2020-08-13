

#include <SteamWorksSystemComponent.h>
#include <platform_impl.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>
#include <ISystem.h>
#include <INetwork.h>

#include <AzFramework/Network/NetworkContext.h>
#include <GridMate/Replica/ReplicaChunk.h>
#include <GridMate/Replica/ReplicaFunctions.h>
#include <GridMate/Replica/RemoteProcedureCall.h>
// NetQuery
#include <AzFramework/Network/NetBindingHandlerBus.h>
#include <GridMate/Serialize/ContainerMarshal.h>


#pragma warning(push)
#pragma warning(disable:4996)
#include "steam/steam_gameserver.h"
#include "steam/steam_api.h"
#pragma warning(pop)

#if defined(WIN32) || defined(WIN64) || defined(_WIN64) || defined(_WIN32)
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#endif

namespace SteamWorks
{
	class SteamWorksSystemReplicaChunk : public GridMate::ReplicaChunkBase
	{
	public:
		AZ_CLASS_ALLOCATOR(SteamWorksSystemReplicaChunk, AZ::SystemAllocator, 0);

		static const char* GetChunkName() { return "SteamWorksSystemReplicaChunk"; }

		SteamWorksSystemReplicaChunk()
		{
		}

		bool IsReplicaMigratable()
		{
			return false;
		}
	};

	class SteamFriendsEvents
	{
	public:
		AZ_CLASS_ALLOCATOR (SteamFriendsEvents, AZ::SystemAllocator, 0)
	private:
		STEAM_CALLBACK(SteamFriendsEvents, OnGameOverlayActivated, GameOverlayActivated_t);
	};

	void SteamFriendsEvents::OnGameOverlayActivated(GameOverlayActivated_t* callback)
	{
		EBUS_EVENT(SteamFriendsNotificationBus, OnGameOverlayActivated, callback->m_bActive);
	}

	void SteamWorksSystemComponent::Reflect(AZ::ReflectContext* context)
	{
		if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
		{
			serialize->Class<SteamWorksSystemComponent, AZ::Component>()
				->Version(0)
				;

			serialize->Class<CSteamID>()
				->Version(0)
				;

			if (AZ::EditContext* ec = serialize->GetEditContext())
			{
				ec->Class<SteamWorksSystemComponent>("SteamWorks", "[Description of functionality provided by this System Component]")
					->ClassElement(AZ::Edit::ClassElements::EditorData, "")
						->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
						->Attribute(AZ::Edit::Attributes::AutoExpand, true)
					;
			}
		}

		if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
		{
			behaviorContext->EBus<SteamWorksRequestBus>("SteamWorksRequestBus")
				->Event("GetSteamID", &SteamWorksRequestBus::Events::GetSteamID)
				->Event("GetSteamName", &SteamWorksRequestBus::Events::GetSteamName)
				;

			behaviorContext->EBus<SteamFriendsNotificationBus>("SteamFriendsNotificationBus")
				->Event("GetSteamID", &SteamFriendsNotificationBus::Events::OnGameOverlayActivated)
				;

			behaviorContext->Class<CSteamID>("CSteamID")
				->Attribute(AZ::Script::Attributes::Storage, AZ::Script::Attributes::StorageType::Value)
				;
		}

		if (auto netContext = azrtti_cast<AzFramework::NetworkContext*>(context))
		{
			netContext->Class<SteamWorksSystemComponent>()
				->Chunk<SteamWorksSystemReplicaChunk>()
				;
		}
	}

	void SteamWorksSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
	{
		provided.push_back(AZ_CRC("SteamWorksService"));
	}

	void SteamWorksSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
	{
		incompatible.push_back(AZ_CRC("SteamWorksService"));
	}

	void SteamWorksSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
	{
		AZ_UNUSED(required);
	}

	void SteamWorksSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
	{
		AZ_UNUSED(dependent);
	}

	void SteamWorksSystemComponent::Init()
	{
	}

	void SteamWorksSystemComponent::Activate()
	{
		AZ_Printf(0, "SteamWorksSystemComponent::Activate");
	#if defined(DEDICATED_SERVER)
		InitServer();
	#else
		#if defined(_RELEASE)
		// client app make sure we don't need to relaunch
		// enable this once we have a working build in steam
		if (SteamAPI_RestartAppIfNecessary(1079650))
		{
			gEnv->pSystem->Quit();
			return;
		}
		#endif

		// create event handlers
		#if !defined(DEDICATED_SERVER)
		m_friendsEvents = azcreate(SteamFriendsEvents);
		#endif
	#endif

		SteamWorksRequestBus::Handler::BusConnect();
		AZ::TickBus::Handler::BusConnect();
	}

	void SteamWorksSystemComponent::Deactivate()
	{
		GridMate::SessionEventBus::Handler::BusDisconnect();
		SteamWorksRequestBus::Handler::BusDisconnect();
		AZ::TickBus::Handler::BusDisconnect();
		if (m_friendsEvents != nullptr)
		{
			azdestroy(m_friendsEvents);
		}
		SteamAPI_Shutdown();
	}

	//NetBindable
	GridMate::ReplicaChunkPtr SteamWorksSystemComponent::GetNetworkBinding ()
	{
		auto replicaChunk = GridMate::CreateReplicaChunk<SteamWorksSystemReplicaChunk>();
		replicaChunk->SetHandler(this);
		m_replicaChunk = replicaChunk;

		return m_replicaChunk;
	}

	void SteamWorksSystemComponent::SetNetworkBinding (GridMate::ReplicaChunkPtr chunk)
	{
		AZ_Printf(0, "SteamWorksSystemComponent::SetNetworkBinding");
		chunk->SetHandler(this);
		m_replicaChunk = chunk;
	}

	void SteamWorksSystemComponent::UnbindFromNetwork ()
	{
		m_replicaChunk->SetHandler(nullptr);
		m_replicaChunk = nullptr;
	}

	bool SteamWorksSystemComponent::OnAuthToken(const AZStd::vector<char>& token, const GridMate::RpcContext&)
	{
		AZ_Printf(0, "Got an auth token!");
		return false;
	}


	CSteamID SteamWorksSystemComponent::GetSteamID () const
	{
		if (SteamUser())
		{
			return SteamUser()->GetSteamID();
		}
		return CSteamID();
	}

	AZStd::string SteamWorksSystemComponent::GetSteamName () const
	{
		if (SteamFriends())
		{
			return SteamFriends()->GetPersonaName();
		}
		return "Server";
	}
	void SteamWorksSystemComponent::CancelAuthTicket(const HAuthTicket &ticket)
	{
		if (SteamGameServer())
		{
			SteamGameServer()->CancelAuthTicket(ticket);
		}
		else if (SteamUser())
		{
			SteamUser()->CancelAuthTicket(ticket);
		}
	}

	#define __STEAMWORKS_AUTH_TOKEN_SIZE 1024
	AuthTokenResult SteamWorksSystemComponent::GetAuthToken()
	{
		AZ_Printf(0, "Creating local auth token");
		// must be constant number, cannot be variable, because stack
		char data[__STEAMWORKS_AUTH_TOKEN_SIZE] = {0};
		uint32_t ticketSize = 0;
		AZ_Error("SteamWorksSystemComponent", SteamUser(), "No steam user found!");
		auto hAuthTicket = SteamUser()->GetAuthSessionTicket(data, __STEAMWORKS_AUTH_TOKEN_SIZE, &ticketSize);

		AZ_Error("SteamWorksSystemComponent", hAuthTicket != k_HAuthTicketInvalid, "Failed to get auth session ticket!");
		AZ_Error("SteamWorksSystemComponent", ticketSize > 0, "Got a ticket that isn't long enough!");

		AZStd::vector<char> result(ticketSize, 0);
		for (uint32_t i = 0; i < ticketSize; ++i)
		{
			result[i] = data[i];
		}

		return { hAuthTicket, result };
	}
	AuthTokenResult SteamWorksSystemComponent::GetServerAuthToken()
	{
		AZ_Printf(0, "Creating server auth token");
		// must be constant number, cannot be variable, because stack
		char data[__STEAMWORKS_AUTH_TOKEN_SIZE] = {0};
		uint32_t ticketSize = 0;
		AZ_Error("SteamWorksSystemComponent", SteamGameServer(), "No steam game server object found!");
		auto hAuthTicket = SteamGameServer()->GetAuthSessionTicket(data, __STEAMWORKS_AUTH_TOKEN_SIZE, &ticketSize);

		AZ_Error("SteamWorksSystemComponent", hAuthTicket != k_HAuthTicketInvalid, "Failed to get auth session ticket!");
		AZ_Error("SteamWorksSystemComponent", ticketSize > 0, "Got a ticket that isn't long enough!");

		AZStd::vector<char> result(ticketSize, 0);
		for (uint32_t i = 0; i < ticketSize; ++i)
		{
			result[i] = data[i];
		}

		return { hAuthTicket, result };
	}

	EBeginAuthSessionResult SteamWorksSystemComponent::ConfirmAuthToken(const AZStd::vector<char> &token, const CSteamID &steamID)
	{
		AZ_Error("SteamWorksSystemComponent", SteamGameServer(), "Tried to confirm an auth token without server init!");

		if (InitServer())
		{
			auto result = SteamGameServer()->BeginAuthSession(&(token.front()), token.size(), steamID);

			return result;
		}
		return k_EBeginAuthSessionResultInvalidTicket;
	}

	bool SteamWorksSystemComponent::InitServer()
	{
		AZ_Printf(0, "SteamWorksSystemComponent::InitServer");
		if (m_isServer)
		{
			return m_isServer;
		}
		auto success = SteamGameServer_Init(INADDR_ANY, 0, 30090, 27017, eServerModeAuthenticationAndSecure, "0.0.4.1");
		AZ_Error("SteamWorksSystemComponent", success, "SteamWorksSystemComponent::Activate failed to activate steam server api!");
		if (success)
		{
			SteamGameServer()->LogOnAnonymous();
			m_isServer = true;
		}
		else
		{
			m_isServer = false;
		}

		return m_isServer;
	}
	void SteamWorksSystemComponent::OnSessionCreated(GridMate::GridSession* session)
	{
		AZ_Printf(0, "SteamWorksSystemComponent::OnSessionCreated");
		// server is created
		if (session->IsHost())
		{
			InitServer();
		}
	}
	void SteamWorksSystemComponent::OnSessionDelete(GridMate::GridSession* session)
	{
		AZ_Printf(0, "SteamWorksSystemComponent::OnSessionDelete");
		if (m_isServer)
		{
			m_isServer = false;
			SteamGameServer_Shutdown();
		}
	}
	void SteamWorksSystemComponent::OnMemberJoined(GridMate::GridSession* session, GridMate::GridMember* member)
	{
		AZ_Printf(0, "SteamWorksSystemComponent::OnMemberJoined");
	}
	void SteamWorksSystemComponent::OnMemberLeaving(GridMate::GridSession* session, GridMate::GridMember* member)
	{
		AZ_Printf(0, "SteamWorksSystemComponent::OnMemberLeaving");
	}

	void SteamWorksSystemComponent::OnTick (float deltaTime, AZ::ScriptTimePoint time)
	{
		if (!m_hasInit)
		{
			AZ_Printf(0, "SteamWorksSystemComponent::OnTick waiting for system");
			ISystem* system = GetISystem();
			if (system)
			{
				GridMate::SessionEventBus::Handler::BusConnect(system->GetINetwork()->GetGridMate());

				gEnv = system->GetGlobalEnvironment();

			#if !defined(DEDICATED_SERVER)
				if (!gEnv->IsEditor())
				{
					auto success = SteamAPI_Init();
					AZ_Error("SteamWorksSystemComponent", success, "SteamWorksSystemComponent::Activate failed to activate steam api!");
				}
			#endif
				m_hasInit = true;
			}
		}

	#if !defined(DEDICATED_SERVER)
		SteamAPI_RunCallbacks();
	#endif
		if (m_isServer)
		{
			SteamGameServer_RunCallbacks();
		}
	}
}
