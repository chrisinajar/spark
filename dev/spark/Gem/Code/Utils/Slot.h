#pragma once

#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/std/string/string.h>
#include <GridMate/Serialize/DataMarshal.h>

namespace spark 
{
	class Slot {
	public:

		AZ_TYPE_INFO(Slot, "{9B5469C2-DA06-4FCD-9A36-047CC60B4FC7}");
		static void Reflect(AZ::ReflectContext* reflection);


		enum Type {
			Invalid = 0,
			Ability,
			//Ultimate,
			Inventory,
			Attack,
			Queue,
			Stop,
			Shop,
			Chat,

			NUM_TYPES
		};
		
		Slot() {}
		Slot(Type t,int index=0):m_type(t),m_index(index){}

		int GetIndex() const { return m_index; }
		Type GetType() const { return m_type;  }

		bool IsValid() const { return m_type != Invalid; }
		operator bool() const { return IsValid(); }
		bool operator==(Type type) const { return type == m_type; }

		bool operator==(const Slot &other) const { return other.m_type == m_type && other.m_index==m_index; }
		bool operator!=(const Slot &other) const { return !((*this) == other); }

		bool Equal(const Slot &other) const { return *this == other; }
		bool IsType(Type type) const { return type == m_type; }

		AZStd::string ToString() const;
	// protected:
		Type m_type=Invalid;
		int  m_index=0;
	};



	template<Slot::Type type>
	class SpecificSlot
		: public Slot
	{
	public:
		SpecificSlot(const Slot &s):Slot(s.GetType()==type ? type : Invalid,s.GetIndex()){}
		SpecificSlot(int index=0):Slot(type,index){}
	};

	using AbilitySlot = SpecificSlot<Slot::Ability>;
	using InventorySlot = SpecificSlot<Slot::Inventory>;

	
}

template<>
class GridMate::Marshaler<spark::Slot>
{
public:
	typedef spark::Slot DataType;

    static const AZStd::size_t MarshalSize = sizeof(int) + sizeof(spark::Slot::Type);

	AZ_FORCE_INLINE void Marshal(WriteBuffer& wb, const DataType& value) const
	{
		Marshaler<int> intMarshaler;
		Marshaler<spark::Slot::Type> typeMarshaler;
		intMarshaler.Marshal(wb, value.m_index);
		typeMarshaler.Marshal(wb, value.m_type);
	}
	AZ_FORCE_INLINE void Unmarshal(DataType& value, ReadBuffer& rb) const
	{
		Marshaler<int> intMarshaler;
		Marshaler<spark::Slot::Type> typeMarshaler;
		intMarshaler.Unmarshal(value.m_index, rb);
		typeMarshaler.Unmarshal(value.m_type, rb);
	}
};
