#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/ComponentBus.h>
#include <AzFramework/Network/NetBindable.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Math/Vector3.h>

#include <AzCore/Script/ScriptAsset.h>
#include <AzCore/Script/ScriptContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/std/containers/vector.h>

#include "Busses/AbilityBus.h"
#include "Busses/LocalizationBus.h"

namespace spark {

	/**
	*	To attach to an ability entity
	*/
	class AbilityComponent
		: public AZ::Component
		, protected AbilityRequestBus::Handler
		, protected AbilityNotificationBus::Handler
		, public AzFramework::NetBindable
		, public LocalizationNotificationBus::Handler
	{
	public:
		AZ_COMPONENT(AbilityComponent, "{59E14373-5285-4CA5-8F63-C1F11B6DAA8A}", AzFramework::NetBindable)

		~AbilityComponent() override {};

		static void Reflect(AZ::ReflectContext* reflection);

		static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

		void Init() override;
		void Activate() override;
		void Deactivate() override;

		void SetAbilityTypeId(AbilityTypeId);
		AbilityTypeId GetAbilityTypeId();

		AZStd::string GetAbilityName() override;
		AZStd::string GetAbilityDescription() override;
		void SetAbilityName(AZStd::string) override;
		void SetAbilityDescription(AZStd::string) override;

		void SetIconTexture(AZStd::string);
		AZStd::string GetIconTexture();

		CastingBehavior GetCastingBehavior();
		void SetCastingBehavior(CastingBehavior);

		void SetCosts(Costs costs);
		Costs GetCosts();

		void SetCooldown(float cooldown);

		void SetLevel(int level);
		int GetLevel();
		UnitId GetCaster() override;

		void OnAttached(UnitId unit) override;
		void OnDetached(UnitId unit) override;

		GridMate::ReplicaChunkPtr GetNetworkBinding() override;
		void SetNetworkBinding(GridMate::ReplicaChunkPtr chunk) override;
		void UnbindFromNetwork() override;
		// event handlers, setters already exist
		void OnNewAbilityName (const AZStd::string& abilityName, const GridMate::TimeContext& tc);
		void OnNewAbilityDescription (const AZStd::string& abilityDescription, const GridMate::TimeContext& tc);
		void OnNewTypeId (const AbilityTypeId& typeId, const GridMate::TimeContext& tc);
		void OnNewIconTexture (const AZStd::string& iconTexture, const GridMate::TimeContext& tc);
		void OnNewCastingBehavior (const CastingBehavior& castingBehavior, const GridMate::TimeContext& tc);
		void OnNewCosts (const Costs& costs, const GridMate::TimeContext& tc);
		void OnNewLevel (const int& level, const GridMate::TimeContext& tc);
		void OnNewCaster (const UnitId& caster, const GridMate::TimeContext& tc);

		void OnLanguageChanged(const AZStd::string &language);
	public:
		AZStd::string m_abilityName;
		AZStd::string m_abilityDescription;

		AbilityTypeId m_typeId;
		AZStd::string m_iconTexture;
		CastingBehavior m_castingBehavior;
		Costs m_costs;
		int m_level = 0;
		UnitId m_caster;

		GridMate::ReplicaChunkPtr m_replicaChunk = nullptr;
	};


}
