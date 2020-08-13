
#include "spark_precompiled.h"


#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Serialization/EditContext.h>

#include "TimerSystemComponent.h"



namespace spark {


	/**
	*	Interface to lua scripts
	*/
	class TimerNotificationBusHandler
		: public TimerNotificationBus::Handler
		, public AZ::BehaviorEBusHandler
	{
	public:
		AZ_EBUS_BEHAVIOR_BINDER(TimerNotificationBusHandler, "{B60DAF5E-8B4B-4077-AB94-438F6B0E2729}", AZ::SystemAllocator, OnTimerFinished);

		void OnTimerFinished(AZStd::string description)
		{
			Call(FN_OnTimerFinished, description);
		}
	};



	void TimerSystemComponent::Reflect(AZ::ReflectContext* context)
	{
		if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
		{
			serialize->Class<TimerSystemComponent, AZ::Component>()
				->Version(0)
				;

			if (AZ::EditContext* ec = serialize->GetEditContext())
			{
				ec->Class<TimerSystemComponent>("TimerSystemComponent", "handle timers")
					->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					//   ->Attribute(AZ::Edit::Attributes::Category, "spark")
					//	->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"));
					//;
					->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
					->Attribute(AZ::Edit::Attributes::AutoExpand, true);
			}
		}

		if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
		{
			behaviorContext->EBus<TimerRequestBus>("TimerRequestBus")
				->Event("ScheduleTimer", &TimerRequestBus::Events::ScheduleTimer)
				->Event("GetRemainingTime", &TimerRequestBus::Events::GetRemainingTime)
				->Event("ClearAllTimers", &TimerRequestBus::Events::ClearAllTimers)
				;


			behaviorContext->EBus<TimerNotificationBus>("TimerNotificationBus")
				->Handler<TimerNotificationBusHandler>()
				->Event("OnTimerFinished", &TimerNotificationBus::Events::OnTimerFinished);
		}
	}

	void TimerSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
	{
		provided.push_back(AZ_CRC("TimerSystemService"));
	}

	void TimerSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
	{
		incompatible.push_back(AZ_CRC("TimerSystemService"));
	}

	void TimerSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
	{
		(void)required;
	}

	void TimerSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
	{
		(void)dependent;
	}

	TimerTicket TimerSystemComponent::ScheduleTimer(float seconds, AZStd::string description)
	{
		TimerInfo info{ ++m_lastTicket,m_now_seconds + seconds , description};

		auto it = m_timers.begin();

		while (it != m_timers.end() && it->seconds < info.seconds)
		{
			++it;
		}
		m_timers.insert(it, info);
		return info.ticket;
	}

	float TimerSystemComponent::GetRemainingTime(TimerTicket ticket)
	{
		for (auto t : m_timers)
		{
			if (t.ticket == ticket)
			{
				return  t.seconds - m_now_seconds;
			}
		}
		return 0.0f;
	}

	void TimerSystemComponent::ClearAllTimers()
	{
		m_timers.clear();
		m_lastTicket = 0;
	}

	void TimerSystemComponent::Init()
	{
	}

	void TimerSystemComponent::Activate()
	{
		TimerRequestBus::Handler::BusConnect();
		AZ::TickBus::Handler::BusConnect();
	}

	void TimerSystemComponent::Deactivate()
	{
		AZ::TickBus::Handler::BusDisconnect();
		TimerRequestBus::Handler::BusDisconnect();
	}

	void TimerSystemComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time)
	{
		m_now_seconds = (float)time.GetSeconds();

		auto head = m_timers.begin();
		while (head != m_timers.end() && m_now_seconds > head->seconds) 
		{
				EBUS_EVENT_ID(head->ticket, TimerNotificationBus, OnTimerFinished, head->description);
				m_timers.pop_front();
				head = m_timers.begin();
		}
	}


}



