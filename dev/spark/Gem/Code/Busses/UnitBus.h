#pragma once

#include <AzCore/Component/EntityId.h>
#include <AzCore/Component/ComponentBus.h>
#include "Utils/UnitOrder.h"
#include "Utils/Status.h"
#include "Utils/CommonTypes.h"
#include "Busses/ModifierBus.h"
#include "Busses/GameplayBusses.h"

namespace spark
{

    class UnitRequests
        : public AZ::ComponentBus
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		//////////////////////////////////////////////////////////////////////////

        // metadata getters
        virtual bool IsSelectable() = 0;
		virtual AZStd::string GetName() = 0;
		virtual void SetName(AZStd::string) = 0;

		virtual UnitTypeId GetUnitTypeId() = 0;
		virtual void SetUnitTypeId(UnitTypeId) = 0;

		virtual bool IsAlive() = 0;
		virtual void SetAlive(bool alive) = 0;

		virtual TeamId GetTeamId() = 0;
		virtual void   SetTeamId(TeamId) = 0;
		virtual AZ::EntityId GetPlayerOwner() = 0;

		virtual void SetQuickBuyItemList(AZStd::vector<ItemTypeId> list) = 0;
		virtual AZStd::vector<ItemTypeId> GetQuickBuyItemList() = 0;

		virtual bool IsVisible() = 0;
		virtual void SetVisible(bool visible) = 0;

		virtual void UpdateVisibility(bool force_update) = 0;

        // orders
		virtual UnitOrder GetCurrentOrder() const = 0;
		virtual void NewOrder(UnitOrder,bool queue=false) = 0;

		//modifiers
		virtual AZStd::vector<ModifierId> GetModifiers() = 0;
		virtual void SetModifiers(AZStd::vector<ModifierId>) = 0;
		virtual void ClearModifiers() = 0;
		virtual void AddModifier(ModifierId) = 0;
		virtual void RemoveModifier(ModifierId) = 0;
		virtual ModifierId FindModifierByTypeId(ModifierTypeId) = 0;

		virtual void ApplyDispel(Dispel) = 0;

		//status
		virtual Status GetStatus() = 0;
		virtual void SetStatus(Status) = 0;
		virtual void Kill (Damage damage) = 0;
		virtual void KillWithAbility (AZ::EntityId, AZ::EntityId) = 0;

		virtual AZStd::string ToString() = 0;
		virtual void Destroy() = 0;
    };
    using UnitRequestBus = AZ::EBus<UnitRequests>;


	class UnitNotifications
        : public AZ::ComponentBus
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
        //////////////////////////////////////////////////////////////////////////

       
		virtual void OnNewOrder(UnitOrder, bool queue) {}

		virtual void OnModifierAttached(ModifierId) {}
		virtual void OnModifierDetached(ModifierId) {}

		virtual void OnSpawned() {}
		virtual void OnDeath() {}
		virtual void OnKilled(Damage) {}
    };
    using UnitNotificationBus = AZ::EBus<UnitNotifications>;




	class UnitComponent;
	using  UnitsFilter=AZStd::delegate<bool(const AZ::EntityId&)>;
	using  UnitsEntitiesFilter=AZStd::delegate<bool(const AZ::Entity*)>;

	struct AllUnitsFilter {
		bool operator()(const AZ::EntityId&) { return true; }
	};


	class UnitsRequests
		: public AZ::EBusTraits
	{
	public:
		// Multiple handlers. Events received in undefined order.
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		// The EBus uses a single address.
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;	

		//static bool AllUnitsFilter(const AZ::EntityId&) { return true; }


		virtual void GetAllUnits(AZStd::vector<AZ::EntityId> &result) = 0;
		virtual void GetAllUnitsComponents(AZStd::vector<UnitComponent*> &result) = 0;

		virtual void GetUnits(AZStd::vector<AZ::EntityId> &result,const UnitsFilter &filter) = 0;
		virtual void GetUnitsEntities(AZStd::vector<AZ::Entity*> &result,const UnitsEntitiesFilter &filter) = 0;
		
	};
	using UnitsRequestBus = AZ::EBus<UnitsRequests>;




	class UnitsNotifications
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;	
        //////////////////////////////////////////////////////////////////////////

		virtual void OnUnitSpawned(UnitId) {}
		virtual void OnUnitDeath(UnitId) {}
		virtual void OnUnitCreated(UnitId, AZStd::string) {}
		virtual void OnUnitKilled(UnitId, Damage) {}
    };
    using UnitsNotificationBus = AZ::EBus<UnitsNotifications>;
}
