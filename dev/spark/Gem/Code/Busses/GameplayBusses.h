#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/ComponentBus.h>

#include <AzCore/RTTI/BehaviorContext.h>
#include "VariableBus.h"
#include "Utils/Filter.h"

namespace spark
{
	extern void ReflectGameplayBusses(AZ::ReflectContext* reflection);


	struct Damage
	{
		AZ_TYPE_INFO(Damage, "{612B90AC-F635-47BB-AB67-76F320BD3C9E}");
		static void Reflect(AZ::ReflectContext* reflection);
		
		Damage(){}

		enum {
			DAMAGE_TYPE_PHYSICAL,
			DAMAGE_TYPE_MAGICAL,
			DAMAGE_TYPE_PURE,
			DAMAGE_TYPE_KILL,
		};
		unsigned int type = DAMAGE_TYPE_PHYSICAL;

		enum {
			DAMAGE_FLAG_NON_LETHAL=BIT(0),
			DAMAGE_FLAG_CRITICAL  =BIT(1),
		};
		unsigned int flag = 0;

		Value damage=0;
		AZ::EntityId source;
		AZ::EntityId target;
		AZ::EntityId ability;

		AZStd::string ToString() const;
	};

	struct AttackInfo {
		AZ_TYPE_INFO(AttackInfo, "{DCCD2271-E2B1-4462-BBE1-CB21ED89D2A7}");
		static void Reflect(AZ::ReflectContext* reflection);
		AZ::EntityId attacker, attacked;
		Value damage;

		//enum {//todo
		//	BEFORE_REDUCTION,
		//	POST_REDUCTION,
		//	MISS,
		//};

		AZStd::string ToString() const;
	};


	class OnDamageTakenRequests
		: public AZ::ComponentBus
	{
		public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        //////////////////////////////////////////////////////////////////////////

		virtual Value ApplyDamage(Damage damage); //apply damage and return the effective hp removed
		virtual Damage GetLastAppliedDamage() { return m_lastDamage; }
	protected:
		Damage m_lastDamage;

	};
	using OnDamageTakenRequestBus = AZ::EBus<OnDamageTakenRequests>;


	class OnDamageTakenNotifications
		: public AZ::ComponentBus
	{
	protected:
		int m_priority = 0;
	
		
	public:
		static void Reflect(AZ::ReflectContext* reflection);

		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		//static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::MultipleAndOrdered;
		
		// Implement a custom handler-ordering function
		struct BusHandlerOrderCompare
            : public AZStd::binary_function<OnDamageTakenNotifications*, OnDamageTakenNotifications*, bool>                           
        {
            AZ_FORCE_INLINE bool operator()(OnDamageTakenNotifications* left, OnDamageTakenNotifications* right) const { return left->GetPriority() < right->GetPriority(); }
        };
		//////////////////////////////////////////////////////////////////////////
	protected:
		friend class OnDamageTakenRequests;
		virtual void FireOnDamageTakenFilter(Damage &damage, bool &hasBeenConsumed);
	public:
		

		void SetPriority(int priority) { m_priority = priority; }
		int  GetPriority()			   { return m_priority; }		

		virtual void OnDamageTakenFilter_(Damage damage) = 0;
		virtual void OnDamageTaken(Damage damage,Value hp_removed) = 0;
		//virtual bool OnDamageTaken(Damage &damage) { return true; };
	};
	using OnDamageTakenNotificationBus = AZ::EBus<OnDamageTakenNotifications>;


	class OnDamageDealtNotifications
		: public AZ::ComponentBus
	{
	public:
		static void Reflect(AZ::ReflectContext* reflection);
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;

		virtual void OnDamageDealtFilter_(Damage damage) {}
		virtual void OnDamageDealt(Damage damage, Value hp_removed) {}
	};
	using OnDamageDealtNotificationBus = AZ::EBus<OnDamageDealtNotifications>;



	class AttackEventsNotifications
	: public AZ::EBusTraits
	{
	public:
		static void Reflect(AZ::ReflectContext* reflection);
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
		typedef VariableId BusIdType;
		//////////////////////////////////////////////////////////////////////////

		virtual void OnAttackEvent(AZStd::string eventId, AttackInfo) {}
	};
	using AttackEventsNotificationBus = AZ::EBus<AttackEventsNotifications>;

	inline void SparkAttackEvent(AZ::EntityId entityId, AZStd::string eventId, AttackInfo attackInfo)
	{
		EBUS_EVENT_ID(VariableId(entityId, eventId), AttackEventsNotificationBus, OnAttackEvent, eventId, attackInfo);
	}

