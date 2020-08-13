#pragma once

#include <AzCore/Component/ComponentBus.h>


namespace spark 
{

	using TimerTicket = int;

	class TimerRequests
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;

		virtual TimerTicket ScheduleTimer(float seconds, AZStd::string description) = 0;
		virtual float GetRemainingTime(TimerTicket) = 0;

		virtual void ClearAllTimers() = 0;
	};
	using TimerRequestBus = AZ::EBus<TimerRequests>;

	class TimerNotifications
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
		using BusIdType = TimerTicket;

		virtual void OnTimerFinished(AZStd::string description){}
	};
	using TimerNotificationBus = AZ::EBus<TimerNotifications>;
}
