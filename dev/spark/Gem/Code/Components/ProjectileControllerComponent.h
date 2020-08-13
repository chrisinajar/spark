#pragma once


#include <AzCore/Component/Component.h>
#include <AzCore/Component/ComponentBus.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Math/Vector3.h>
#include <LmbrCentral/Ai/NavigationComponentBus.h>
#include <AzFramework/Network/NetBindable.h>

#include "Busses/ProjectileBus.h"

namespace spark
{

	class ProjectileControllerComponent
		: public AZ::Component
		, public AZ::TickBus::Handler
		, protected ProjectileRequestBus::Handler
		, public AzFramework::NetBindable
	{
	public:
		AZ_COMPONENT(ProjectileControllerComponent, "{C512BF3A-A426-4CE2-8A51-1C47BCA30D09}", AzFramework::NetBindable)

		~ProjectileControllerComponent() override {};

		static void Reflect(AZ::ReflectContext* reflection);

		void Init() override;
		void Activate() override;
		void Deactivate() override;


		void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

		void Fire(ProjectileInfo projectileInfo) override;

		
		void Despawn();
		// Called during network binding on the master. Implementations should create and return a new binding.
		GridMate::ReplicaChunkPtr GetNetworkBinding() override;
		void SetNetworkBinding(GridMate::ReplicaChunkPtr chunk) override;

		// Called when network is unbound. Implementations should release their references to the binding.
		void UnbindFromNetwork() override;

		void OnNewParticleEffect (const AZStd::string& particleEffect, const GridMate::TimeContext& tc);
		void SetParticleEffect (const AZStd::string& particleEffect);

		bool m_isNavEntity = false;
	private:
		ProjectileInfo m_projectileInfo;
		bool m_isMoving = false;
		bool m_hasHit = false;
		bool m_needsParticleEffect = false;

		GridMate::ReplicaChunkPtr m_replicaChunk = nullptr;
		AZ::EntityId m_particleEntity;
		AZStd::string m_particleEffect;
	};
}
