#pragma once

#include <AzCore/Component/ComponentBus.h>

namespace spark 
{

	class CooldownNotifications
		: public AZ::ComponentBus
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;

		virtual void OnCooldownStarted(float time) {}
		virtual void OnCooldownFinished(){}
	};
	using CooldownNotificationBus = AZ::EBus<CooldownNotifications>;

}
