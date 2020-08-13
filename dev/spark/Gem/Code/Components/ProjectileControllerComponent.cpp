

#include "spark_precompiled.h"

#include "ProjectileControllerComponent.h"

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Math/Transform.h>
#include <AzCore/Serialization/EditContext.h>
#include <LmbrCentral/Rendering/ParticleComponentBus.h>

#include <AzFramework/Network/NetworkContext.h>
#include <AzFramework/Network/NetBindingHandlerBus.h>
#include <AzFramework/Network/NetBindingComponent.h>
#include <GridMate/Replica/ReplicaFunctions.h>
#include <GridMate/Replica/ReplicaChunk.h>
#include <GridMate/Replica/DataSet.h>
#include <AzFramework/Components/TransformComponent.h>
#include <AzFramework/Entity/EntityContextBus.h>

#include "Utils/Marshaler.h"
#include "Busses/ProjectileManagerBus.h"
#include "Busses/GameplayBusses.h"
#include "UnitAttackComponent.h"
#include "Busses/NavigationManagerBus.h"
#include "Busses/MapBus.h"

using namespace spark;

class ProjectileManagerNotificationBusHandler
	: public ProjectileManagerNotificationBus::Handler
	, public AZ::BehaviorEBusHandler
{
public:
	AZ_EBUS_BEHAVIOR_BINDER(ProjectileManagerNotificationBusHandler, "{69FFDAF4-8E6A-44DC-9266-87E5423BF56C}", AZ::SystemAllocator, OnProjectileCreated);

	void OnProjectileCreated(ProjectileInfo info, AZ::EntityId id)
	{
		Call(FN_OnProjectileCreated, info, id);
	}
};

class ProjectileNotificationBusHandler
	: public ProjectileNotificationBus::Handler
	, public AZ::BehaviorEBusHandler
{
public:
	AZ_EBUS_BEHAVIOR_BINDER(ProjectileNotificationBusHandler, "{79BB20FE-09AD-4253-AE5E-72BFF4603119}", AZ::SystemAllocator, OnProjectileHit);

	void OnProjectileHit(ProjectileInfo info)
	{
		Call(FN_OnProjectileHit, info);
	}
};

class ProjectileControllerReplicaChunk : public GridMate::ReplicaChunkBase
{
public:
    AZ_CLASS_ALLOCATOR(ProjectileControllerReplicaChunk, AZ::SystemAllocator, 0);

    static const char* GetChunkName() { return "ProjectileControllerReplicaChunk"; }

       ProjectileControllerReplicaChunk()
        : m_particleEffect("ParticleEffect")
    {
    }

    bool IsReplicaMigratable()
    {
        return true;
    }

    GridMate::DataSet<AZStd::string>::BindInterface<ProjectileControllerComponent, &ProjectileControllerComponent::OnNewParticleEffect> m_particleEffect;
};

void ProjectileControllerComponent::Reflect(AZ::ReflectContext* reflection)
{
	if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
	{
		serializationContext->Class<ProjectileControllerComponent, AzFramework::NetBindable, AZ::Component>()
			->Version(2)
			;

		if (auto editContext = serializationContext->GetEditContext())
		{
			editContext->Class<ProjectileControllerComponent>("ProjectileControllerComponent", "todo description")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
				->Attribute(AZ::Edit::Attributes::Category, "spark")
				->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"))
				;// ->DataElement(nullptr, &ProjectileControllerComponent::m_projectileInfo, "projectileInfo", "todo");
		}
	} 
	// projectile stuff for lua
	ProjectileInfo::Reflect(reflection);

	if (auto netContext = azrtti_cast<AzFramework::NetworkContext*>(reflection))
	{
	    netContext->Class<ProjectileControllerComponent>()
			->Chunk<ProjectileControllerReplicaChunk>()
	        ->Field("ParticleEffect", &ProjectileControllerReplicaChunk::m_particleEffect)
	        ;
	}

	if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
	{
		behaviorContext->EBus<ProjectileManagerRequestBus>("ProjectileManagerRequestBus")
			->Event("CreateProjectile", &ProjectileManagerRequestBus::Events::CreateProjectile)
			->Event("FireProjectile", &ProjectileManagerRequestBus::Events::FireProjectile)
			->Event("IsProjectileReleased", &ProjectileManagerRequestBus::Events::IsProjectileReleased)
			->Event("ReleaseProjectile", &ProjectileManagerRequestBus::Events::ReleaseProjectile)
			;

		behaviorContext->EBus<ProjectileRequestBus>("ProjectileRequestBus")
			->Event("Fire", &ProjectileRequestBus::Events::Fire)
			;

		behaviorContext->EBus<ProjectileManagerNotificationBus>("ProjectileManagerNotificationBus")
			->Handler<ProjectileManagerNotificationBusHandler>()
			->Event("OnProjectileCreated", &ProjectileManagerNotificationBus::Events::OnProjectileCreated)
			;

		behaviorContext->EBus<ProjectileNotificationBus>("ProjectileNotificationBus")
			->Handler<ProjectileNotificationBusHandler>()
			->Event("OnProjectileHit", &ProjectileNotificationBus::Events::OnProjectileHit)
			;
	}
}

