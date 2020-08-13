

#include "spark_precompiled.h"

#include "ProjectileManagerSystemComponent.h"
#include "ProjectileControllerComponent.h"
#include "Busses/DynamicSliceManagerBus.h"

#include <AzCore/Serialization/EditContext.h>
#include <AzFramework/Input/Channels/InputChannel.h>
#include <AzFramework/Input/Devices/Keyboard/InputDeviceKeyboard.h>
#include <LmbrCentral/Rendering/ParticleComponentBus.h>
#include <LmbrCentral/Shape/CylinderShapeComponentBus.h>
#include <LmbrCentral/Scripting/TriggerAreaComponentBus.h>


#include <AzFramework/Network/NetworkContext.h>
#include <AzFramework/Network/NetBindingHandlerBus.h>
#include <AzFramework/Network/NetBindingComponent.h>
#include <AzFramework/Components/TransformComponent.h>
#include <GridMate/Replica/ReplicaFunctions.h>

#include <GridMate/Replica/ReplicaChunk.h>
#include <GridMate/Replica/DataSet.h>
#include <GridMate/Serialize/CompressionMarshal.h>

#include "Components/TriggerAreaComponent.h"
#include "Components/UnitNavigationComponent.h"



using namespace spark;
using namespace AzFramework;


//ProjectileManagerSystemComponent implementation

void ProjectileManagerSystemComponent::Reflect(AZ::ReflectContext* reflection)
{
	if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
	{
		serializationContext->Class<ProjectileManagerSystemComponent, AZ::Component>()
			->Version(2);


		if (auto editContext = serializationContext->GetEditContext())
		{
			editContext->Class<ProjectileManagerSystemComponent>("ProjectileManagerSystemComponent", "Responsible for the lifetime of projectiles")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
				->Attribute(AZ::Edit::Attributes::Category, "spark")
				->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"));
		}
	}
}

void ProjectileManagerSystemComponent::Init()
{
	 
}

void ProjectileManagerSystemComponent::Activate()
{
	ProjectileManagerRequestBus::Handler::BusConnect();
}

void ProjectileManagerSystemComponent::Deactivate()
{
	ProjectileManagerRequestBus::Handler::BusDisconnect();

	ResetProjectileManager();
}


void ProjectileManagerSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
{
	provided.push_back(AZ_CRC("ProjectileManagerService"));
}

void ProjectileManagerSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
{
	incompatible.push_back(AZ_CRC("ProjectileManagerService"));
}

void ProjectileManagerSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
{
	(void)required;
}

void ProjectileManagerSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
{
	(void)dependent;
}


void ProjectileManagerSystemComponent::RegisterProjectileAsset(const ProjectileAsset & asset)
{

}

void ProjectileManagerSystemComponent::OnSliceInstantiated(const AZ::Data::AssetId& sliceAssetId, const AZ::SliceComponent::SliceInstanceAddress& instance) {

	const AzFramework::SliceInstantiationTicket ticket = (*AzFramework::SliceInstantiationResultBus::GetCurrentBusId());

	
	// Stop listening for this ticket (since it's done). We can have have multiple tickets in flight.
	AzFramework::SliceInstantiationResultBus::MultiHandler::BusDisconnect(ticket);

	auto it = m_projectileSlices.find(ticket);

	if (it == m_projectileSlices.end())return;


	ProjectileInfo info = it->second;
	m_projectileSlices.erase(it);

	for (AZ::Entity *entity : instance.second->GetInstantiated()->m_entities)
	{
		EBUS_EVENT_ID( entity->GetId(), ProjectileRequestBus, Fire, info);
	}

	AZ_Printf(0, "OnSliceInstantiated with ticket=%d and assetId=%s", ticket.m_requestId, sliceAssetId.ToString<AZStd::string>().c_str());

	AZ::Entity *e = (*(instance.second->GetInstantiated()->m_entities.rbegin()));
	AZ_Printf(0, "ProjectileManagerSystemComponent::OnSliceInstantiated  entity's state is %d", e->GetState());
}


void ProjectileManagerSystemComponent::OnSlicePreInstantiate(const AZ::Data::AssetId& sliceAssetId, const AZ::SliceComponent::SliceInstanceAddress& instance) {
	//m_id = (*(instance.second->GetInstantiated()->m_entities.rbegin()))->GetId();
	//AzFramework::SliceInstantiationResultBus::Handler::BusDisconnect();
	AZ::Entity *e = (*(instance.second->GetInstantiated()->m_entities.rbegin()));

	
	AZ_Printf(0, "ProjectileManagerSystemComponent::OnSlicePreInstantiate  entity's state is %d", e->GetState());
}


ProjectileId ProjectileManagerSystemComponent::CreateProjectile(ProjectileInfo projectileInfo)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId())) return 0;

	projectileInfo.projectileId = m_projectileId++;

	return projectileInfo.projectileId;
}

