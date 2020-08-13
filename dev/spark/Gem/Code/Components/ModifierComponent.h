#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/ComponentBus.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Math/Vector3.h>
#include <AzFramework/Network/NetBindable.h>


#include <AzCore/Script/ScriptAsset.h>
#include <AzCore/Script/ScriptContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/std/containers/vector.h>

#include "Busses/ModifierBus.h"
#include "Busses/CooldownBus.h"

#include <LmbrCentral/Scripting/TriggerAreaComponentBus.h>

namespace spark {

	/**
	*	To attach to an ability entity
	*/
	class ModifierComponent
		: public AZ::Component
		, public AzFramework::NetBindable
		, protected AZ::TickBus::Handler
		, protected ModifierRequestBus::Handler
		, protected CooldownNotificationBus::Handler
		, protected ModifierNotificationBus::MultiHandler
		, protected LmbrCentral::TriggerAreaNotificationBus::Handler
	{
	public:
		AZ_COMPONENT(ModifierComponent, "{EDE338AD-6960-44CF-AB89-941341006A2E}", AzFramework::NetBindable)

		static void Reflect(AZ::ReflectContext* reflection);

		static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

		void Init() override;
		void Activate() override;
		void Deactivate() override;

		void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

		//NetBindable
		GridMate::ReplicaChunkPtr GetNetworkBinding ();
		void SetNetworkBinding (GridMate::ReplicaChunkPtr chunk);
		void UnbindFromNetwork ();
		// handlers
		void OnNewParent (const AZ::EntityId &parent, const GridMate::TimeContext&);
		void OnNewAbility (const AZ::EntityId &ability, const GridMate::TimeContext&);
		void OnNewCaster (const AZ::EntityId &caster, const GridMate::TimeContext&);
		void OnNewVisible (const bool &visible, const GridMate::TimeContext&);
		void OnNewIconTexture (const AZStd::string &iconTexture, const GridMate::TimeContext&);
		void OnNewModifierTypeId (const ModifierTypeId &modifierTypeId, const GridMate::TimeContext&);
		void OnNewAuraRange (const float &auraRange, const GridMate::TimeContext&);
		void OnNewParticle (const AZStd::string &particle, const GridMate::TimeContext&);

		void SetModifierTypeId(ModifierTypeId);
		ModifierTypeId GetModifierTypeId();

		void SetIconTexture(AZStd::string);
		AZStd::string GetIconTexture();

		bool IsVisible();
		void SetVisible(bool);

		bool DoesStack();
		void SetDoesStack(bool);


		void SetCaster(UnitId);
		UnitId GetCaster();

		void SetParent(AZ::EntityId);
		AZ::EntityId GetParent();

		void SetAbility(AbilityId);
		AbilityId GetAbility();

		void SetParticle(AZStd::string);
		AZStd::string GetParticle();

		void Destroy();

		//aura
		bool IsAura();
		void SetAuraRadius(float radius);
		float GetAuraRadius();
		void RemoveAura();

		AZStd::string ToString();

		//c++ only
		void IncrementReferenceCounter();
		void DecrementReferenceCounter();
		int GetReferenceCounter();
		void SetDuration(float duration);
	protected:
		//cooldown notifications
		void OnCooldownFinished() override;

		 /// Sent when enteringEntityId enters this trigger.
		void OnTriggerAreaEntered(AZ::EntityId enteringEntityId) override;
        /// Sent when enteringEntityId exits this trigger.
		void OnTriggerAreaExited(AZ::EntityId exitingEntityId) override;

		void OnDetached(UnitId) override; //to handle the case when a modifier applied by an aura is dispelled within the aura -> need to be reapplied
		
	private:
		void DestroyAfterLingerDuration(ModifierId modifierId);

		GridMate::ReplicaChunkPtr m_replicaChunk;

		AZ::EntityId m_parent;
		AZ::EntityId m_ability;
		AZ::EntityId m_caster;
		bool m_visible = true;
		bool m_doesStack = false;
		int m_referenceCounter = 1;
		bool m_ignoreDeactivate = false;

		AZStd::string m_iconTexture;
		ModifierTypeId m_modifierTypeId;
		float m_auraRange = 0.0f;
		bool m_hasAuraComponents = false;
		AZStd::string m_particle;

		AZ::EntityId m_particleEntity;
		bool m_needsParticleEffect = false;

		bool m_active = false;
		
		AZStd::unordered_map<UnitId, ModifierId> m_auraModifiers;
	};


}
