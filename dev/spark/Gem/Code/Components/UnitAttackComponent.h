#pragma once


#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/containers/array.h>

#include "Busses/UnitAttackBus.h"
#include "Busses/UnitNavigationBus.h"
#include "Utils/SSM.h"
#include "Busses/ProjectileManagerBus.h"

namespace spark
{

	class UnitAttackComponent
		: public AZ::Component
        , public AZ::TickBus::Handler
		, public UnitNavigationNotificationBus::Handler
		, protected UnitAttackRequestBus::Handler
	{
	public:
		AZ_COMPONENT(UnitAttackComponent, "{6018344E-535E-4E27-8785-6BF9AAF690AC}")

		~UnitAttackComponent() override {};

		static void Reflect(AZ::ReflectContext* reflection);

		static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
		static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
		static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
		static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

		void Init() override;
		void Activate() override;
		void Deactivate() override;

		void Attack(AZ::EntityId target) override;
		void Stop() override;

		float GetMaxRange();

		float GetAttackPoint();
		float GetBackswingPoint();
		float GetIdlePoint();

		float GetBaseAttackPoint();
		float GetBaseBackswingPoint();
		float GetBaseIdlePoint();

		float GetIncreasedAttackSpeed();
		void  SetIncreasedAttackSpeed(float value);

		bool IsMelee();
		void SetMelee(bool);

		void SetProjectileAsset(ProjectileAsset);
		ProjectileAsset GetProjectileAsset();

		static void SendAttackEvents(AttackInfo info);

	private:
		void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
		void OnMovingDone() override;
		void OnDestinationUnreachable() override;

		Value GetDamage();
		Value GetProjectileSpeed();
		

		void Attack();


		enum {
			S_IDLE,
			S_FORESWING,
			S_BACKSWING,
			S_MOVING
		};
		enum {
			E_UPDATE,
			E_STOP,
			E_TRAVERSAL_COMPLETE,
			E_DESTINATION_UNREACHABLE
		};
		SSM m_stateMachine;
		float m_elapsedTime;
		float m_timeSinceLastAttack;
		bool  m_attacking = false;
		AZ::EntityId m_target;
		bool OnStateIdle	 (SSM& m, const SSM::Event& e);
		bool OnStateForeswing(SSM& m, const SSM::Event& e);
		bool OnStateBackswing(SSM& m, const SSM::Event& e);
		bool OnStateMoving	 (SSM& m, const SSM::Event& e);

		void OnAttackingInterrupted();

		float m_maxRange=3;
		bool  m_isMelee=true;
		float m_baseAttackPoint=0.6;
		float m_baseBackswingPoint=0.4;
		float m_IAS=0;
		float m_projectileSpeed=10;
		AttackInfo m_attackInfo;
		ProjectileAsset m_projectileAsset="projectile_001";
	};

}