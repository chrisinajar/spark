#include "spark_precompiled.h"

#include "DynamicSliceManagerComponent.h"


#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

#include <AzFramework/Entity/GameEntityContextBus.h>
#include <AzCore/Asset/AssetManagerBus.h>
#include "Utils/Log.h"

namespace spark
{

	void DynamicSliceManagerComponent::Reflect(AZ::ReflectContext* context)
	{
		if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
		{
			serialize->Class<DynamicSliceManagerComponent, AZ::Component>()
				->Version(0)
				;

			if (AZ::EditContext* ec = serialize->GetEditContext())
			{
				ec->Class<DynamicSliceManagerComponent>("DynamicSliceManagerComponent", "handles dynamic slices")
					->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					//   ->Attribute(AZ::Edit::Attributes::Category, "spark")
					//	->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"));
					//;
					->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
					->Attribute(AZ::Edit::Attributes::AutoExpand, true);
			}
		}

		if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
		{
			behaviorContext->EBus<DynamicSliceManagerRequestBus>("DynamicSliceManagerRequestBus")
				->Event("GetDynamicSliceAsset", &DynamicSliceManagerRequestBus::Events::GetDynamicSliceAsset)
				->Event("GetDynamicSliceAssetId", &DynamicSliceManagerRequestBus::Events::GetDynamicSliceAssetId)
				->Event("SpawnDynamicSlice", &DynamicSliceManagerRequestBus::Events::SpawnDynamicSlice);
		}
	}

	void DynamicSliceManagerComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
	{
		provided.push_back(AZ_CRC("DynamicSliceManagerService"));
	}

	void DynamicSliceManagerComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
	{
		incompatible.push_back(AZ_CRC("DynamicSliceManagerService"));
	}

	void DynamicSliceManagerComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
	{
		(void)required;
	}

	void DynamicSliceManagerComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
	{
		(void)dependent;
	}

	void DynamicSliceManagerComponent::Init()
	{

	}

	void DynamicSliceManagerComponent::Activate()
	{
		DynamicSliceManagerRequestBus::Handler::BusConnect();
	}

	void DynamicSliceManagerComponent::Deactivate()
	{
		DynamicSliceManagerRequestBus::Handler::BusDisconnect();
	}


	DynamicSliceAsset DynamicSliceManagerComponent::GetDynamicSliceAsset(DynamicSliceTypeId id)
	{
		auto it = m_slices_cache.find(id);
		if ( it != m_slices_cache.end() )
		{
			return it->second.second;
		}

		AZ::Data::AssetId assetId;
		AZ::Data::Asset<AZ::DynamicPrefabAsset> dynamicSliceAsset;

		AZStd::string str = AZStd::string::format("slices/%s.dynamicslice", id.c_str());

		EBUS_EVENT_RESULT(assetId, AZ::Data::AssetCatalogRequestBus, GetAssetIdByPath, str.c_str(), AZ::AzTypeInfo<AZ::DynamicPrefabAsset>::Uuid(), true);

		if (!assetId.IsValid())
		{
			sWARNING("DynamicSliceManagerComponent::GetDynamicSliceAsset  cannot instantiate slice asset with id:"+id);
			return dynamicSliceAsset;
		}

		//load the dynamic slice and initialize it
		dynamicSliceAsset.Create(assetId, true);

		m_slices_cache[id] = AZStd::make_pair<DynamicSliceAssetId, DynamicSliceAsset>(assetId,dynamicSliceAsset);

		return dynamicSliceAsset;
	}

	DynamicSliceAssetId DynamicSliceManagerComponent::GetDynamicSliceAssetId(DynamicSliceTypeId id)
	{
		auto it = m_slices_cache.find(id);
		if (it != m_slices_cache.end())
		{
			return it->second.first;
		}

		return GetDynamicSliceAsset(id).GetId();
	}

	AzFramework::SliceInstantiationTicket DynamicSliceManagerComponent::SpawnDynamicSlice(DynamicSliceTypeId id)
	{
		AzFramework::SliceInstantiationTicket ticket;
		auto asset = GetDynamicSliceAsset(id);
		
		if (asset.GetId().IsValid())
		{
			EBUS_EVENT_RESULT(ticket, AzFramework::GameEntityContextRequestBus, InstantiateDynamicSlice, asset, AZ::Transform::CreateIdentity(), nullptr);
		}
		return ticket;
	}

}
