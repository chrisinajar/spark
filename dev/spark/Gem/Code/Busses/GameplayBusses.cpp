#include "spark_precompiled.h"

#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Serialization/SerializeContext.h>

#include "Busses/GameplayBusses.h"
#include "Busses/UnitBus.h"
#include "ProjectileBus.h"


namespace spark {


	void DamageScriptConstructor(Damage* self, AZ::ScriptDataContext& dc)
	{
		AZ_Printf(0,"called DamageScriptConstructor");
		if (dc.GetNumArguments() == 0)
		{
			new(self) Damage();
			return;
		}

		dc.GetScriptContext()->Error(AZ::ScriptContext::ErrorType::Error, true, "Invalid arguments passed to Damage().");
		new(self) Damage();
	}

	void AttackInfoScriptConstructor(AttackInfo* self, AZ::ScriptDataContext& dc)
	{
		AZ_Printf(0,"called AttackInfoScriptConstructor");
		if (dc.GetNumArguments() == 0)
		{
			new(self) AttackInfo();
			return;
		}

		dc.GetScriptContext()->Error(AZ::ScriptContext::ErrorType::Error, true, "Invalid arguments passed to AttackInfo().");
		new(self) AttackInfo();
	}	


	void Damage::Reflect(AZ::ReflectContext * reflection)
	{
		if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
		{
			serializationContext->Class<Damage>()
				//->Version(1)
				->Field("type", &Damage::type)
				->Field("flag", &Damage::flag)
				->Field("damage", &Damage::damage)
				->Field("source", &Damage::source)
				->Field("target", &Damage::target)
				->Field("ability", &Damage::ability);

			//if (auto editContext = serializationContext->GetEditContext())
			//{
			//	editContext->Class<Damage>("Damage", "")
			//		->DataElement(nullptr, &Damage::type, "type", "")
			//		->DataElement(nullptr, &Damage::lethal, "lethal", "")
			//		->DataElement(nullptr, &Damage::damage, "damage", "");
			//}
		}

		if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
		{
			behaviorContext->Class<Damage>("Damage")
				->Attribute(AZ::Script::Attributes::Storage, AZ::Script::Attributes::StorageType::Value)
				->Attribute(AZ::Script::Attributes::ConstructorOverride, &DamageScriptConstructor)
				->Property("type", BehaviorValueProperty(&Damage::type))
				->Property("flag", BehaviorValueProperty(&Damage::flag))
				->Property("damage", BehaviorValueProperty(&Damage::damage))
				->Property("source", BehaviorValueProperty(&Damage::source))
				->Property("target", BehaviorValueProperty(&Damage::target))
				->Property("ability", BehaviorValueProperty(&Damage::ability))
				->Enum<(int)DAMAGE_TYPE_PHYSICAL>("DAMAGE_TYPE_PHYSICAL")
				->Enum<(int)DAMAGE_TYPE_MAGICAL>("DAMAGE_TYPE_MAGICAL")
				->Enum<(int)DAMAGE_TYPE_PURE>("DAMAGE_TYPE_PURE")
				->Enum<(int)DAMAGE_TYPE_KILL>("DAMAGE_TYPE_KILL")
				->Enum<(int)DAMAGE_FLAG_NON_LETHAL>("DAMAGE_FLAG_NON_LETHAL")
				->Enum<(int)DAMAGE_FLAG_CRITICAL>("DAMAGE_FLAG_CRITICAL")
				->Method("ToString",&Damage::ToString);
		}

	}
	AZStd::string Damage::ToString() const
	{
		const char* types[] = { "DAMAGE_TYPE_PHYSICAL","DAMAGE_TYPE_MAGICAL","DAMAGE_TYPE_PURE","DAMAGE_TYPE_KILL" };
		return AZStd::string::format("Damage(type=%s,flag=%s,damage=%f,source=%s,target=%s)", type<3 ? types[(int)type] : "DAMAGE_TYPE_UNKNOWN", flag, damage, source.IsValid() ? source.ToString().c_str() : "invalid", target.IsValid() ? target.ToString().c_str() : "invalid");
	}

