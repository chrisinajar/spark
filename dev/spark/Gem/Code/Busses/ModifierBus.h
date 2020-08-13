#pragma once

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/ComponentBus.h>
#include "Utils/CommonTypes.h"

namespace spark {
	
	class Dispel{
	public:

		AZ_TYPE_INFO(Dispel, "{E08AF657-BA24-4D2D-8E7B-895C0D98BF65}");
		static void Reflect(AZ::ReflectContext* reflection);

		enum {
			DISPEL_FLAG_TOTAL = BIT(1),
			DISPEL_FLAG_DEATH = BIT(2),

			DISPEL_LEVEL_BASIC  = 1,
			DISPEL_LEVEL_STRONG = 128,
			DISPEL_LEVEL_MAX = 255,
		};

		unsigned char level = 1;
		unsigned char flag = 0;

		Dispel() {}

		bool operator==(const Dispel &other) const { return other.level == level && other.flag == flag; }
		bool operator!=(const Dispel &other) const { return !((*this) == other); }

		AZStd::string ToString() const;

		static Dispel DeathDispel()
		{
			Dispel d;
			d.level = 255;
			d.flag = DISPEL_FLAG_DEATH;
			return d;
		}
	};

	class ModifierRequests
		: public AZ::ComponentBus
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;

		virtual void SetModifierTypeId(ModifierTypeId) = 0;
		virtual ModifierTypeId GetModifierTypeId() = 0;

		virtual void SetIconTexture(AZStd::string) = 0;
		virtual AZStd::string GetIconTexture() = 0;

		virtual bool IsVisible() = 0;
		virtual void SetVisible(bool) = 0;

		virtual void SetCaster(UnitId) = 0;
		virtual UnitId GetCaster() = 0;

		virtual void SetParent(AZ::EntityId) = 0;
		virtual AZ::EntityId GetParent() = 0;

		virtual void SetAbility(AbilityId) = 0;
		virtual AbilityId GetAbility() = 0;

		virtual void SetParticle(AZStd::string) = 0;
		virtual AZStd::string GetParticle() = 0;

		virtual bool DoesStack() = 0;
		virtual void SetDoesStack(bool) = 0;

		virtual void Destroy() = 0;

		//aura
		virtual bool IsAura() = 0;
		virtual void SetAuraRadius(float radius) = 0;
		virtual float GetAuraRadius() = 0;
		virtual void RemoveAura() = 0;

		virtual AZStd::string ToString() = 0;


		//c++ only
		virtual void IncrementReferenceCounter() = 0;
		virtual void DecrementReferenceCounter() = 0;
		virtual int GetReferenceCounter() = 0;

		virtual void SetDuration(float duration) = 0;

	};
	using ModifierRequestBus = AZ::EBus<ModifierRequests>;


	class ModifierNotifications
        : public AZ::ComponentBus
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
        //////////////////////////////////////////////////////////////////////////

		virtual void OnAttached(UnitId){}
		virtual void OnDetached(UnitId){}
		virtual void OnDestroy(){}
		virtual bool IsDispellable(Dispel) { return true; }
	};
	using ModifierNotificationBus = AZ::EBus<ModifierNotifications>;



	class ModifiersNotifications
		: public AZ::EBusTraits
	{
	public:
		// Multiple handlers. Events received in undefined order.
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		// The EBus uses a single address.
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;	


		virtual void OnModifierCreated(ModifierId id, ModifierTypeId type) {}
	};
	using ModifiersNotificationBus = AZ::EBus<ModifiersNotifications>;

	class ModifierTypeNotifications
		: public AZ::EBusTraits
	{
	public:
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		
		typedef ModifierTypeId BusIdType;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;	

		
		virtual void OnModifierCreated(ModifierId id) {}
	};
	using ModifierTypeNotificationBus = AZ::EBus<ModifierTypeNotifications>;



	class AuraNotifications
		: public AZ::ComponentBus
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		//////////////////////////////////////////////////////////////////////////

		virtual bool IsUnitAffected(UnitId) { return true; }
	};
	using AuraNotificationBus = AZ::EBus<AuraNotifications>;
}
