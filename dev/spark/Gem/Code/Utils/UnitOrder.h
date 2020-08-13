#pragma once

#include <AzCore/Math/Vector3.h>
#include <AzCore/Component/EntityId.h>
#include <GridMate/Serialize/DataMarshal.h>

#include "Busses/AbilityBus.h"
#include "Marshaler.h"

namespace spark {
	class UnitOrder {
 	public:

		enum Type {
			INVALID,
			STOP,
			MOVE,
			FOLLOW,
			ATTACK,
			ATTACK_MOVE,
			CAST,
			BUY,
			SELL,
			DROP,
			PICKUP,
			UPGRADE,
		};
		int type=STOP;

		AZ_TYPE_INFO(UnitOrder, "{05FEB582-2C60-48FD-AE6C-7BD23AA27E14}");
		UnitOrder(UnitOrder::Type t=INVALID) :type(t) {}

		static void Reflect(AZ::ReflectContext* reflection);

		AZ::Vector3 position;
		AZ::EntityId target;
		AZStd::string typeId;
		float distance;
		CastContext castContext;
		
 		AZStd::string ToString() const{
			switch (type)
			{
			case STOP:
				return "UnitOrder(STOP)";
			case MOVE:
				return AZStd::string::format("UnitOrder(MOVE, position=(%f,%f,%f) )", (float)position.GetX(), (float)position.GetY(), (float)position.GetZ());
			case FOLLOW:
				return AZStd::string::format("UnitOrder(FOLLOW, target=%s, distance=%f)",target.ToString().c_str(),distance);
			case ATTACK:
				return "UnitOrder(ATTACK, target=" + target.ToString() + ")";
			case ATTACK_MOVE:
				return AZStd::string::format("UnitOrder(ATTACK_MOVE, position=(%f,%f,%f) )", (float)position.GetX(), (float)position.GetY(), (float)position.GetZ());
			case CAST:
				return AZStd::string::format("UnitOrder(CAST, ability=%s )", castContext.ability.ToString().c_str());
			case BUY:
				return AZStd::string::format("UnitOrder(BUY, item=%s )", target.ToString().c_str());
			case SELL:
				return AZStd::string::format("UnitOrder(SELL, item=%s )", target.ToString().c_str());
			case DROP:
				return AZStd::string::format("UnitOrder(DROP, item=%s )", target.ToString().c_str());
			case PICKUP:
				return AZStd::string::format("UnitOrder(PICKUP, ability=%s )", target.ToString().c_str());
			case UPGRADE:
				return AZStd::string::format("UnitOrder(UPGRADE, ability=%s )", target.ToString().c_str());
			}
			return "UnitOrder(invalid)";
		}

 		static UnitOrder MoveOrder(AZ::Vector3 position) {
			UnitOrder c = { UnitOrder::MOVE };
			c.position = position;
			return c;
		}
 		static UnitOrder FollowOrder(AZ::EntityId target,float distance) {
			UnitOrder c = { UnitOrder::FOLLOW };
			c.target = target;
			c.distance = distance;
			return c;
		}
 		static UnitOrder AttackOrder(AZ::EntityId target) {
			UnitOrder c = { UnitOrder::ATTACK };
			c.target = target;
			return c;
		}
		static UnitOrder AttackMoveOrder(AZ::Vector3 position) {
			UnitOrder c = { UnitOrder::ATTACK_MOVE };
			c.position = position;
			return c;
		}
 		static UnitOrder StopOrder() {
			UnitOrder c = { UnitOrder::STOP };
			return c;
		}

		static UnitOrder CastOrder(CastContext castContext) {
			UnitOrder c = { UnitOrder::CAST };
			c.castContext = castContext;
			return c;
		}

		static UnitOrder BuyItemOrder(AZStd::string itemTypeId) {
			UnitOrder c = { UnitOrder::BUY };
			c.typeId = itemTypeId;
			return c;
		}
		static UnitOrder SellItemOrder(AZ::EntityId item) {
			UnitOrder c = { UnitOrder::SELL };
			c.target = item;
			return c;
		}
		static UnitOrder DropItemOrder(AZ::EntityId item) {
			UnitOrder c = { UnitOrder::DROP };
			c.target = item;
			return c;
		}
		static UnitOrder PickUpItemOrder(AZ::EntityId item) {
			UnitOrder c = { UnitOrder::PICKUP };
			c.target = item;
			return c;
		}
		static UnitOrder UpgradeOrder(AZ::EntityId ability) {
			UnitOrder c = { UnitOrder::UPGRADE };
			c.target = ability;
			return c;
		}

		bool operator==(const UnitOrder &other) const
		{
			if (type != other.type)return false;

			switch (type)
			{
			case ATTACK: return target == other.target;
			case MOVE: return position == other.position;
			case STOP: return true;
			}
			return false;
		}
 	private:
		
	};

}

template<>
class GridMate::Marshaler<spark::UnitOrder>
{
public:
	typedef spark::UnitOrder DataType;

	AZ_FORCE_INLINE void Marshal(WriteBuffer& wb, const DataType& value) const
	{
		DataType temp = value;
		temp.typeId.clear();
		temp.typeId.set_capacity(0);
		wb.WriteRaw(&temp, sizeof(temp));

		Marshaler<AZStd::string> stringMarshaler;
		stringMarshaler.Marshal(wb, value.typeId);
	}
	AZ_FORCE_INLINE void Unmarshal(DataType& value, ReadBuffer& rb) const
	{
		rb.ReadRaw(&value, sizeof(value));

		Marshaler<AZStd::string> stringMarshaler;
		stringMarshaler.Unmarshal(value.typeId, rb);
	}
};