	void AttackInfo::Reflect(AZ::ReflectContext * reflection)
	{
		if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
		{
			serializationContext->Class<AttackInfo>()
				//->Version(1)
				->Field("attacker", &AttackInfo::attacker)
				->Field("attacked", &AttackInfo::attacked)
				->Field("damage", &AttackInfo::damage);
		}

		if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
		{
			behaviorContext->Class<AttackInfo>("AttackInfo")
				->Attribute(AZ::Script::Attributes::Storage, AZ::Script::Attributes::StorageType::ScriptOwn)
				->Attribute(AZ::Script::Attributes::ConstructorOverride, &AttackInfoScriptConstructor)
				->Property("attacker", BehaviorValueGetter(&AttackInfo::attacker), nullptr)
				->Property("attacked", BehaviorValueGetter(&AttackInfo::attacked), nullptr)
				->Property("damage", BehaviorValueProperty(&AttackInfo::damage))
				->Method("ToString",&AttackInfo::ToString);
		}

	}
	AZStd::string AttackInfo::ToString() const
	{
		return AZStd::string::format("AttackInfo(attacker=%s,attacked=%s,damage=%f)",  attacker.IsValid()?attacker.ToString().c_str():"invalid", attacked.IsValid()?attacked.ToString().c_str():"invalid",damage);
	}





	void OnDamageTakenNotifications::FireOnDamageTakenFilter(Damage &damage, bool &hasBeenConsumed)
	{
		if (hasBeenConsumed)return;

		AZ_Warning("", false, "FireOnDamageTaken called");

		FilterResultTuple<Damage> result;
		result.Push();
		this->OnDamageTakenFilter_(damage); 
		result.Pop();

		switch(result.action)
		{
		case FilterResult::FILTER_PREVENT:
		{
			hasBeenConsumed = true;
			auto top = FilterResult::GetStackTop();
			if (top)top->action = FilterResult::FILTER_PREVENT;
			break;
		}
		case FilterResult::FILTER_MODIFY:
			result.CopyTo(damage);
		}
		
	}

	// this handler should probably be in unit component?
	Value OnDamageTakenRequests::ApplyDamage(Damage damage) {
		AZ_Warning("", false, "ApplyDamage called");
		const AZ::EntityId id= (*OnDamageTakenRequestBus::GetCurrentBusId());

		FilterResultTuple<Damage> result;
		result.Push();
		bool consumed = false;
		EBUS_EVENT_ID(id, OnDamageTakenNotificationBus, FireOnDamageTakenFilter, damage, consumed);
		result.Pop();

		if (result.action == FilterResult::FILTER_PREVENT)return 0;

		//Remove HPs
		Value damage_multiplier = 1;
		switch (damage.type) 
		{
		case Damage::DAMAGE_TYPE_PHYSICAL:
		{
			Value armor = 0;
			EBUS_EVENT_RESULT(armor, VariableManagerRequestBus, GetValue, VariableId(id, "armor_reduction"));
			damage_multiplier = armor;
		}break;
		case Damage::DAMAGE_TYPE_MAGICAL:
		{
			Value magic_resistance = 0;
			EBUS_EVENT_RESULT(magic_resistance, VariableManagerRequestBus, GetValue, VariableId(id, "magic_resistance"));
			damage_multiplier = 1.0f - magic_resistance;
		}break;
		case Damage::DAMAGE_TYPE_PURE: break;
		}
		Value hp = 0;
		EBUS_EVENT_RESULT(hp, VariableManagerRequestBus, GetValue, VariableId(id, "hp"));
		Value pre = hp;

		Value hp_removed = damage_multiplier * damage.damage;

		hp = AZStd::max(0.0f, hp - hp_removed);

		if (pre > 0.0f && hp == 0.0f && (damage.flag & Damage::DAMAGE_FLAG_NON_LETHAL))
		{
			hp = 1.0f;
		}

		hp_removed = pre - hp;
		m_lastDamage = damage;

		AZ_Warning("UnitComponent", false, "[%s]That's lotta damage! %s\thp removed=%f (%f->%f)", id.ToString().c_str(), damage.ToString().c_str(), damage_multiplier * damage.damage, pre, hp);
		EBUS_EVENT_ID(id, OnDamageTakenNotificationBus, OnDamageTaken, damage, hp_removed);

		EBUS_EVENT(VariableManagerRequestBus, SetValue, VariableId(id, "hp"), hp);

		if (damage.source.IsValid())
		{
			EBUS_EVENT_ID(damage.source, OnDamageDealtNotificationBus, OnDamageDealt, damage, hp_removed);
		}
		
		if (pre > 0.0f && hp == 0.0f && !(damage.flag & Damage::DAMAGE_FLAG_NON_LETHAL))
		{
			// killed dead
			EBUS_EVENT_ID(id, UnitRequestBus, Kill, damage);
		}

		return hp_removed;
	}


