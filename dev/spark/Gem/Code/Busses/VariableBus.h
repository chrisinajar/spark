#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/ComponentBus.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Math/Vector3.h>

#include <AzCore/RTTI/BehaviorContext.h>

#include <GridMate/Serialize/DataMarshal.h>
#include "Utils/Marshaler.h"

namespace spark 
{

	struct VariableData 
	{
		using Value = float;
		using IdType = AZStd::string;
		Value value=0;
		Value bonusValue = 0;
		Value maxValue=-1;
		Value minValue=0;
		bool isDependent=false;//if true the value is setted by the GetValue method of the variable handler each tick

		VariableData()
			: value(0)
			, bonusValue(0)
			, maxValue(-1)
			, minValue(0)
			, isDependent(false)
		{
		}
		VariableData(const VariableData& other)
			: value(other.value)
			, bonusValue(other.bonusValue)
			, maxValue(other.maxValue)
			, minValue(other.minValue)
			, isDependent(other.isDependent)
		{
		}
		
		void operator=(const VariableData& other)
		{
			value = other.value;
			bonusValue = other.bonusValue;
			maxValue = other.maxValue;
			minValue = other.minValue;
			isDependent = other.isDependent;
		}

		bool operator==(const VariableData& other) const
		{
			return value == other.value &&
				bonusValue == other.bonusValue &&
				maxValue == other.maxValue &&
				minValue == other.minValue &&
				isDependent == other.isDependent;
		}

		AZStd::string ToString() const
		{
			return AZStd::string::format("VariableData(value=%f,bonus=%f,%s)", value, bonusValue, isDependent ? "dependent" : "not dependent");
		}
	};

	namespace 
	{
		using Value = VariableData::Value;
		using IdType = AZStd::string;
	}

	class VariableId
    {
		friend class VariableManagerComponent;
		friend class VariableHolderComponent;
    public:
        AZ_TYPE_INFO(VariableId, "{A90A3EA3-6004-4585-95AE-93BC2D2ADA72}");
		
		~VariableId() = default;

		VariableId() = default;

		explicit VariableId(AZ::EntityId entityId, const char* variableId)
			: m_entityId(entityId)
			, m_variableId(variableId)
		{}

		explicit VariableId(AZ::EntityId entityId, AZStd::string variableId)
			: m_entityId(entityId)
			, m_variableId(variableId)
		{}

		static void Reflect(AZ::ReflectContext* reflection);

		inline bool operator==(const VariableId& rhs) const
		{
			return rhs.m_entityId == m_entityId &&
				   rhs.m_variableId == m_variableId;
		}

		inline bool operator!=(const VariableId& rhs) const
		{
			return !((*this) == rhs);
		}

		AZStd::string ToString() const
		{
			return AZStd::string::format("VariableId(entityId=%s, variable=%s)", m_entityId.IsValid() ? m_entityId.ToString().c_str() : "invalid", m_variableId.c_str());
		}
		AZ::EntityId GetEntityId() const { return m_entityId; }
		AZStd::string GetVariableId() const { return m_variableId; }

	private:
        AZ::EntityId m_entityId;
        AZStd::string m_variableId;
    };


    // C++ -> lua calls for getting variable values
	class VariableRequests
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		/**
		 * Overrides the default AZ::EBusTraits address policy so that the bus
		 * has multiple addresses at which to receive messages. This bus is 
		 * identified by EntityId. Messages addressed to an ID are received by 
		 * handlers connected to that ID.
		 */
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
		/**
		 * Overrides the default AZ::EBusTraits ID type so that VariableId IDs are 
		 * used to access the addresses of the bus.
		 */
		typedef VariableId BusIdType;
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		//////////////////////////////////////////////////////////////////////////

		virtual Value GetValue(float deltaTime, AZ::ScriptTimePoint time) = 0;
		
		virtual Value OnSet(Value current,Value proposed) = 0;

