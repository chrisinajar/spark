#pragma once


#include <AzCore/Component/EntityId.h>
#include <AzCore/std/string/string.h>
#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/JSON/document.h>

#include <GridMate/Serialize/DataMarshal.h>
#include "Marshaler.h"

namespace spark
{

	class Amount
	{
	public:
		AZ_TYPE_INFO(Amount, "{8C1E5D07-AC0F-43FC-AC7E-C10B341E21D1}");
		static void Reflect(AZ::ReflectContext* reflection);

		Amount() :amount(0.0f) {}
		Amount(AZStd::string variable, float amount):variable(variable), amount(amount) {}

		AZ_FORCE_INLINE bool operator== (const Amount& other) const
		{
			return amount == other.amount &&
				other.variable.compare(variable) == 0;
		}
		
		AZStd::string variable;
		float amount;
	};

	using Costs = AZStd::vector<Amount>;
	using Price = Costs;


	//check if the unit can cover the costs of the casting/buying
	bool HasEnoughResources(AZ::EntityId id, Costs costs);

	Costs GetDifferences(AZ::EntityId id, Costs costs);

	bool ConsumeResources(AZ::EntityId id, Costs costs);
	

	Costs CostsFromJson(const rapidjson::Value &value);
}

// marshaler
template<>
class GridMate::Marshaler<spark::Amount>
{
public:
	typedef spark::Amount DataType;

	AZ_FORCE_INLINE void Marshal(WriteBuffer& wb, const DataType& value) const
	{
		Marshaler<AZStd::string> stringMarshaler;
		Marshaler<float> floatMarshaler;

		stringMarshaler.Marshal(wb, value.variable);
		floatMarshaler.Marshal(wb, value.amount);
	}
	AZ_FORCE_INLINE void Unmarshal(DataType& value, ReadBuffer& rb) const
	{
		Marshaler<AZStd::string> stringMarshaler;
		Marshaler<float> floatMarshaler;

		stringMarshaler.Unmarshal(value.variable, rb);
		floatMarshaler.Unmarshal(value.amount, rb);
	}
};