	//class OnDamageTakenNotificationBusHandler
	//	: public OnDamageTakenNotificationBus::Handler
	//	, public AZ::BehaviorEBusHandler
	//{
	//public:
	//	AZ_EBUS_BEHAVIOR_BINDER(OnDamageTakenNotificationBusHandler, "{9241E855-E07E-432F-8CAD-200B1CDDD9C4}", AZ::SystemAllocator, OnDamageTaken);

	//	bool OnDamageTaken(Damage &damage) {
	//		
	//		bool result=true;
	//		CallResult(result, FN_OnDamageTaken, damage);
	//		AZ_Printf(0, "OnDamageTakenNotificationBusHandler::OnDamageTaken()   result=%s", result ? "true" : "false");
	//		return result;
	//	}
	//};
	//this is just AZ_EBUS_BEHAVIOR_BINDER without the constructor, used for debugging purposes
	#define CUSTOM_EBUS_BEHAVIOR_BINDER(_Handler,_Uuid,_Allocator,...)\
	AZ_CLASS_ALLOCATOR(_Handler,_Allocator,0)\
    AZ_RTTI(_Handler,_Uuid,AZ::BehaviorEBusHandler)\
    typedef _Handler ThisType;\
    enum {\
        AZ_SEQ_FOR_EACH(AZ_BEHAVIOR_EBUS_FUNC_ENUM, AZ_EBUS_SEQ(__VA_ARGS__))\
        FN_MAX\
    };\
    int GetFunctionIndex(const char* functionName) const override {\
        AZ_SEQ_FOR_EACH(AZ_BEHAVIOR_EBUS_FUNC_INDEX, AZ_EBUS_SEQ(__VA_ARGS__))\
        return -1;\
    }\
    void Disconnect() override {\
        BusDisconnect();\
    }\
    bool Connect(AZ::BehaviorValueParameter* id = nullptr) override {\
        return AZ::Internal::EBusConnector<_Handler>::Connect(this, id);\
    }



	class OnDamageTakenNotificationBusHandler
		: public OnDamageTakenNotificationBus::Handler
		, public AZ::BehaviorEBusHandler
	{
	public:
		AZ_EBUS_BEHAVIOR_BINDER(OnDamageTakenNotificationBusHandler, "{9241E855-E07E-432F-8CAD-200B1CDDD9C4}", AZ::SystemAllocator, OnDamageTakenFilter_, OnDamageTaken);
		void OnDamageTakenFilter_(Damage damage) {
			
			Call(FN_OnDamageTakenFilter_, damage);
		}
		void OnDamageTaken(Damage damage,Value hp_removed) {
			Call(FN_OnDamageTaken, damage, hp_removed);
		}
	};

