#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/ComponentBus.h>

#include <AzCore/Math/Vector2.h>
#include <Utils/CommonTypes.h>

namespace spark
{
	
	class NavigationAreaRequests
		: public AZ::ComponentBus
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;


		virtual void UpdateHeightfield() = 0;

		virtual void SetTilesResolution(AZ::Vector2) = 0;
		virtual AZ::Vector2 GetTilesResolution() = 0;

		virtual AZ::Vector3 GetAabbMin() = 0;
		virtual AZ::Vector3 GetAabbMax() = 0;

		virtual bool IsPointInside(AZ::Vector3) = 0;
		virtual bool IsUnitInside(UnitId) = 0;
	};
	using NavigationAreaRequestBus = AZ::EBus<NavigationAreaRequests>;


	class NavigationAreaNotifications
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;

		virtual void OnNavigationAreaActivated(AZ::Entity* e) {}
		virtual void OnNavigationAreaDeactivated(AZ::Entity* e) {}

		virtual void OnMainNavigationAreaChanged(AZ::EntityId) {}
	};
	using NavigationAreaNotificationBus = AZ::EBus<NavigationAreaNotifications>;

}
