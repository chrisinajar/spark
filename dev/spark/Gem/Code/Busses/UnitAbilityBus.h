#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/std/string/string.h>

#include "AbilityBus.h"
#include "Utils/Slot.h"

namespace spark {


	class UnitAbilityRequests
		: public AZ::ComponentBus
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		//////////////////////////////////////////////////////////////////////////

		// Put your public methods here
		virtual bool		 HasAbilityInSlot(Slot slot) = 0;
		virtual AZ::EntityId GetAbilityInSlot(Slot slot) = 0;
		virtual void         SetAbilityInSlot(Slot slot,AZ::EntityId ability) = 0;
		virtual void         ClearAbilitySlot(Slot slot) = 0;
		virtual void		 ClearAllAbilities() = 0;

		virtual AbilityId HasAbilityType(AbilityTypeId) = 0;
		virtual bool HasAbility(AbilityId) = 0;
		virtual Slot GetAbilitySlot(AbilityId) = 0;
		virtual void DetachAbility(AbilityId) = 0;

		virtual AZStd::vector<AbilityId> GetAbilities() = 0;

		virtual bool CanCastAbility(AZ::EntityId ability) = 0; 
		virtual AZ::EntityId CanCastAbilityInSlot(Slot slot) = 0; //check if the ability exists, if is not in cooldown and if the unit have enough resources. Return the entityId of the ability if castable

		virtual void CastAbility(CastContext) = 0;
		virtual void InterruptCasting() = 0;


		virtual AZStd::string ToString()=0;
	};
	using UnitAbilityRequestBus = AZ::EBus<UnitAbilityRequests>;



	class UnitAbilityNotifications
		: public AZ::ComponentBus
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		//////////////////////////////////////////////////////////////////////////

		// Put your public methods here
		virtual void OnCastFinished() {}

		
		virtual void OnAbilityAttached(AZ::EntityId ability) {}
		virtual void OnAbilityDetached(AZ::EntityId ability) {}
	};
	using UnitAbilityNotificationBus = AZ::EBus<UnitAbilityNotifications>;

}