	class OnDamageDealtNotificationBusHandler
		: public OnDamageDealtNotificationBus::Handler
		, public AZ::BehaviorEBusHandler
	{
	public:
		AZ_EBUS_BEHAVIOR_BINDER(OnDamageDealtNotificationBusHandler, "{13E58D19-A9CD-4697-94AE-E30E5236F805}", AZ::SystemAllocator, OnDamageDealtFilter_, OnDamageDealt);
		void OnDamageDealtFilter_(Damage damage) {

			Call(FN_OnDamageDealtFilter_, damage);
		}
		void OnDamageDealt(Damage damage, Value hp_removed) {
			Call(FN_OnDamageDealt, damage, hp_removed);
		}
	};



	void OnDamageTakenNotifications::Reflect(AZ::ReflectContext * reflection)
	{
		Damage::Reflect(reflection);

		if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
		{
			behaviorContext->EBus<OnDamageTakenRequestBus>("OnDamageTakenRequestBus")
				->Event("ApplyDamage", &OnDamageTakenRequestBus::Events::ApplyDamage);

			behaviorContext->EBus<OnDamageTakenNotificationBus>("OnDamageTakenNotificationBus")
				->Handler<OnDamageTakenNotificationBusHandler>()
				->Event("SetPriority", &OnDamageTakenNotificationBus::Events::SetPriority)
				->Event("GetPriority", &OnDamageTakenNotificationBus::Events::GetPriority)
				;
		}
	}

	void OnDamageDealtNotifications::Reflect(AZ::ReflectContext * reflection)
	{
		if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
		{
			behaviorContext->EBus<OnDamageDealtNotificationBus>("OnDamageDealtNotificationBus")
				->Handler<OnDamageDealtNotificationBusHandler>();
		}
	}

	

	class AttackEventsNotificationBusHandler
		: public AttackEventsNotificationBus::Handler
		, public AZ::BehaviorEBusHandler
	{
	public:
		AZ_EBUS_BEHAVIOR_BINDER(AttackEventsNotificationBusHandler, "{2034366A-4728-4331-9115-15FECA583769}", AZ::SystemAllocator, OnAttackEvent);

		void OnAttackEvent(AZStd::string eventId, AttackInfo info) {
			AZ_Printf(0, "AttackEventsNotificationBusHandler::OnAttackEvent");
			AZ_Printf(0, "eventId before : %s", eventId.c_str());
			Call(FN_OnAttackEvent, eventId, info);

			AZ_Printf(0, "eventId after : %s", eventId.c_str());
		}
	};
	void AttackEventsNotifications::Reflect(AZ::ReflectContext * reflection)
	{
		AttackInfo::Reflect(reflection);

		if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
		{
			behaviorContext->EBus<AttackEventsNotificationBus>("AttackEventsNotificationBus")
				->Handler<AttackEventsNotificationBusHandler>()
				->Event("OnAttackEvent", &AttackEventsNotificationBus::Events::OnAttackEvent);
		}
	}



	void ProjectileInfo::Reflect(AZ::ReflectContext* reflection)
	{
		if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
		{
			serializationContext->Class<ProjectileInfo>()
				->Version(1)
				->Field("startingPosition", &ProjectileInfo::startingPosition)
				->Field("attacker",  &ProjectileInfo::attacker)
				->Field("targetType",  &ProjectileInfo::targetType)
				->Field("targetEntity",  &ProjectileInfo::targetEntity)
				->Field("targetPosition",  &ProjectileInfo::targetPosition)
				->Field("asset",  &ProjectileInfo::asset)
				->Field("type",  &ProjectileInfo::type)
				->Field("speed",  &ProjectileInfo::speed)
				->Field("damage",  &ProjectileInfo::damage);

		}
		if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
		{
			behaviorContext->Class<ProjectileInfo>("ProjectileInfo")
				->Attribute(AZ::Script::Attributes::Storage, AZ::Script::Attributes::StorageType::Value)
				->Enum<(int)ProjectileInfo::TARGET_ENTITY>("TARGET_ENTITY")
				->Enum<(int)ProjectileInfo::TARGET_POSITION>("TARGET_POSITION")
				->Enum<(int)ProjectileInfo::NORMAL_ATTACK>("TYPE_NORMAL_ATTACK")
				->Enum<(int)ProjectileInfo::PROJECTILE>("TYPE_PROJECTILE")
				->Property("startingPosition", BehaviorValueProperty(&ProjectileInfo::startingPosition))
				->Property("attacker",  BehaviorValueProperty(&ProjectileInfo::attacker))
				->Property("targetType",  &ProjectileInfo::GetTargetType, nullptr)
				->Property("targetEntity",  BehaviorValueGetter(&ProjectileInfo::targetEntity), &ProjectileInfo::SetTargetEntity)
				->Property("targetPosition",  BehaviorValueGetter(&ProjectileInfo::targetPosition), &ProjectileInfo::SetTargetPosition)
				->Property("speed",  BehaviorValueProperty(&ProjectileInfo::speed))
				->Property("damage",  BehaviorValueProperty(&ProjectileInfo::damage))
				->Property("particle",  BehaviorValueProperty(&ProjectileInfo::particle))
				->Property("projectileId",  BehaviorValueProperty(&ProjectileInfo::projectileId))
				->Property("triggerRadius",  BehaviorValueProperty(&ProjectileInfo::triggerRadius))
				->Property("asset", BehaviorValueProperty(&ProjectileInfo::asset))
				// ->Property("type",  BehaviorValueProperty(&ProjectileInfo::type))
				;
		}
	}

