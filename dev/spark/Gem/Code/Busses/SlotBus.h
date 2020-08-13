#pragma once
#include <AzCore/Component/ComponentBus.h>
#include <AzCore/RTTI/BehaviorContext.h>

#include "Utils/Slot.h"


namespace spark {

	class SlotNotifications
		: public AZ::EBusTraits
	{
	public:
		// Multiple handlers. Events received in undefined order.
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		// The EBus uses a single address.
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;


		virtual void OnSlotPressed(const Slot&) = 0;
		virtual void OnSlotReleased(const Slot&) = 0;
	};
	using SlotNotificationBus = AZ::EBus<SlotNotifications>;



	class SlotNotificationBusHandler
		: public SlotNotificationBus::Handler
		, public AZ::BehaviorEBusHandler
	{
	public:
		AZ_EBUS_BEHAVIOR_BINDER(SlotNotificationBusHandler, "{452A05A9-5B3D-4843-A0B2-BD676F157E6C}", AZ::SystemAllocator, OnSlotPressed, OnSlotReleased);

		void OnSlotPressed(const Slot& slot)
		{

			Call(FN_OnSlotPressed,slot);
		}
		void OnSlotReleased(const Slot& slot)
		{
			Call(FN_OnSlotReleased,slot);
		}
	};
}