#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/ComponentBus.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Math/Vector3.h>

#include <GridMate/Serialize/DataMarshal.h>

#include <AzCore/RTTI/BehaviorContext.h>
#include "Utils/CommonTypes.h"
#include "Utils/Amount.h"

namespace spark {

	

	class CastingBehavior {
		
	public:
		AZ_TYPE_INFO(CastingBehavior, "{0A2FB698-CD2F-4E14-9B07-D03FDD0301B1}");
		static void Reflect(AZ::ReflectContext* reflection);

		enum {
			NONE = 0
		};

		enum TargetingBehavior {
			PASSIVE = BIT(1),
			IMMEDIATE = BIT(2),
			POINT_TARGET = BIT(3),
			UNIT_TARGET = BIT(4),
			DIRECTION_TARGET = POINT_TARGET | BIT(5),
			VECTOR_TARGET = BIT(6),
			TARGETING_BEHAVIOR_MASK = BIT(7) - 1,
		};

		enum TargetTeam {
			FRIENDLY = BIT(7),
			ENEMY = BIT(8),
			BOTH = (FRIENDLY | ENEMY),
			TARGET_TEAM_MASK = BOTH,
		};

		enum TargetType {
			HERO = BIT(9),
			CREEP = BIT(10),
			ANCIENT = BIT(11),
			BUILDING = BIT(12),
			MECHANICAL = BIT(13),
			COURIER = BIT(14),
			TREE = BIT(15),
			RUNE = BIT(16),
			NORMAL_UNIT = (HERO | CREEP | MECHANICAL),
			UNIT = (NORMAL_UNIT | ANCIENT),
			ALL = (UNIT | BUILDING | COURIER | TREE | RUNE),
			TARGET_TYPE_MASK = ALL,
		};

		CastingBehavior() {}
		CastingBehavior(int flags);

		int GetFlags() const;
		bool Contains(int flag) const;

		AZ_FORCE_INLINE bool operator== (const CastingBehavior& other) const
		{
			return other.m_flags == m_flags;
		}
		AZ_FORCE_INLINE bool operator!= (const CastingBehavior& other) const
		{
			return other.m_flags != m_flags;
		}
	private:
		int m_flags = NONE;
	};


	

	class AbilityRequests
		: public AZ::ComponentBus
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;

		virtual void SetAbilityTypeId(AbilityTypeId) = 0;
		virtual AbilityTypeId GetAbilityTypeId() = 0;

		//v deprecated:  (use the InfoRequestBus instead)
		virtual AZStd::string   GetAbilityName() = 0;
		virtual AZStd::string   GetAbilityDescription() = 0;
		virtual void SetAbilityName(AZStd::string) = 0;
		virtual void SetAbilityDescription(AZStd::string) = 0;
		//^ deprecated 


		virtual CastingBehavior GetCastingBehavior() = 0;
		virtual void SetCastingBehavior(CastingBehavior) = 0;

		virtual void SetIconTexture(AZStd::string) = 0;
		virtual AZStd::string GetIconTexture() = 0;

		virtual void SetCosts(Costs costs) = 0;
		virtual Costs GetCosts() = 0;

		virtual void SetCooldown(float cooldown) = 0;

