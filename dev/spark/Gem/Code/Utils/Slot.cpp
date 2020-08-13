#include "spark_precompiled.h"

#include "Slot.h"

#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

using namespace spark;

void SlotScriptConstructor(Slot* self, AZ::ScriptDataContext& dc)
{
	if (dc.GetNumArguments() == 0)
    {
        *self = Slot();
        return;
    }
	else if (dc.GetNumArguments() == 1)
	{
		if (dc.IsNumber(0))
		{
			int type=0;
			dc.ReadArg(0, type);
			new(self) Slot((Slot::Type)type);
			return;
		}
	}
    else if (dc.GetNumArguments() == 2)
    {
        if (dc.IsNumber(0) && dc.IsNumber(1))
        {
            int type=0,index=0;
			dc.ReadArg(0, type);
			dc.ReadArg(1, index);
			new(self) Slot((Slot::Type)type,index);
            return;
        }
    }

    dc.GetScriptContext()->Error(AZ::ScriptContext::ErrorType::Error, true, "Invalid arguments passed to Slot().");
    new(self) Slot();
}


void Slot::Reflect(AZ::ReflectContext* reflection)
{
	if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
    {
		serializationContext->Class<Slot>()
			->Version(1)
			->Field("index", &Slot::m_index)
			->Field("type",  &Slot::m_type);

    }

	if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
	{
		behaviorContext->Class<Slot>("Slot")
			->Attribute(AZ::Script::Attributes::Storage, AZ::Script::Attributes::StorageType::Value)
			->Attribute(AZ::Script::Attributes::ConstructorOverride, &SlotScriptConstructor)

			->Enum<(int)Slot::Invalid>("Invalid")
			->Enum<(int)Slot::Ability>("Ability")
			->Enum<(int)Slot::Inventory>("Inventory")
			->Enum<(int)Slot::Attack>("Attack")
			->Enum<(int)Slot::Queue>("Queue")
			->Enum<(int)Slot::Stop>("Stop")
			->Enum<(int)Slot::Shop>("Shop")
			->Enum<(int)Slot::Chat>("Chat")

			->Method("GetIndex", &Slot::GetIndex)
			->Method("GetType", &Slot::GetType)
			->Method("IsValid", &Slot::IsValid)
			->Method("ToString", &Slot::ToString)
			->Attribute(AZ::Script::Attributes::Operator, AZ::Script::Attributes::OperatorType::ToString)
			->Method("Equal", &Slot::Equal)
			->Attribute(AZ::Script::Attributes::Operator, AZ::Script::Attributes::OperatorType::Equal);

	}
}

AZStd::string Slot::ToString() const
{
	const char* types[] = { "Invalid","Ability","Inventory","Attack","Queue","Stop","Shop","Chat" };
	return AZStd::string::format("Slot(type:%d[%s],index:%d)",  m_type, m_type < NUM_TYPES ? types[m_type] : "unknown", m_index);
}