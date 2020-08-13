#include "spark_precompiled.h"

#include "Amount.h"

#include "Busses/VariableBus.h"

#include <AzCore/Script/ScriptContext.h>
#include <AzCore/Script/ScriptSystemBus.h>
#include <AzCore/Script/ScriptProperty.h>

#include "Utils/Log.h"
#include <AzCore/JSON/document.h>

namespace spark
{
	void AmountBehaviorScriptConstructor(Amount* self, AZ::ScriptDataContext& dc)
	{
		if (dc.GetNumArguments() == 0)
		{
			*self = Amount();
			return;
		}
		else if (dc.GetNumArguments() == 2)
		{
			if (dc.IsString(0) && dc.IsNumber(1) )
			{
				float amount = 0.0f;
				const char* variable = nullptr;
				dc.ReadArg(0, variable);
				dc.ReadArg(1, amount);

				new(self) Amount{ variable,amount };
				return;
			}
		}

		dc.GetScriptContext()->Error(AZ::ScriptContext::ErrorType::Error, true, "Invalid arguments passed to Amount().");
		new(self) Amount();
	}

	void Amount::Reflect(AZ::ReflectContext* reflection)
	{
		if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
		{
			serializationContext->Class<Amount>()
				->Version(1)
				->Field("variable", &Amount::variable)
				->Field("amount", &Amount::amount);

		}
		if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
		{
			behaviorContext->Class<Amount>("Amount")
				->Attribute(AZ::Script::Attributes::Storage, AZ::Script::Attributes::StorageType::Value)
				->Attribute(AZ::Script::Attributes::ConstructorOverride, &AmountBehaviorScriptConstructor)
				->Property("variable", BehaviorValueProperty(&Amount::variable))
				->Property("amount", BehaviorValueProperty(&Amount::amount));
		}
	}

	//check if the unit can cover the costs of the casting/buying
	bool HasEnoughResources(AZ::EntityId id, Costs costs)
	{
		for (auto c : costs)
		{
			Value value = 0;
			EBUS_EVENT_RESULT(value, VariableManagerRequestBus, GetValue, VariableId(id, c.variable));
			if (value < c.amount)
			{
				sWARNING("Not enough " + c.variable +"!");
				return false;
			}
		}
		return true;
	}

	bool ConsumeResources(AZ::EntityId id, Costs costs)
	{
		for (auto c : costs)
		{
			Value value = 0;
			EBUS_EVENT_RESULT(value, VariableManagerRequestBus, GetValue, VariableId(id, c.variable));
			value -= c.amount;
			EBUS_EVENT_RESULT(value, VariableManagerRequestBus, SetValue, VariableId(id, c.variable), value);
		}
		//todo check if the resources are correctly consumed, covering all the costs
		return true;
	}

	Costs GetDifferences(AZ::EntityId id, Costs costs)
	{
		//todo
		return Costs();
	}



	Costs CostsFromJson(const rapidjson::Value &value)
	{
		Costs costs;
		if (value.IsObject())
		{
			for (auto var = value.MemberBegin(); var != value.MemberEnd(); ++var)
			{
				if (var->value.IsNumber()) 
				{
					costs.emplace_back(var->name.GetString(), (float)var->value.GetDouble());
				}
			}
		}
		return costs;
	}

}
