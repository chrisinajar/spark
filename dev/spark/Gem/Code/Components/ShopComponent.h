#pragma once

#include "Busses/ShopBus.h"

#include <AzFramework/Network/NetBindable.h>

namespace spark
{

	class ShopComponent
		: public AZ::Component,
		protected ShopRequestBus::Handler,
		protected ShopsRequestBus::Handler,
		public AzFramework::NetBindable
	{
	public:
		AZ_COMPONENT(ShopComponent, "{0C6160F7-7061-4884-8FCC-D073FDD086B4}", AzFramework::NetBindable);

		static void Reflect(AZ::ReflectContext* context);

		////////////////////////////////////////////////////////////////////////
		// AZ::Component interface implementation
		void Init() override;
		void Activate() override;
		void Deactivate() override;

		static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);
		////////////////////////////////////////////////////////////////////////


		ItemId  BuyItem(UnitId, ItemTypeId);
		void	SellItem(UnitId, ItemId);

		
		bool IsInRange(UnitId);

		ItemList GetItemList();
		//void AddItem( ItemTypeId, Price price, int stock);
		void AddItemShopInfo(ItemShopInfo);
		void AddItem(ItemTypeId);

		void GetAllShops(AZStd::vector<AZ::EntityId> &result);

		//NetBindable
		GridMate::ReplicaChunkPtr GetNetworkBinding() override;
		void SetNetworkBinding(GridMate::ReplicaChunkPtr chunk) override;
		void UnbindFromNetwork() override;

		void OnNewItemList(const ItemList&, const GridMate::TimeContext& tc);
	private:

		ItemList m_itemList;


		void OnNewItemListImpl(ItemList&);
		GridMate::ReplicaChunkPtr m_replicaChunk = nullptr;
	};

}