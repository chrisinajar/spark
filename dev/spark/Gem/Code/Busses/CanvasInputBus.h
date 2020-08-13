#pragma once

#include <AzCore/Component/ComponentBus.h>

namespace spark
{


	enum MouseButton
	{
		Left=0,
		Middle=1,
		Right=2
	};

	enum MouseEventType
	{
		onmousedown = 1,
		onmouseup,
		onclick,
	};

	class CanvasInputRequests
	: public AZ::EBusTraits
	{
	public:

		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;

		virtual int GetMouseButton() = 0;
		virtual int GetMouseEventType() = 0;
		virtual void ConsumeEvent() = 0;
	};
	using CanvasInputRequestBus = AZ::EBus<CanvasInputRequests>;



	class MouseInputNotifications
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;

		virtual bool onmousedown(int button) { return false; }
		virtual bool onmouseup(int button) { return false; }
		virtual void onmousemove(int x,int y) {}
	};
	using MouseInputNotificationBus = AZ::EBus<MouseInputNotifications>;

}
