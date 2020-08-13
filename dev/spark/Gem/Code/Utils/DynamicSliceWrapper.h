#pragma once

#include <AzFramework/Entity/GameEntityContextBus.h>
#include <AzCore/Math/Transform.h>
#include <AzCore/std/functional.h>

namespace spark {


	using DynamicSliceAsset = AZ::Data::Asset<AZ::DynamicPrefabAsset>;
	using DynamicSliceAssetId = AZ::Data::AssetId;

	class DynamicSliceWrapper :
		protected AzFramework::SliceInstantiationResultBus::Handler
	{
		AZ::EntityId m_id;
		AzFramework::SliceInstantiationTicket m_ticket;
		DynamicSliceAsset m_asset;
		//AZ::Data::AssetId m_assetId;

		using Callback = std::function<void(AZ::Entity*)>;
		//todo handle copy/destruction
	public:
		DynamicSliceWrapper() {}
		DynamicSliceWrapper(const DynamicSliceAsset &asset);

		~DynamicSliceWrapper();

		void Load();
		bool Loaded() const;

		bool IsReady() const;
		
		AZ::EntityId GetId() const;
		AZ::Data::AssetId GetAssetId() const;

		AzFramework::SliceInstantiationTicket GetInstantiationTicket() const { return m_ticket; }

		Callback onPreInstantiate;
		Callback onInstantiated;
		AZ::Transform transform = AZ::Transform::CreateIdentity();
	protected:
		enum {
			invalid,
			waiting,
			loading,
			loaded
		}m_state=invalid;

		void OnSlicePreInstantiate(const AZ::Data::AssetId& sliceAssetId, const AZ::SliceComponent::SliceInstanceAddress& instance);
		void OnSliceInstantiated(const AZ::Data::AssetId& sliceAssetId, const AZ::SliceComponent::SliceInstanceAddress& instance);

		void OnSliceInstantiationFailed(const AZ::Data::AssetId& /*sliceAssetId*/);
		void OnSliceInstantiationFailedOrCanceled(const AZ::Data::AssetId& /*sliceAssetId*/, bool /*canceled*/);
	};




	

}
