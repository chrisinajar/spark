#pragma once

#include <AzCore/Component/EntityId.h>
#include <AzCore/Component/ComponentBus.h>
#include "Utils/UnitOrder.h"

class CSteamID;

namespace spark
{


	class GameNetSyncRequests
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
		//////////////////////////////////////////////////////////////////////////

		virtual void SendNewOrder(AZ::EntityId unit, UnitOrder, bool queue) = 0;
		virtual void SendUIEvent (const AZStd::string &eventName, const AZStd::string &eventValue) = 0;
		virtual AZ::EntityId GetLocalPlayer () = 0;
		virtual AZStd::vector<AZ::EntityId> GetPlayerList () = 0;
		virtual AZ::EntityId CreateFakePlayer () = 0;
		virtual void SetPlayerSteamID (const AZ::EntityId playerEntityId, const CSteamID &name) = 0;
	};
	using GameNetSyncRequestBus = AZ::EBus<GameNetSyncRequests>;
	
	class GameNetSyncNotifications
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
		//////////////////////////////////////////////////////////////////////////

		virtual void OnUIEvent (AZ::EntityId playerId, AZStd::string eventName, AZStd::string eventValue) {}
	};
	using GameNetSyncNotificationBus = AZ::EBus<GameNetSyncNotifications>;


	class GameInterface : public AZ::EBusTraits
	{
	public:
		// EBusTraits 
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;

		// GameInterface
		virtual void LeaveSession() = 0;        
	};
	using GameBus = AZ::EBus<GameInterface>;    



	class UnitNetSyncRequests
		: public AZ::ComponentBus
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		//////////////////////////////////////////////////////////////////////////

		virtual void SetNamedParameterBool (const AZStd::string &valueName, const bool &value) = 0;
		virtual void SetNamedParameterFloat (const AZStd::string &valueName, const float &value) = 0;
	};
	using UnitNetSyncRequestBus = AZ::EBus<UnitNetSyncRequests>;
}