void ProjectileManagerSystemComponent::FireProjectile(ProjectileInfo projectileInfo)
{
	if (!AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId())) return;
	AZ_Printf(0, "OnProjectileFired");

	if (!projectileInfo.autoRelease)
	{
		m_projectiles[projectileInfo.projectileId] = projectileInfo;
	}

	if (projectileInfo.type == ProjectileInfo::NORMAL_ATTACK)
	{
		// it's a right click attack, right now these are stored in slices so we gotta do this dance
		//const AZ::Data::AssetType& projectileAssetType = azrtti_typeid<AZ::DynamicPrefabAsset>();
		//AZ::Data::Asset<AZ::DynamicPrefabAsset> projectileAsset(projectileInfo.asset, projectileAssetType);

		AZ::Data::Asset<AZ::DynamicPrefabAsset> projectileAsset;
		EBUS_EVENT_RESULT(projectileAsset, DynamicSliceManagerRequestBus, GetDynamicSliceAsset, projectileInfo.asset);

		AzFramework::SliceInstantiationTicket ticket;
		AZ::Transform transform = AZ::Transform::CreateTranslation(projectileInfo.startingPosition);
		EBUS_EVENT_RESULT(ticket, AzFramework::GameEntityContextRequestBus, InstantiateDynamicSlice, projectileAsset, transform, nullptr);
		AzFramework::SliceInstantiationResultBus::MultiHandler::BusConnect(ticket);

		m_projectileSlices[ticket] = projectileInfo;		
	}
	else
	{
		// normal projectile
		// these are basically just hollow entities, they can have stuff attached to them later
		AZ::Entity* entity = aznew AZ::Entity;
		if (entity == nullptr)
		{
			AZ_Warning("ProjectileManagerSystemComponent", false, "Failed to create an entity with AzFramework::GameEntityContextRequestBus, CreateGameEntity");
			return;
		}

		// AZ_Printf(0, "the projectile state is %i", entity->GetState())
		auto projectileComponent = entity->CreateComponent<ProjectileControllerComponent>();
		entity->CreateComponent<AzFramework::TransformComponent>();
		if (projectileInfo.triggerRadius > 0.f)
		{
			entity->CreateComponent(LmbrCentral::CylinderShapeComponentTypeId);
			entity->CreateComponent<UnitNavigationComponent>();
			projectileComponent->m_isNavEntity = true;
			entity->CreateComponent<spark::Spark2dTriggerAreaComponent>();// ("{E3DF5790-F0AD-43AE-9FB2-0A37F873DECB}");
		}
		entity->CreateComponent<AzFramework::NetBindingComponent>();
		// AZ_Printf(0, "the projectile state is %i", entity->GetState());

		// activate the entity and then make a bunch of ebus calls
		entity->Init();
		entity->Activate();

		EBUS_EVENT(AzFramework::GameEntityContextRequestBus, AddGameEntity, entity);

		if (projectileInfo.triggerRadius > 0.f)
		{
			// trigger area
			EBUS_EVENT_ID(entity->GetId(), LmbrCentral::CylinderShapeComponentRequestsBus, SetRadius, projectileInfo.triggerRadius);
			EBUS_EVENT_ID(entity->GetId(), LmbrCentral::CylinderShapeComponentRequestsBus, SetHeight, 5.0f);
			EBUS_EVENT_ID(entity->GetId(), LmbrCentral::TriggerAreaRequestsBus, AddRequiredTag, AZ::Crc32("unit"));
		}

		// transform
		//EBUS_EVENT_ID(entity->GetId(), AZ::TransformBus, SetWorldTranslation, projectileInfo.startingPosition);
		EBUS_EVENT_ID(entity->GetId(), NavigationEntityRequestBus, SetPosition, projectileInfo.startingPosition);

		// projectile controller
		EBUS_EVENT_ID(entity->GetId(), ProjectileRequestBus, Fire, projectileInfo);
	}
	return; //don't send to all proxies
}

void ProjectileManagerSystemComponent::ResetProjectileManager()
{
	AZ_Warning("ProjectileManagerSystemComponent", false, "ResetProjectileManager");

	//clear old entities:
	//ctrl+g in the editor don't reconstruct system components, so we want to clear the old state to not have invalid entities in the pools 
	m_projectiles.clear();
	m_projectileSlices.clear();
}

bool ProjectileManagerSystemComponent::IsProjectileReleased (const ProjectileId pId)
{
	return m_projectiles.find(pId) == m_projectiles.end();
}

void ProjectileManagerSystemComponent::ReleaseProjectile (const ProjectileId pId)
{
	auto it = m_projectiles.find(pId);
	if (it == m_projectiles.end()) return;
	m_projectiles.erase(it);
}