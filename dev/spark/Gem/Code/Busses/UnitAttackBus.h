#pragma once

#include <AzCore/Component/ComponentBus.h>
#include "Busses/ProjectileBus.h"


namespace spark {


	class UnitAttackRequests
		: public AZ::ComponentBus
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		//////////////////////////////////////////////////////////////////////////

		// Put your public methods here
		virtual void Attack(AZ::EntityId target) = 0;
		virtual void Stop() = 0;

		virtual float GetMaxRange() = 0;
		virtual float GetAttackPoint() = 0;
		virtual float GetBackswingPoint() = 0;
		virtual float GetIdlePoint() = 0;

		virtual float GetBaseAttackPoint() = 0;
		virtual float GetBaseBackswingPoint() = 0;
		virtual float GetBaseIdlePoint() = 0;

		virtual float GetIncreasedAttackSpeed() = 0;
		virtual void  SetIncreasedAttackSpeed(float value) = 0;

		virtual bool IsMelee() = 0;
		virtual void SetMelee(bool) = 0;

		virtual void SetProjectileAsset(ProjectileAsset) = 0;
		virtual ProjectileAsset GetProjectileAsset() = 0;
	};
	using UnitAttackRequestBus = AZ::EBus<UnitAttackRequests>;

	class UnitAttackNotifications
		: public AZ::ComponentBus
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		//////////////////////////////////////////////////////////////////////////

		// Put your public methods here
		virtual void OnAttackingInterrupted() {}
	};
	using UnitAttackNotificationBus = AZ::EBus<UnitAttackNotifications>;

}