		virtual void SetLevel(int level) = 0;
		virtual int GetLevel() = 0;
		virtual UnitId GetCaster() = 0;
	};
	using AbilityRequestBus = AZ::EBus<AbilityRequests>;




	class AbilityNotifications
		: public AZ::ComponentBus
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides (Configuring this Ebus)
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;

		//////////////////////////////////////////////////////////////////////////
		virtual void OnAttached(UnitId unit) {}
		virtual void OnDetached(UnitId unit) {}

		// callback copied from elsewhere

		virtual void	OnSpellStartFilter() {}
		virtual void	OnSpellStart() {}					    //When cast time ends, resources have been spent - most abilities begin to do their work in this function. No return type, no parameters.
		virtual void    OnAbilityPhaseStartFilter() { }
		virtual void    OnAbilityPhaseStart() { }  //When cast time begins, resources have not been spent.
		virtual void	OnAbilityPhaseInterrupted() {}		    //When cast time is cancelled for any reason.No return type, no parameters.
		virtual void	OnChannelFinish(bool interrupted) {}    //When channel finishes, bInterrupted parameter notifies if the channel finished or not. No return type.
		virtual void	OnUpgradeFilter() {};
		virtual void	OnUpgrade() {};						    //When the ability is leveled up.No parameters, no return type.

	};
	using AbilityNotificationBus = AZ::EBus<AbilityNotifications>;



	class AbilitiesNotifications
		: public AZ::EBusTraits
	{
	public:
		// Multiple handlers. Events received in undefined order.
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		// The EBus uses a single address.
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;	


		virtual void OnAbilityCreated(AbilityId id, AbilityTypeId type) {}


		virtual void OnAttached(AbilityId id, UnitId unit) {}
		virtual void OnDetached(AbilityId id, UnitId unit) {}

		// callback copied from elsewhere
		virtual void	OnSpellStartFilter(AbilityId id) {}
		virtual void	OnSpellStart(AbilityId id) {}					    //When cast time ends, resources have been spent - most abilities begin to do their work in this function. No return type, no parameters.
		virtual void    OnAbilityPhaseStartFilter(AbilityId id) { }
		virtual void    OnAbilityPhaseStart(AbilityId id) { }  //When cast time begins, resources have not been spent.Return true for successful cast, or false for unsuccessful, no parameters.
		virtual void	OnAbilityPhaseInterrupted(AbilityId id) {}		    //When cast time is cancelled for any reason.No return type, no parameters.
		virtual void	OnChannelFinish(AbilityId id,bool interrupted) {}    //When channel finishes, bInterrupted parameter notifies if the channel finished or not. No return type.
		virtual void	OnUpgradeFilter(AbilityId id) {};
		virtual void	OnUpgrade(AbilityId id) {};
	};
	using AbilitiesNotificationBus = AZ::EBus<AbilitiesNotifications>;

	class AbilityTypeNotifications
		: public AZ::EBusTraits
	{
	public:
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		
		typedef AbilityTypeId BusIdType;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;	

		
		virtual void OnAbilityCreated(AbilityId id) {}
	};
	using AbilityTypeNotificationBus = AZ::EBus<AbilityTypeNotifications>;



	class AbilityNotificationBusHandler
		: public AbilityNotificationBus::Handler
		, public AZ::BehaviorEBusHandler
	{
	public:
		AZ_EBUS_BEHAVIOR_BINDER(AbilityNotificationBusHandler, "{77FB4B70-DE91-446C-806C-74B254B496B6}", AZ::SystemAllocator,
			OnAttached,OnDetached,OnSpellStart, OnSpellStartFilter, OnAbilityPhaseStartFilter,OnAbilityPhaseStart,OnAbilityPhaseInterrupted,OnChannelFinish,OnUpgrade,OnUpgradeFilter);
		
		void OnAttached(UnitId unit)
		{
			Call(FN_OnAttached,unit);
		}
		void OnDetached(UnitId unit)
		{
			Call(FN_OnDetached,unit);
		}

		//callback copied from elsewhere
		void OnSpellStartFilter()
		{
			Call(FN_OnSpellStartFilter);
		}
		void OnSpellStart() 
		{
			Call(FN_OnSpellStart);
		}
		void OnAbilityPhaseStartFilter()
		{
			Call(FN_OnAbilityPhaseStartFilter);
		}
		void OnAbilityPhaseStart() 
		{
			Call(FN_OnAbilityPhaseStart);
		} 
		void OnAbilityPhaseInterrupted()
		{
			Call(FN_OnAbilityPhaseInterrupted);
		}		  
		void OnChannelFinish(bool interrupted) 
		{
			Call(FN_OnChannelFinish,interrupted);
		} 
		void OnUpgradeFilter()
		{
			Call(FN_OnUpgradeFilter);
		}
		void OnUpgrade() 
		{
			Call(FN_OnUpgrade);
		}			
	};


	class AbilitiesNotificationBusHandler
		: public AbilitiesNotificationBus::Handler
		, public AZ::BehaviorEBusHandler
	{
	public:
		AZ_EBUS_BEHAVIOR_BINDER(AbilitiesNotificationBusHandler, "{06E90802-7A82-4CAD-B90A-8ACD7AFB52CF}", AZ::SystemAllocator, OnAbilityCreated ,
			OnAttached, OnDetached, OnSpellStartFilter, OnSpellStart, OnAbilityPhaseStartFilter, OnAbilityPhaseStart, OnAbilityPhaseInterrupted, OnChannelFinish, OnUpgradeFilter,OnUpgrade);

		void OnAbilityCreated(AbilityId id, AbilityTypeId type)
		{
			Call(FN_OnAbilityCreated, id,type);
		}

		void OnAttached(AbilityId id, UnitId unit)
		{
			Call(FN_OnAttached, id,unit);
		}
		void OnDetached(AbilityId id, UnitId unit)
		{
			Call(FN_OnDetached, id, unit);
		}

		// callback copied from elsewhere
		void OnSpellStartFilter(AbilityId id) 
		{
			Call(FN_OnSpellStartFilter, id);
		}
		void OnSpellStart(AbilityId id)
		{
			Call(FN_OnSpellStart, id);
		}
		void OnAbilityPhaseStartFilter(AbilityId id)
		{
			Call(FN_OnAbilityPhaseStartFilter, id);
		}
		void OnAbilityPhaseStart(AbilityId id)
		{
			Call(FN_OnAbilityPhaseStart, id);
		}
		void OnAbilityPhaseInterrupted(AbilityId id)
		{
			Call(FN_OnAbilityPhaseInterrupted, id);
		}
		void OnChannelFinish(AbilityId id, bool interrupted)
		{
			Call(FN_OnChannelFinish, id);
		}
		void OnUpgradeFilter(AbilityId id)
		{
			Call(FN_OnUpgradeFilter, id);
		}
		void OnUpgrade(AbilityId id)
		{
			Call(FN_OnUpgrade, id);
		}
	};

	class AbilityTypeNotificationBusHandler
		: public AbilityTypeNotificationBus::Handler
		, public AZ::BehaviorEBusHandler
	{
	public:
		AZ_EBUS_BEHAVIOR_BINDER(AbilityTypeNotificationBusHandler, "{06CE24C4-F710-4E0D-B972-D6192CF5613A}", AZ::SystemAllocator, OnAbilityCreated);

		void OnAbilityCreated(AbilityId id)
		{
			Call(FN_OnAbilityCreated, id);
		}
	};


	struct CastContext
	{
		AZ_TYPE_INFO(ProjectileInfo, "{BBAECB0D-F629-4B18-8D50-85E8B72F05B7}");
		static void Reflect(AZ::ReflectContext* reflection);

		bool isItem;
		UnitId caster;
		UnitId target;
		AZ::Vector3 cursorPosition;

		AZ::EntityId ability;
		CastingBehavior behaviorUsed;
	};

	class CastContextRequests
		: public AZ::ComponentBus
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;

		virtual bool IsItem() = 0;
		virtual UnitId GetCaster() = 0;
		virtual UnitId GetCursorTarget() = 0;
		virtual AZ::Vector3 GetCursorPosition() = 0;

		virtual CastingBehavior GetBehaviorUsed() = 0;
	};
	using CastContextRequestBus = AZ::EBus<CastContextRequests>;

}

// marshaler
template<>
class GridMate::Marshaler<spark::CastingBehavior>
{
public:
	typedef spark::CastingBehavior DataType;

	AZ_FORCE_INLINE void Marshal(WriteBuffer& wb, const DataType& value) const
	{
		Marshaler<int> intMarshaler;

		intMarshaler.Marshal(wb, value.GetFlags());
	}
	AZ_FORCE_INLINE void Unmarshal(DataType& value, ReadBuffer& rb) const
	{
		Marshaler<int> intMarshaler;
		int flags;
		intMarshaler.Unmarshal(flags, rb);

		value = spark::CastingBehavior(flags);
		AZ_Printf(0, "Unmarshal<CastingBehavior> : %d", value.GetFlags());
	}
};
