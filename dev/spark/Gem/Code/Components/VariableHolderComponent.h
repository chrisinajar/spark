#pragma once

#include <AzCore/Component/Component.h>
#include <AzFramework/Entity/GameEntityContextBus.h>
#include <AzFramework/Network/NetBindable.h>

#include "Components/VariableManagerComponent.h"
#include "Busses/VariableBus.h"
#include "Utils/Filter.h"

namespace spark
{
	class VariableHolderComponent
		: public AZ::Component
		, protected AZ::TickBus::Handler
		, public VariableHolderRequestBus::Handler
		, public AzFramework::NetBindable
	{
		// friends
		friend class VariableHolderComponentChunk;
	public:
		AZ_COMPONENT(VariableHolderComponent, "{E697D9E0-1528-4A10-8F7E-C3690304D177}", AzFramework::NetBindable)

		typedef AZStd::unordered_map<AZStd::string, VariableData> VariableMap;
		typedef AZStd::pair<AZStd::string, VariableData> VariableDataPair;

		static void Reflect(AZ::ReflectContext* reflection);

		static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
		static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
		static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
		static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

		void Init () override;
		void Activate () override;
		void Deactivate () override;

		//NetBindable
		GridMate::ReplicaChunkPtr GetNetworkBinding ();
		void SetNetworkBinding (GridMate::ReplicaChunkPtr chunk);
		void UnbindFromNetwork ();

		// variable holder request bus
		bool  RegisterDependentVariable (VariableId) override;
		bool  RegisterVariable (VariableId) override;
		bool  RegisterVariableAndInizialize (VariableId,Value) override;
		bool  VariableExists (VariableId) override;
		bool  SetValue (VariableId, Value) override;

		Value GetValue (VariableId) override;
		Value GetBaseValue (VariableId) override;
		Value GetBonusValue (VariableId) override;

		void UpdateValues ();
		void UpdateValue (const AZStd::string& valueName);
		void OnNewVariables (const VariableMap& variables, const GridMate::TimeContext& tc);
		void OnNewDataPair (const VariableDataPair& dataPair, const GridMate::TimeContext& tc);
		void OnTick (float deltaTime, AZ::ScriptTimePoint time) override;

	private:
		GridMate::ReplicaChunkPtr m_replicaChunk;
		VariableMap m_variables;

		bool m_updating = false;
		bool m_hasChanged = false;
		AZStd::unordered_map<AZStd::string, int> m_valueIndexMap;
		int m_replicaValueIndex = 0;
		int m_replicaValueCount = 0;
	};
}
