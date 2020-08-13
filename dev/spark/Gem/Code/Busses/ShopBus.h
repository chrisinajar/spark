#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/ComponentBus.h>

#include "Utils/CommonTypes.h"
#include "Utils/Amount.h"

namespace spark
{
	struct ItemShopInfo
	{
		AZ_TYPE_INFO(ItemShopInfo, "{2DDF9E9A-EC06-477E-8661-0E4454829D5F}");
		static void Reflect(AZ::ReflectContext* reflection);

		ItemTypeId id;
		Costs price;// dunno about that, can the same item be sold in different shops at different prices? I'll keep it for mods anyway
		int stock=-1; // if -1 is unlimited

		Number GetGoldCost();

		AZ_FORCE_INLINE bool operator== (const ItemShopInfo& other) const
		{
			return stock == other.stock && id == other.id && price == other.price;
		}
	};
	using ItemList = AZStd::vector<ItemShopInfo>;


	class ShopRequests
		: public AZ::ComponentBus
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
		

		virtual ItemId  BuyItem(UnitId, ItemTypeId) = 0;
		virtual void	SellItem(UnitId, ItemId) = 0;

		virtual bool IsInRange(UnitId) = 0;

		virtual ItemList GetItemList() = 0;

		virtual void AddItemShopInfo(ItemShopInfo) = 0;
		virtual void AddItem(ItemTypeId) = 0;
	};
	using ShopRequestBus = AZ::EBus<ShopRequests>;


	class ShopsRequests
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
		

		virtual void GetAllShops(AZStd::vector<AZ::EntityId> &result) = 0;
	};
	using ShopsRequestBus = AZ::EBus<ShopsRequests>;
}