	inline bool SparkAttackFilterEvent(AZStd::string eventId, AttackInfo &attackInfo)
	{
		auto result = SendFilterEventId<AttackEventsNotificationBus>(VariableId(attackInfo.attacker, eventId), &AttackEventsNotificationBus::Events::OnAttackEvent, eventId, attackInfo);
		
		switch (result.action)
		{
		case FilterResult::FILTER_PREVENT:
			return false;
		case FilterResult::FILTER_MODIFY:
			result.CopyTo(eventId, attackInfo);
			break;
		}

		result = SendFilterEventId<AttackEventsNotificationBus>(VariableId(attackInfo.attacked, eventId), &AttackEventsNotificationBus::Events::OnAttackEvent, eventId, attackInfo);

		switch (result.action)
		{
		case FilterResult::FILTER_PREVENT:
			return false;
		case FilterResult::FILTER_MODIFY:
			result.CopyTo(eventId, attackInfo);
			break;
		}
		return true;
	}


	class OnRightClickedNotifications
		: public AZ::ComponentBus
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;

		static void Reflect(AZ::ReflectContext* reflection);

		virtual void OnRightClickedFilter() {}
		virtual void OnRightClicked() {}
	};
	using OnRightClickedNotificationBus = AZ::EBus<OnRightClickedNotifications>;

	class OnRightClickedGlobalNotifications
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;

		static void Reflect(AZ::ReflectContext* reflection);

		virtual void OnEntityRightClickedFilter(AZ::EntityId) {}
		virtual void OnEntityRightClicked(AZ::EntityId) {}
	};
	using OnRightClickedGlobalNotificationBus = AZ::EBus<OnRightClickedGlobalNotifications>;



#define SPARK_SINGLE_EVENT_NOTIFICATION_BUS(_FunctionName,...)	class _FunctionName##Notifications: public AZ::ComponentBus{\
	public:\
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;\
		virtual void _FunctionName(__VA_ARGS__) {}\
	};\
	using _FunctionName##NotificationBus = AZ::EBus<_FunctionName##Notifications>;


#define SPARK_SINGLE_EVENT_NOTIFICATION_BUS_AND_HANDLER_0(_FunctionName,_ClassGuid)\
	SPARK_SINGLE_EVENT_NOTIFICATION_BUS(_FunctionName)\
	class _FunctionName##NotificationBusHandler: public _FunctionName##NotificationBus::Handler, public AZ::BehaviorEBusHandler{\
	public:\
		AZ_EBUS_BEHAVIOR_BINDER(_FunctionName##NotificationBusHandler, _ClassGuid, AZ::SystemAllocator, _FunctionName);\
		void _FunctionName() {\
			Call(FN_##_FunctionName);\
		}\
	};

#define SPARK_SINGLE_EVENT_NOTIFICATION_BUS_AND_HANDLER_1(_FunctionName,_ClassGuid,param1)\
	SPARK_SINGLE_EVENT_NOTIFICATION_BUS(_FunctionName,param1)\
	class _FunctionName##NotificationBusHandler: public _FunctionName##NotificationBus::Handler, public AZ::BehaviorEBusHandler{\
	public:\
		AZ_EBUS_BEHAVIOR_BINDER(_FunctionName##NotificationBusHandler, _ClassGuid, AZ::SystemAllocator, _FunctionName);\
		void _FunctionName(param1 p1) {\
			Call(FN_##_FunctionName,p1);\
		}\
	};

#define SPARK_SINGLE_EVENT_NOTIFICATION_BUS_AND_HANDLER_2(_FunctionName,_ClassGuid,param1,param2)\
	SPARK_SINGLE_EVENT_NOTIFICATION_BUS(_FunctionName,param1,param2)\
	class _FunctionName##NotificationBusHandler: public _FunctionName##NotificationBus::Handler, public AZ::BehaviorEBusHandler{\
	public:\
		AZ_EBUS_BEHAVIOR_BINDER(_FunctionName##NotificationBusHandler, _ClassGuid, AZ::SystemAllocator, _FunctionName);\
		void _FunctionName(param1 p1,param2 p2) {\
			Call(FN_##_FunctionName,p1,p2);\
		}\
	};

#define SPARK_SINGLE_EVENT_NOTIFICATION_BUS_AND_HANDLER_3(_FunctionName,_ClassGuid,param1,param2,param3)\
	SPARK_SINGLE_EVENT_NOTIFICATION_BUS(_FunctionName,param1,param2,param3)\
	class _FunctionName##NotificationBusHandler: public _FunctionName##NotificationBus::Handler, public AZ::BehaviorEBusHandler{\
	public:\
		AZ_EBUS_BEHAVIOR_BINDER(_FunctionName##NotificationBusHandler, _ClassGuid, AZ::SystemAllocator, _FunctionName);\
		void _FunctionName(param1 p1,param2 p2,param3 p3) {\
			Call(FN_##_FunctionName,p1,p2,p3);\
		}\
	};

	//SPARK_SINGLE_EVENT_NOTIFICATION_BUS_AND_HANDLER_1(OnDamageDealt,"{FEE1F3A4-3B33-4DCC-917E-9DC646B120DF}",Damage)	
	

}
