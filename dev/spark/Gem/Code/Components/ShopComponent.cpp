#include "spark_precompiled.h"

#include "ShopComponent.h"

#include <AzFramework/Entity/EntityContextBus.h>
#include <AzFramework/Entity/EntityContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

#include "Busses/GameManagerBus.h"
#include "Busses/VariableBus.h"
#include "Busses/UnitAbilityBus.h"
#include "Busses/InfoBus.h"


#include <AzCore/Component/TransformBus.h>
#include <LmbrCentral/Shape/ShapeComponentBus.h>

#include "Utils/Slot.h"
#include "Utils/Log.h"

#include <AzCore/JSON/document.h>

#include <AzFramework/Network/NetworkContext.h>
#include <AzFramework/Network/NetBindingHandlerBus.h>
#include <GridMate/Replica/ReplicaFunctions.h>
#include <GridMate/Replica/RemoteProcedureCall.h>

#include <GridMate/Replica/ReplicaChunk.h>
#include <GridMate/Replica/DataSet.h>
#include <GridMate/Serialize/CompressionMarshal.h>
#include "Utils/Marshaler.h"

using namespace GridMate;



template<typename DataType>
class StupidThrottle
{
public:
	AZ_TYPE_INFO(StupidThrottle, "{EAA1EA07-E067-40EB-B9F4-C5E53C17A13C}", DataType);

	bool WithinThreshold(const DataType& newValue) const
	{
		return false;
	}

	void UpdateBaseline(const DataType& baseline)
	{
		m_baseline = baseline;
	}

private:
	DataType m_baseline;
};


template<>
class GridMate::Marshaler<spark::ItemShopInfo>
{
public:
	typedef spark::ItemShopInfo DataType;

	void Marshal(WriteBuffer& wb, const DataType& value) const
	{
		Marshaler<AZStd::string> stringMarshaler;
		GridMate::ContainerMarshaler<spark::Costs> costsMarshaler;
		Marshaler<int> intMarshaler;

		stringMarshaler.Marshal(wb, value.id);
		costsMarshaler.Marshal(wb, value.price);
		intMarshaler.Marshal(wb, value.stock);
	}
	void Unmarshal(DataType& value, ReadBuffer& rb) const
	{
		Marshaler<AZStd::string> stringMarshaler;
		GridMate::ContainerMarshaler<spark::Costs> costsMarshaler;
		Marshaler<int> intMarshaler;

		stringMarshaler.Unmarshal(value.id, rb);
		costsMarshaler.Unmarshal(value.price, rb);
		intMarshaler.Unmarshal( value.stock, rb);
	}
};



namespace spark
{

	class ShopComponentChunk
		: public GridMate::ReplicaChunkBase
	{
	public:
		AZ_CLASS_ALLOCATOR(ShopComponentChunk, AZ::SystemAllocator, 0);

		ShopComponentChunk()
			: m_itemList("items")
		{}

		bool IsReplicaMigratable() override
		{
			return true;
		}

		static const char* GetChunkName()
		{
			return "ShopComponentChunk";
		}

		GridMate::DataSet<ItemList, GridMate::ContainerMarshaler<ItemList> >::BindInterface<ShopComponent, &ShopComponent::OnNewItemList> m_itemList;
	};


	void ItemShopInfo::Reflect(AZ::ReflectContext* reflection)
	{

		if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
		{
			serializationContext->Class<ItemShopInfo>()
				->Version(1)
				->Field("id", &ItemShopInfo::id)
				->Field("price", &ItemShopInfo::price)
				->Field("stock", &ItemShopInfo::stock);

		}


		if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
		{
			behaviorContext->Class<ItemShopInfo>("ItemShopInfo")
				//->Attribute(AZ::Script::Attributes::Storage, AZ::Script::Attributes::StorageType::Value)
				//->Attribute(AZ::Script::Attributes::ConstructorOverride, &SlotScriptConstructor)
				->Property("id", BehaviorValueProperty(&ItemShopInfo::id))
				->Property("price", BehaviorValueProperty(&ItemShopInfo::price))
				->Property("stock", BehaviorValueProperty(&ItemShopInfo::stock))
				->Method("GetGoldCost", &ItemShopInfo::GetGoldCost);
		}


	}