		virtual Value GetMinValue() = 0;
		virtual Value GetMaxValue() = 0;
	};
	using VariableRequestBus = AZ::EBus<VariableRequests>;

	// * -> C++ (* -> manager) calls to get/set/register/etc variables
	class VariableManagerRequests
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
		//////////////////////////////////////////////////////////////////////////
		
		virtual bool  RegisterDependentVariable(VariableId) = 0;
		virtual bool  RegisterVariable(VariableId) = 0;
		virtual bool  RegisterVariableAndInizialize(VariableId,Value) = 0;
		virtual bool  VariableExists(VariableId) = 0;
		virtual bool  SetValue(VariableId, Value) = 0;

		virtual Value GetValue(VariableId) = 0;
		virtual Value GetBaseValue(VariableId) = 0;
		virtual Value GetBonusValue(VariableId) = 0;

		virtual AZStd::string ToString() = 0;//for debugging
	};
	using VariableManagerRequestBus = AZ::EBus<VariableManagerRequests>;



	// C++ -> C++ (manager -> holder) calls to get/set/register/etc variables
	class VariableHolderRequests
		: public AZ::ComponentBus
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		//////////////////////////////////////////////////////////////////////////

		virtual bool  RegisterDependentVariable(VariableId) = 0;
		virtual bool  RegisterVariable(VariableId) = 0;
		virtual bool  RegisterVariableAndInizialize(VariableId,Value) = 0;
		virtual bool  VariableExists(VariableId) = 0;
		virtual bool  SetValue(VariableId, Value) = 0;

		virtual Value GetValue(VariableId) = 0;
		virtual Value GetBaseValue(VariableId) = 0;
		virtual Value GetBonusValue(VariableId) = 0;
	};
	using VariableHolderRequestBus = AZ::EBus<VariableHolderRequests>;



	class VariableBonusModifierRequests
	: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
		typedef VariableId BusIdType;
		//////////////////////////////////////////////////////////////////////////

		virtual Value GetModifierBonus(AZStd::string id) = 0;
	};
	using VariableBonusModifierBus = AZ::EBus<VariableBonusModifierRequests>;



	class VariableNotifications
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
		typedef VariableId BusIdType;
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		//////////////////////////////////////////////////////////////////////////

		virtual void OnSetValueFilter(VariableId, Value) {}
		virtual void OnSetValue(VariableId, Value) {}
	};
	using VariableNotificationBus = AZ::EBus<VariableNotifications>;

	class VariableHolderNotifications
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
		typedef AZ::EntityId BusIdType;
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		//////////////////////////////////////////////////////////////////////////

		virtual void OnSetValueFilter(VariableId, Value) {}
		virtual void OnSetValue(VariableId, Value) {}
	};
	using VariableHolderNotificationBus = AZ::EBus<VariableHolderNotifications>;

	class VariableManagerNotifications
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
		//////////////////////////////////////////////////////////////////////////

		virtual void OnSetValueFilter(VariableId, Value){}
		virtual void OnSetValue(VariableId, Value) {}
	};
	using VariableManagerNotificationBus = AZ::EBus<VariableManagerNotifications>;
}

// marshaler
template<>
class GridMate::Marshaler<spark::VariableId>
{
public:
	typedef spark::VariableId DataType;

	AZ_FORCE_INLINE void Marshal(WriteBuffer& wb, const DataType& value) const
	{
		// AZ_Printf(0, "Marshal<VariableId> : %s", value.ToString().c_str());
		Marshaler<AZStd::string> stringMarshaler;
		Marshaler<AZ::EntityId>  entityIdMarshaler;

		entityIdMarshaler.Marshal(wb, value.GetEntityId());
		stringMarshaler.Marshal(wb, value.GetVariableId());
	}
	AZ_FORCE_INLINE void Unmarshal(DataType& value, ReadBuffer& rb) const
	{
		Marshaler<AZStd::string> stringMarshaler;
		Marshaler<AZ::EntityId>  entityIdMarshaler;

		AZ::EntityId id;
		AZStd::string str;

		entityIdMarshaler.Unmarshal(id, rb);
		stringMarshaler.Unmarshal(str, rb);

		value = spark::VariableId(id, str);
		// AZ_Printf(0, "Unmarshal<VariableId> : %s", value.ToString().c_str());
	}
};

template<>
class GridMate::Marshaler<spark::VariableData>
{
public:
	typedef spark::VariableData DataType;
	static const AZStd::size_t MarshalSize = sizeof(DataType);

	AZ_FORCE_INLINE void Marshal(WriteBuffer& wb, const DataType& value) const
	{
		// AZ_Printf(0, "Marshal<VariableData> : %s", value.ToString().c_str());
		DataType temp = value;
		wb.WriteRaw(&temp, sizeof(temp));
	}
	AZ_FORCE_INLINE void Unmarshal(DataType& value, ReadBuffer& rb) const
	{
		rb.ReadRaw(&value, sizeof(value));
		// AZ_Printf(0, "Unmarshal<VariableData> : %s", value.ToString().c_str());
	}
};

template<>
struct AZStd::hash<spark::VariableId>
{
	AZ_FORCE_INLINE AZStd::size_t operator()(const spark::VariableId& value) const
	{
		AZStd::size_t seed = 0;
		hash_combine(seed, value.GetVariableId());
		hash_combine(seed, value.GetEntityId());

		return seed;
	}
};