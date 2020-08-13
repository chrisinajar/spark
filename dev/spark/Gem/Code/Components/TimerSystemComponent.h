#pragma once


#include "Busses/TimerBus.h"
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/containers/list.h>

namespace spark
{

	class TimerSystemComponent
		: public AZ::Component,
		public TimerRequestBus::Handler,
		public AZ::TickBus::Handler
	{
	public:
		AZ_COMPONENT(TimerSystemComponent, "{B774C555-BF6D-4428-AC8E-2685B0B60DBA}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

		TimerTicket ScheduleTimer(float seconds, AZStd::string description);

		float GetRemainingTime(TimerTicket);

		void ClearAllTimers();

    protected:

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////

		void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;


	private:
		struct TimerInfo
		{
			TimerTicket ticket;
			float seconds;
			AZStd::string description;
		};
		TimerTicket m_lastTicket = 0;
		float m_now_seconds = 0;

		AZStd::list<TimerInfo> m_timers;
	};


}