	Number spark::ItemShopInfo::GetGoldCost()
	{
		for (auto amount : price)
		{
			if (amount.variable == "gold")return amount.amount;
		}
		return 0;
	}


	void ShopComponent::Reflect(AZ::ReflectContext* context)
	{
		ItemShopInfo::Reflect(context);

		if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
		{
			serialize->Class<ShopComponent, AZ::Component>()
				->Version(0)
				;

			if (AZ::EditContext* ec = serialize->GetEditContext())
			{
				ec->Class<ShopComponent>("ShopComponent", "")
					->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					->Attribute(AZ::Edit::Attributes::Category, "spark")
					->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"))
					;
			}
		}

		if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
		{
			behaviorContext->EBus<ShopRequestBus>("ShopRequestBus")
				->Event("BuyItem", &ShopRequestBus::Events::BuyItem)
				->Event("SellItem", &ShopRequestBus::Events::SellItem)
				->Event("IsInRange", &ShopRequestBus::Events::IsInRange)
				->Event("GetItemList", &ShopRequestBus::Events::GetItemList)
				->Event("AddItem", &ShopRequestBus::Events::AddItem)
				->Event("AddItemShopInfo", &ShopRequestBus::Events::AddItemShopInfo)
				;

			behaviorContext->EBus<ShopsRequestBus>("ShopsRequestBus")
				->Event("GetAllShops", &ShopsRequestBus::Events::GetAllShops)
				;
		}

		if (auto netContext = azrtti_cast<AzFramework::NetworkContext*>(context))
		{
			netContext->Class<ShopComponent>()
				->Chunk<ShopComponentChunk>()
				->Field("items", &ShopComponentChunk::m_itemList)
				;
		}
	}


	void ShopComponent::Init()
	{

	}

	void ShopComponent::Activate()
	{
		ShopRequestBus::Handler::BusConnect(GetEntityId());
		ShopsRequestBus::Handler::BusConnect();
	}

	void ShopComponent::Deactivate()
	{
		ShopRequestBus::Handler::BusDisconnect();
		ShopsRequestBus::Handler::BusDisconnect();
	}


	void ShopComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
	{
		provided.push_back(AZ_CRC("ShopService"));
	}
	void ShopComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
	{
		incompatible.push_back(AZ_CRC("ShopService"));
	}
	void ShopComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
	{
		required.push_back(AZ_CRC("ShapeService", 0xe86aa5fe));	// need for sure a shape component
		required.push_back(AZ_CRC("ProximityTriggerService", 0x561f262c));//need a trigger area ?
	}
	void ShopComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
	{

	}


	Slot GetFirstFreeSlot(UnitId unit)
	{
		Slot slot;
		bool occupied = true;

		for (int i = 0; i < 9; i++) {
			slot = Slot(Slot::Inventory, i);
			EBUS_EVENT_ID_RESULT(occupied, unit, UnitAbilityRequestBus, HasAbilityInSlot, slot);
			if (!occupied)return slot;
		}
		return Slot();
	}


#define REPLICATE_VALUE(memberVal) \
if (m_replicaChunk) { if (ShopComponentChunk* chunk = static_cast<ShopComponentChunk*>(m_replicaChunk.get())){	chunk-> memberVal .Set(memberVal); } }


