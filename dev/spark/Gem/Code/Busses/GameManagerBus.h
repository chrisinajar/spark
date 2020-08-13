#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/ComponentBus.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/Math/Color.h>
#include <AzCore/std/string/string.h>

#include <AzCore/JSON/document.h>
#include <AzFramework/Entity/GameEntityContextBus.h>
#include <LmbrCentral/Scripting/TagComponentBus.h>

#include "Utils/CommonTypes.h"



namespace spark {
	namespace GamePhase {
		enum Type {
			INIT,
			WAITING_FOR_PLAYERS,
			CONFIGURE_GAME,
			HERO_SELECT,
			PRE_GAME,
			GAME,
			POST_GAME
		};
	}

	class GameManagerSystemRequests
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;

		virtual bool    LoadGameFile(AZStd::string filename) = 0;
		virtual bool    LoadGameMode(AZStd::string gamemodeName) = 0;
		virtual void    PlayGame() = 0;

		virtual void    ExecuteConsoleCommand(AZStd::string cmd) = 0;//for dev/debugging todo:remove
	};
	using GameManagerSystemRequestBus = AZ::EBus<GameManagerSystemRequests>;

	class GameManagerRequests
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;

		virtual bool GameManagerAlreadyExists() = 0;
		virtual bool IsServer() = 0;

		virtual AZStd::string GetGamemode () const = 0;
		virtual GamePhase::Type GetGamePhase () const = 0;
		virtual void SetGamePhase (GamePhase::Type) = 0;

		virtual AZStd::string GetActualFilename(AZStd::string hint) = 0;

		virtual UnitId	GetNearestUnit(AZ::Vector3) = 0;
		virtual UnitId  GetNearestEnemyUnit(UnitId me, AZ::Vector3) = 0;

		virtual AZStd::vector<UnitId> GetEntitiesHavingTags(AZStd::vector<LmbrCentral::Tag>) = 0;
		virtual AZStd::vector<UnitId> GetEntitiesHavingTag(LmbrCentral::Tag) = 0;
		virtual AZStd::vector<UnitId> GetUnitsInsideSphere(AZ::Vector3 center, float radius) = 0;

		virtual ShopId GetNearestShop(UnitId) = 0;
		virtual AZStd::vector<ShopId> GetShopsInRange(UnitId) = 0;
		virtual AZStd::vector<ShopId> GetAllShops() = 0;
		 
		virtual TeamId GetTeamIdByName(AZStd::string name) = 0;
		virtual AZStd::string GetTeamName(TeamId) = 0;
		virtual int GetNumberTeams() = 0;
		virtual void SetTeamColor(TeamId, AZ::Color) = 0;
		virtual AZ::Color GetTeamColor(TeamId) = 0;	

		virtual float		GetDistance(UnitId, UnitId) = 0;
		virtual AZ::Vector3 GetForwardDirection(UnitId unit) = 0;

		virtual void	RegisterAbility(AbilityTypeId, AZStd::string filename) {};
		virtual void	RegisterUnit(UnitTypeId, AZStd::string filename) {};

		virtual AbilityId  CreateAbility(AbilityTypeId) = 0;
		virtual ItemId     CreateItem(ItemTypeId) = 0;
		virtual ModifierId CreateModifier(UnitId,AbilityId,ModifierTypeId) = 0;

		virtual UnitId CreateUnit(UnitTypeId) = 0;
		virtual UnitId CreateUnitWithJson(UnitTypeId,AZStd::string json) = 0;
		//virtual AzFramework::SliceInstantiationTicket CreateUnitAtPosition(UnitTypeId,AZ::Vector3 pos) = 0;

		virtual ParticleSystemId CreateParticleSystem(ParticleSystemTypeId) = 0;

		virtual void DestroyEntity(AZ::EntityId) = 0;

		virtual void FinishGameConfigure() = 0;

		virtual AZ::EntityId GetGameManagerEntity() = 0;
		//virtual Unit    CreateUnit(UnitId) = 0;
	};
	using GameManagerRequestBus = AZ::EBus<GameManagerRequests>;


	class GameManagerNotifications
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;

		virtual void OnGameManagerActivated (AZ::Entity* gameManager) {}
		virtual void OnGameManagerReady (AZ::Entity* gameManager) {}
		virtual void OnGamePhaseChange (GamePhase::Type newPhase) {}
	};
	using GameManagerNotificationBus = AZ::EBus<GameManagerNotifications>;

	class GameManagerSliceNotifications
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;	
		typedef AzFramework::SliceInstantiationTicket BusIdType;

		virtual void OnUnitCreated (const AZ::EntityId &id) {}
	};
	using GameManagerSliceNotificationBus = AZ::EBus<GameManagerSliceNotifications>;
}
