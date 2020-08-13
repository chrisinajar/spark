#pragma once

#include <AzCore/Slice/SliceAsset.h>
#include <AzCore/Component/Component.h>
#include "Utils/DynamicSliceWrapper.h"
#include "Busses/ProjectileBus.h"

namespace spark {

	

	class ProjectileManagerRequests
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;

		
		virtual void RegisterProjectileAsset(const ProjectileAsset&) = 0;
		virtual ProjectileId CreateProjectile(ProjectileInfo) = 0;
		virtual void FireProjectile(ProjectileInfo projectileInfo) = 0;

		virtual void ResetProjectileManager() = 0;
		virtual bool IsProjectileReleased (const ProjectileId) = 0;
		virtual void ReleaseProjectile (const ProjectileId) = 0;
	};
	using ProjectileManagerRequestBus = AZ::EBus<ProjectileManagerRequests>;

	class ProjectileManagerNotifications
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
		typedef ProjectileId BusIdType;
		//////////////////////////////////////////////////////////////////////////


		virtual void OnProjectileCreated (ProjectileInfo, AZ::EntityId) {};
	};
	using ProjectileManagerNotificationBus = AZ::EBus<ProjectileManagerNotifications>;
}