	ItemId ShopComponent::BuyItem(UnitId unit, ItemTypeId itemType)
	{
		AZ_Printf(0, "shop : called BuyItem (\"%s\")", itemType.c_str());
		ItemId itemId;
		if (!unit.IsValid())return itemId;

		auto it = m_itemList.begin();
		for (; it != m_itemList.end(); ++it)
		{
			if (it->id == itemType)break;
		}

		auto Error = [](const char* s)->ItemId {AZ_Warning(0, false, s); return ItemId(); };


		//check if the shop sell the item 
		if (it == m_itemList.end())return Error("The shop don't sell that item");

		if (it->stock == 0) return Error("item is out of stock");
		if (!IsInRange(unit))return Error("shop is not in range");


		if (!HasEnoughResources(unit, it->price)) return Error("you can't afford this item");

		//check if the unit has the needed base items
		AZStd::vector<ItemId> items_to_delete;
		AZStd::string infoStr;
		EBUS_EVENT_RESULT(infoStr, InfoRequestBus, GetItemJsonString, itemType);
		rapidjson::Document info;
		info.Parse(infoStr.c_str());

		if (info.IsObject() && info.HasMember("dependencies"))
		{
			auto &dependencies = info["dependencies"];
			if (dependencies.IsArray())
			{
				for (rapidjson::Value::ConstValueIterator it = dependencies.Begin(); it != dependencies.End(); ++it)
				{
					//check if the unit have each required item
					ItemId id;
					EBUS_EVENT_ID_RESULT(id, unit, UnitAbilityRequestBus, HasAbilityType, it->GetString());//not checking if IsString() since the dependencies are generated

					if (!id.IsValid())return Error("you don't have the required items");//todo specify the missing item?

					items_to_delete.push_back(id);
				}
			}
		}

		Slot slot = GetFirstFreeSlot(unit);
		if (!slot.IsValid() && items_to_delete.empty())return Error("you don't have space in your inventory");


		//all checks passed

		//destroy the required items
		for (auto id : items_to_delete)
		{
			//detach it
			EBUS_EVENT_ID(unit, UnitAbilityRequestBus, DetachAbility, id);
			//destroy it
			EBUS_EVENT(AzFramework::GameEntityContextRequestBus, DestroyGameEntity, id);
		}

		if (!items_to_delete.empty()) slot = GetFirstFreeSlot(unit);

		ConsumeResources(unit, it->price);
		
		it->stock = AZStd::max(-1, it->stock - 1);

		if (it->stock >= 0)
		{
			REPLICATE_VALUE(m_itemList);
		}

		EBUS_EVENT_RESULT(itemId, GameManagerRequestBus, CreateItem, itemType);
		EBUS_EVENT_ID(unit, UnitAbilityRequestBus, SetAbilityInSlot, slot, itemId);

		return itemId;
	}

	void ShopComponent::SellItem(UnitId unit, ItemId itemId)
	{
		if (!unit.IsValid())return;
		if (!itemId.IsValid())return;
		if (!IsInRange(unit))return;

		Slot slot;
		EBUS_EVENT_ID_RESULT(slot, unit, UnitAbilityRequestBus, GetAbilitySlot, itemId);

		if (!slot.IsValid())return;

		//todo check if the shop accept to buy the item


		//all checks passed

		EBUS_EVENT(UnitAbilityRequestBus, ClearAbilitySlot, slot);
		EBUS_EVENT(AzFramework::GameEntityContextRequestBus, DestroyGameEntity, itemId);

		Value sellValue = 0.0f;
		EBUS_EVENT_RESULT(sellValue, VariableManagerRequestBus, GetValue, VariableId(GetEntityId(), "price"));//or value?

		Value gold = 0.0f;
		EBUS_EVENT_RESULT(gold, VariableManagerRequestBus, GetValue, VariableId(GetEntityId(), "gold"));
		gold += sellValue;
		EBUS_EVENT(VariableManagerRequestBus, SetValue, VariableId(GetEntityId(), "gold"), gold);

		//todo add 1 to the stock (need a way to get the ItemTypeId)

	}


	bool spark::ShopComponent::IsInRange(UnitId unit)
	{
		AZ::Vector3 position;
		EBUS_EVENT_ID_RESULT(position, unit, AZ::TransformBus, GetWorldTranslation);
		bool in_range = false;
		EBUS_EVENT_ID_RESULT(in_range, GetEntityId(), LmbrCentral::ShapeComponentRequestsBus, IsPointInside, position);
		return in_range;
	}

