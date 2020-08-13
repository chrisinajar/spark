#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzFramework/Entity/GameEntityContextBus.h>
#include <AzFramework/Network/NetBindable.h>
#include <GridMate/Session/Session.h>

#include <AzCore/std/containers/unordered_map.h>
#include "Busses/VariableBus.h"
#include "Utils/Marshaler.h"


namespace spark
{
	struct VariablesData;

	class VariableManagerComponent
		: public AZ::Component
		, public VariableManagerRequestBus::Handler
		, public GridMate::SessionEventBus::Handler
		, public AzFramework::NetBindable
	{
	public:
		AZ_COMPONENT(VariableManagerComponent, "{BF73AA43-596F-49B7-A363-B3921EA7FEC0}", AzFramework::NetBindable)

		using VariableMap = AZStd::unordered_map<VariableId, VariableData>;
		using VariableIdMap = AZStd::unordered_map<AZ::EntityId, AZStd::vector<AZStd::string>>;

		~VariableManagerComponent() override {};

		static void Reflect(AZ::ReflectContext* reflection);

		void Init() override;
		void Activate() override;
		void Deactivate() override;

		//NetBindable
		GridMate::ReplicaChunkPtr GetNetworkBinding() override;
		void SetNetworkBinding(GridMate::ReplicaChunkPtr chunk) override;
		void UnbindFromNetwork() override;

		//SessionEventBus
		/// Callback that notifies the title when a new member joins the game session.
		void OnMemberJoined(GridMate::GridSession* session, GridMate::GridMember* member);
        /// Callback that notifies the title that a member is leaving the game session. member pointer is NOT valid after the callback returns.
		void OnMemberLeaving(GridMate::GridSession* session, GridMate::GridMember* member);

		//VariableManagerRequestBus
		bool  RegisterDependentVariable(VariableId);
		bool  RegisterVariable(VariableId);
		bool  RegisterVariableAndInizialize(VariableId,Value);
		bool  VariableExists(VariableId);
		Value GetValue(VariableId);
		Value GetBaseValue(VariableId);
		Value GetBonusValue(VariableId);
		bool  SetValue(VariableId, Value);
		bool  OnNetSetValue(VariableId, Value, const GridMate::RpcContext &);
		void OnNewVariables (const VariableMap& variables, const GridMate::TimeContext& tc);
		void OnNewVariablesByEntity (const VariableIdMap& variablesByEntity, const GridMate::TimeContext& tc);

		AZStd::string ToString();

	private:
		void UpdateValues();

		bool m_updating = false;
		bool m_hasChanged = false;

		VariableMap m_variables;
		VariableIdMap m_variablesByEntity;

		AZStd::string m_log;

		GridMate::ReplicaChunkPtr m_replicaChunk;
		int m_tickCounter = 0;

		friend class VariableManagerComponentChunk;
	};

}
