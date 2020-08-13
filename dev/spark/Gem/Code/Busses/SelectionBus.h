#pragma once

#include <AzCore/Component/ComponentBus.h>
#include "Utils/UnitsGroup.h"


namespace spark {

	class SelectionRequests
		: public AZ::EBusTraits
	{
	public:
		// One handler is supported.
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		// The EBus uses a single address.
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;

		virtual void AddUnitToSelection(AZ::EntityId, bool setMainUnit) = 0;
		virtual void UnselectUnit(AZ::EntityId) = 0;
		virtual void SelectUnit(AZ::EntityId) = 0;
		virtual void ClearSelection() = 0;
		
		virtual void RotateToNextSelectedUnit() = 0;
		virtual void RotateToPreviousSelectedUnit() = 0;
		virtual void SetMainSelectedUnit(AZ::EntityId) = 0;
		virtual void SetMainSelectedUnitByIndex(int) = 0;

		virtual AZ::EntityId GetMainSelectedUnit() = 0;
		virtual AZStd::vector<AZ::EntityId>   GetSelectedUnits() = 0;
		virtual void						  SetSelectedUnits(AZStd::vector<AZ::EntityId>) = 0;
	};
	using SelectionRequestBus = AZ::EBus<SelectionRequests>;


	class SelectionNotifications
		: public AZ::EBusTraits
	{
	public:
		// Multiple handlers. Events received in undefined order.
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		// The EBus uses a single address.
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;


		virtual void OnSelectionChanged(AZStd::vector<AZ::EntityId> units) {}
		virtual void OnMainSelectedUnitChanged(AZ::EntityId) {}
	};
	using SelectionNotificationBus = AZ::EBus<SelectionNotifications>;


	class SelectionNotificationBusHandler
		: public SelectionNotificationBus::Handler
		, public AZ::BehaviorEBusHandler
	{
	public:
		AZ_EBUS_BEHAVIOR_BINDER(SelectionNotificationBusHandler, "{8A0890FF-3EA0-4D3F-B473-16A2C7E9554C}", AZ::SystemAllocator, OnSelectionChanged, OnMainSelectedUnitChanged);

		void OnSelectionChanged(AZStd::vector<AZ::EntityId> units) 
		{
			Call(FN_OnSelectionChanged, units);
		}

		void OnMainSelectedUnitChanged(AZ::EntityId unit)
		{
			Call(FN_OnMainSelectedUnitChanged, unit);
		}
	};

}