void ProjectileControllerComponent::OnNewParticleEffect (const AZStd::string& particleEffect, const GridMate::TimeContext& tc)
{
	(void)tc;
	SetParticleEffect(particleEffect);
}

GridMate::ReplicaChunkPtr ProjectileControllerComponent::GetNetworkBinding()
{
    auto replicaChunk = GridMate::CreateReplicaChunk<ProjectileControllerReplicaChunk>();
    replicaChunk->SetHandler(this);
    m_replicaChunk = replicaChunk;

    return m_replicaChunk;
}

void ProjectileControllerComponent::SetNetworkBinding(GridMate::ReplicaChunkPtr chunk)
{
	chunk->SetHandler(this);
	m_replicaChunk = chunk;

    ProjectileControllerReplicaChunk* projectileChunk = static_cast<ProjectileControllerReplicaChunk*>(m_replicaChunk.get());
	SetParticleEffect(projectileChunk->m_particleEffect.Get());
}

void ProjectileControllerComponent::UnbindFromNetwork()
{
    m_replicaChunk->SetHandler(nullptr);
    m_replicaChunk = nullptr;
}

void ProjectileControllerComponent::SetParticleEffect (const AZStd::string& particleEffect)
{
	if (m_particleEffect == particleEffect)
	{
		return;
	}

	if (!m_needsParticleEffect && !m_isMoving)
	{
		AZ::TickBus::Handler::BusConnect();
	}
	
	m_needsParticleEffect = true;
	m_particleEffect = particleEffect;

    if (m_replicaChunk && AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
    {
        // If you are authoritative over the entity and the component is replicated, update the value of the DataSet and propagate to clients 
        auto projectileChunk = static_cast<ProjectileControllerReplicaChunk*>(m_replicaChunk.get());
        projectileChunk->m_particleEffect.Set(m_particleEffect);
    }
}

void ProjectileControllerComponent::Init()
{
}

void ProjectileControllerComponent::Activate()
{
	ProjectileRequestBus::Handler::BusConnect(GetEntityId());

	AZ::Entity* entity = aznew AZ::Entity;
	entity->CreateComponent("{65BC817A-ABF6-440F-AD4F-581C40F92795}");
	auto tsComponent = entity->CreateComponent<AzFramework::TransformComponent>();
	tsComponent->SetParent(GetEntityId());
	entity->Init();
	entity->Activate();
	EBUS_EVENT(AzFramework::GameEntityContextRequestBus, AddGameEntity, entity);
	m_particleEntity = entity->GetId();
}

void ProjectileControllerComponent::Deactivate()
{
	ProjectileRequestBus::Handler::BusDisconnect();
	EBUS_EVENT(AzFramework::GameEntityContextRequestBus, DestroyGameEntity, m_particleEntity);

	if (m_isMoving)
	{
		m_isMoving = false;
		AZ::TickBus::Handler::BusDisconnect();
	}
}

void ProjectileControllerComponent::Despawn()
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId())) return;

	if (m_projectileInfo.targetType == ProjectileInfo::TARGET_ENTITY)
	{
		m_projectileInfo.targetEntity.SetInvalid();
	}
	EBUS_EVENT(AzFramework::GameEntityContextRequestBus, DestroyGameEntity, GetEntityId());
	//EBUS_EVENT(ProjectileManagerRequestBus, ReleaseProjectile, GetEntityId());
}

void ProjectileControllerComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time)
{	

	if (m_needsParticleEffect)
	{
		m_needsParticleEffect = false;
		if (!m_particleEffect.empty())
		{
			// particle effect
			AZ_Printf(0, "Adding the particle effect %s", m_particleEffect.c_str());
			LmbrCentral::ParticleEmitterSettings fireInfo;
			fireInfo.m_selectedEmitter = m_particleEffect;
			fireInfo.m_sizeScale = 0.3;
			EBUS_EVENT_ID(m_particleEntity, LmbrCentral::ParticleComponentRequestBus, SetupEmitter, fireInfo.m_selectedEmitter, fireInfo);
			EBUS_EVENT_ID(m_particleEntity, LmbrCentral::ParticleComponentRequestBus, Show);
		}
		else
		{
			EBUS_EVENT_ID(m_particleEntity, LmbrCentral::ParticleComponentRequestBus, Hide);
		}

		if (!m_isMoving)
		{
			AZ::TickBus::Handler::BusDisconnect();
		}
	}

    AZ::Vector3 currentPosition;
    AZ::Vector3 currentRotation;
	AZ::Vector3 targetPosition;

    EBUS_EVENT_ID_RESULT(currentPosition, GetEntityId(), AZ::TransformBus, GetWorldTranslation);
    EBUS_EVENT_ID_RESULT(currentRotation, GetEntityId(), AZ::TransformBus, GetLocalRotation);


	if (m_projectileInfo.targetType == ProjectileInfo::TARGET_ENTITY)
	{
		if (!m_projectileInfo.targetEntity.IsValid())
		{
			bool isReleased = false;
			EBUS_EVENT_RESULT(isReleased, ProjectileManagerRequestBus, IsProjectileReleased, m_projectileInfo.projectileId);
			if (isReleased)
			{
				Despawn();
			}
			return;
		}
		EBUS_EVENT_ID_RESULT(targetPosition, m_projectileInfo.targetEntity, AZ::TransformBus, GetWorldTranslation);
		targetPosition.SetZ(targetPosition.GetZ() + 1.0f); // don't aim to the feet
	}
	else
	{
		targetPosition = m_projectileInfo.targetPosition;
	}

	auto delta = targetPosition - currentPosition;
	delta.SetZ(0.0f);
	auto direction = delta.GetNormalized();
	float directionAngle = std::atan2(direction.GetY(), direction.GetX());

	
	
	auto velocity = direction * m_projectileInfo.speed * deltaTime;

	if (velocity.GetLength() > delta.GetLength())//delta.GetLength() > 0.1f)  //the projectile hit the target
	{
		//send all events
		if (!m_hasHit)
		{
			m_hasHit = true;
			if (m_projectileInfo.targetType == ProjectileInfo::TARGET_ENTITY)
			{
				AZ_Printf(0, "Projectile OnHit() on target entity");
				EBUS_EVENT_ID(m_projectileInfo.targetEntity, ProjectileNotificationBus, OnProjectileHit, m_projectileInfo);
			}
			else
			{
				AZ_Printf(0, "Projectile OnHit() on projectile directly");
				EBUS_EVENT_ID(GetEntityId(), ProjectileNotificationBus, OnProjectileHit, m_projectileInfo);
			}
			
			if (m_projectileInfo.type == ProjectileInfo::NORMAL_ATTACK)
			{
				AZ_Printf(0, "projectile is normal attack");
				AttackInfo info;
				info.attacker = m_projectileInfo.attacker;
				// always entity target when it's an attack
				info.attacked = m_projectileInfo.targetEntity;
				info.damage = m_projectileInfo.damage.damage;
				UnitAttackComponent::SendAttackEvents(info);
			}
		}

		bool isReleased = false;
		EBUS_EVENT_RESULT(isReleased, ProjectileManagerRequestBus, IsProjectileReleased, m_projectileInfo.projectileId);
		if (isReleased)
		{
			Despawn();
			return;
		}
	}

	auto new_position = currentPosition + velocity;
	float terrain_height;
	EBUS_EVENT_RESULT(terrain_height, MapRequestBus, GetTerrainHeightAt, new_position);
	new_position.SetZ(terrain_height + 4.0f);

	// move towards the target position
	if (m_isNavEntity)
	{
		EBUS_EVENT_ID(GetEntityId(), NavigationEntityRequestBus, SetPosition, new_position);
	}
	else
	{
		EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus, SetWorldTranslation, new_position);
	}
	currentRotation.SetZ(directionAngle);
	EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus, SetLocalRotation, currentRotation);
}

void ProjectileControllerComponent::Fire(ProjectileInfo projectileInfo)
{
	AZ_Printf(0, "Projectile Fire()");
	m_projectileInfo = projectileInfo;
	SetParticleEffect(projectileInfo.particle);

	m_hasHit = false;

	if (!m_isMoving)
	{
		EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus, SetWorldTranslation, projectileInfo.startingPosition);
		EBUS_EVENT_ID(m_projectileInfo.projectileId, ProjectileManagerNotificationBus, OnProjectileCreated, m_projectileInfo, GetEntityId());
		m_isMoving = true;
		AZ::TickBus::Handler::BusConnect();
	}
}

