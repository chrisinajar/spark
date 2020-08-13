#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/std/string/string.h>
#include <AzCore/Slice/SliceAsset.h>
#include <AzCore/Math/Transform.h>

#include "GameplayBusses.h"

namespace spark {

	//using ProjectileAsset=AZ::Data::Asset<AZ::DynamicPrefabAsset>;
	//using ProjectileAssetId = AZ::Data::AssetId;
	using ProjectileAsset = AZStd::string;
	using Projectile = AZ::EntityId;
	using ProjectileId = uint32_t;


	struct ProjectileInfo {
		AZ_TYPE_INFO(ProjectileInfo, "{995AF801-290F-4BDF-ACD2-F23D818C7F91}");
		static void Reflect(AZ::ReflectContext* reflection);

		void SetTargetEntity (AZ::EntityId id);
		void SetTargetPosition (AZ::Vector3 position);
		int GetTargetType ();

		AZ::Vector3 startingPosition;
		AZ::EntityId attacker;

		enum TargetType {
			TARGET_ENTITY,
			TARGET_POSITION,
		} targetType=TARGET_ENTITY;
		AZ::EntityId targetEntity;
		AZ::Vector3 targetPosition;

		//ProjectileAssetId asset;
		ProjectileAsset asset;

		enum ProjectileType {
			NORMAL_ATTACK,
			PROJECTILE, // ability or item
			//other
		} type=PROJECTILE;

		float speed;
		Damage damage;
		AZStd::string particle;
		ProjectileId projectileId;
		bool autoRelease = false;
		float triggerRadius = 0.f;
	};

	class ProjectileRequests
		: public AZ::ComponentBus
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		//////////////////////////////////////////////////////////////////////////

		virtual void Fire(ProjectileInfo info) = 0;

		//virtual void SetProjectileSpeed(float speed) = 0;
		//virtual void SetProjectileSource(AZ::EntityId id) = 0;
		//virtual void SetProjectileTarget(AZ::EntityId id) = 0;

		//virtual void SetProjectileAsset(const ProjectileAsset&) = 0;
		//virtual ProjectileAsset GetProjectileAsset() = 0;
		///@TODO: add physical projectiles 
	};
	using ProjectileRequestBus = AZ::EBus<ProjectileRequests>;



	class ProjectileNotifications
		: public AZ::ComponentBus
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		//////////////////////////////////////////////////////////////////////////


		virtual void OnProjectileHit (ProjectileInfo) {};
	};
	using ProjectileNotificationBus = AZ::EBus<ProjectileNotifications>;




}