	void ProjectileInfo::SetTargetEntity (AZ::EntityId id)
	{
		targetEntity = id;
		targetType = ProjectileInfo::TARGET_ENTITY;
	}
	void ProjectileInfo::SetTargetPosition (AZ::Vector3 position)
	{
		targetPosition = position;
		targetType = ProjectileInfo::TARGET_POSITION;
		AZ_Printf(0, "Setting target type to position!");
	}
	int ProjectileInfo::GetTargetType ()
	{
		return (int)targetType;
	}




	class OnRightClickedNotificationsHandler
		: public OnRightClickedNotificationBus::Handler
		, public AZ::BehaviorEBusHandler
	{
	public:
		AZ_EBUS_BEHAVIOR_BINDER(OnRightClickedNotificationsHandler, "{B6CBF76D-2B85-4CF5-9BB7-4CC72619DF42}", AZ::SystemAllocator, OnRightClickedFilter, OnRightClicked);

		void OnRightClickedFilter() {
			Call(FN_OnRightClickedFilter);
		}
		void OnRightClicked() {
			Call(FN_OnRightClicked);
		}
	};

	class OnRightClickedGlobalNotificationsHandler
		: public OnRightClickedGlobalNotificationBus::Handler
		, public AZ::BehaviorEBusHandler
	{
	public:
		AZ_EBUS_BEHAVIOR_BINDER(OnRightClickedGlobalNotificationsHandler, "{6A0B067F-0E68-4E9F-9396-B47B742E0593}", AZ::SystemAllocator, OnEntityRightClickedFilter, OnEntityRightClicked);

		void OnEntityRightClickedFilter(AZ::EntityId id) {
			Call(FN_OnEntityRightClickedFilter,id);
		}
		void OnEntityRightClicked(AZ::EntityId id) {
			Call(FN_OnEntityRightClicked,id);
		}
	};

	void OnRightClickedNotifications::Reflect(AZ::ReflectContext * reflection)
	{
		if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
		{
			behaviorContext->EBus<OnRightClickedNotificationBus>("OnRightClickedNotificationBus")
				->Handler<OnRightClickedNotificationsHandler>();
		}
	}
	void OnRightClickedGlobalNotifications::Reflect(AZ::ReflectContext * reflection)
	{
		if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
		{
			behaviorContext->EBus<OnRightClickedGlobalNotificationBus>("OnRightClickedGlobalNotificationBus")
				->Handler<OnRightClickedGlobalNotificationsHandler>();
		}
	}

	void ReflectGameplayBusses(AZ::ReflectContext * reflection)
	{
		OnRightClickedNotifications::Reflect(reflection);
		OnRightClickedGlobalNotifications::Reflect(reflection);
		OnDamageDealtNotifications::Reflect(reflection);
	}
}
