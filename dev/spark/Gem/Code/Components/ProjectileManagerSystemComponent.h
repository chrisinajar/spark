#pragma once


#include "Busses/ProjectileManagerBus.h"
#include <AzCore/std/containers/queue.h>

#include <AzCore/std/containers/list.h>
#include <AzFramework/Entity/GameEntityContextBus.h>

namespace spark
{

	class ProjectileManagerSystemComponent
		: public AZ::Component
		, public ProjectileManagerRequestBus::Handler
		, public AzFramework::SliceInstantiationResultBus::MultiHandler
	{
	public:
		AZ_COMPONENT(ProjectileManagerSystemComponent, "{45F3E27D-A255-4DA6-84AC-E74839FF5CC1}");

		static void Reflect(AZ::ReflectContext* context);

		static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
		static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
		static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
		static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

	protected:

		////////////////////////////////////////////////////////////////////////
		// AZ::Component interface implementation
		void Init() override;
		void Activate() override;
		void Deactivate() override;
		////////////////////////////////////////////////////////////////////////

	public:

		void RegisterProjectileAsset(const ProjectileAsset &asset);
		ProjectileId CreateProjectile(ProjectileInfo);
		void FireProjectile(ProjectileInfo projectileInfo);
		void ResetProjectileManager();
		bool IsProjectileReleased (const ProjectileId);
		void ReleaseProjectile (const ProjectileId);

	private:
		void OnSliceInstantiated(const AZ::Data::AssetId& sliceAssetId, const AZ::SliceComponent::SliceInstanceAddress& instance);
		void OnSlicePreInstantiate(const AZ::Data::AssetId& sliceAssetId, const AZ::SliceComponent::SliceInstanceAddress& instance);


		AZStd::unordered_map< AzFramework::SliceInstantiationTicket, ProjectileInfo > m_projectileSlices;
		AZStd::unordered_map< ProjectileId, ProjectileInfo > m_projectiles;
		ProjectileId m_projectileId = 1;
	};


}