	ItemList ShopComponent::GetItemList()
	{
		return m_itemList;
	}


	void ShopComponent::AddItemShopInfo(ItemShopInfo itemShopInfo)
	{
		auto it = m_itemList.begin();
		for (; it != m_itemList.end(); ++it)
		{
			if (it->id == itemShopInfo.id)break;
		}

		//check if the shop already sell the item 
		if (it == m_itemList.end()) {
			//not present, insert it
			m_itemList.push_back(itemShopInfo);
		}
		else
		{
			//already present, override it
			*it = itemShopInfo;
		}

		REPLICATE_VALUE(m_itemList);
	}

	void ShopComponent::AddItem(ItemTypeId itemType)
	{
		rapidjson::Value *json = nullptr;
		EBUS_EVENT_RESULT(json, InfoRequestBus, GetItemJson, itemType);

		if (!json)
		{
			sERROR("ItemTypeId does not exist in the gamemode file");
			return;
		}

		auto it = m_itemList.begin();
		for (; it != m_itemList.end(); ++it)
		{
			if (it->id == itemType)break;
		}

		//check if the shop already sell the item 
		if (it == m_itemList.end()) {
			//not present, insert it
			ItemShopInfo info;
			info.id = itemType;
			info.stock = -1; //default infinite stock size

			if (json->HasMember("price"))
			{
				info.price = CostsFromJson((*json)["price"]); //default price
			}

			m_itemList.push_back(info);

			sLOG("added " + itemType + " to the shop item list");
		}
		else
		{
			//already present
			sWARNING(itemType + " is already present");
		}

		REPLICATE_VALUE(m_itemList);
	}

	//void spark::ShopComponent::AddItem(ItemTypeId itemType, Price price, int stock)
	//{
	//	auto it = m_itemList.begin();
	//	for (; it != m_itemList.end(); ++it)
	//	{
	//		if (it->id == itemType)break;
	//	}
	//
	//	//todo uncomment this and add a way to add the list of items 
	//
	//	//check if the shop sell the item 
	//	if (it == m_itemList.end()) {
	//		ItemShopInfo info;
	//		info.id = itemType;
	//		info.price = price;
	//		info.stock = stock;
	//		m_itemList.push_back(info);
	//	}
	//	else
	//	{
	//		//not sure about that case
	//
	//		if (it->stock == -1)return;
	//		it->stock += stock;
	//		it->price = price;
	//	}
	//
	//	AZ_Printf(0, "added \"%s\" to the shop", itemType.c_str());
	//
	//}

	void ShopComponent::GetAllShops(AZStd::vector<AZ::EntityId>& result)
	{
		result.push_back(GetEntityId());
	}



	GridMate::ReplicaChunkPtr ShopComponent::GetNetworkBinding()
	{
		auto replicaChunk = GridMate::CreateReplicaChunk<ShopComponentChunk>();
		replicaChunk->SetHandler(this);
		m_replicaChunk = replicaChunk;

		if (ShopComponentChunk* chunk = static_cast<ShopComponentChunk*>(m_replicaChunk.get()))
		{
			chunk->m_itemList.Set(m_itemList);
		}

		return m_replicaChunk;
	}

	void ShopComponent::SetNetworkBinding(GridMate::ReplicaChunkPtr chunk)
	{
		chunk->SetHandler(this);
		m_replicaChunk = chunk;

		
		if (ShopComponentChunk* chunk = static_cast<ShopComponentChunk*>(m_replicaChunk.get()))
		{
			m_itemList = chunk->m_itemList.Get();
		}
	}

	void ShopComponent::UnbindFromNetwork()
	{
		if (m_replicaChunk)
		{
			m_replicaChunk->SetHandler(nullptr);
			m_replicaChunk = nullptr;
		}
	}

	void ShopComponent::OnNewItemList(const ItemList& itemList, const GridMate::TimeContext& tc)
	{
		m_itemList = itemList;
	}

}