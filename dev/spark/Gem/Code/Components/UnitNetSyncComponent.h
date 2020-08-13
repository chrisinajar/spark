#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TransformBus.h>
#include <GridMate/Replica/ReplicaChunk.h>
#include <AzFramework/Network/NetBindable.h>

#include "Busses/NetSyncBusses.h"
#include "Busses/UnitNavigationBus.h"
#include "Busses/UnitBus.h"

namespace spark
{
	class UnitNetSyncComponent
		: public AZ::Component
		, protected UnitNavigationNotificationBus::Handler
		, protected UnitNotificationBus::Handler
		, protected UnitNetSyncRequestBus::Handler
		, public AzFramework::NetBindable
	{
	public:
		AZ_COMPONENT(UnitNetSyncComponent, "{FCB4D3CF-8F5F-4265-985A-D452EA701AAA}", AzFramework::NetBindable);

		static void Reflect(AZ::ReflectContext* context);

		void Init() override;
		void Activate() override;
		void Deactivate() override;

		GridMate::ReplicaChunkPtr GetNetworkBinding();
		void SetNetworkBinding(GridMate::ReplicaChunkPtr chunk);
		void UnbindFromNetwork();

		void OnPathChanged(const SparkNavPath &path) override;
		bool OnNetPathChanged(SparkNavPath path, const GridMate::RpcContext &);

		bool OnNetTransformChanged(const AZ::Transform& transform, const GridMate::RpcContext&);

		void Synchronize();
		bool OnNetSynchronize(const GridMate::RpcContext&);

		// UnitNetSyncRequestBus + rpc
		void SetNamedParameterBool (const AZStd::string &valueName, const bool &value);
		bool OnNetSetNamedParameterBool (const AZStd::string &valueName, const bool &value, const GridMate::RpcContext &context);
		void SetNamedParameterFloat (const AZStd::string &valueName, const float &value);
		bool OnNetSetNamedParameterFloat (const AZStd::string &valueName, const float &value, const GridMate::RpcContext &context);


		// UnitNotificationBus + rpc
		void OnNewOrder(UnitOrder, bool queue) override;
		bool OnNetOnNewOrder(UnitOrder, bool queue, const GridMate::RpcContext&);
		void OnModifierAttached(ModifierId) override;
		bool OnNetOnModifierAttached(ModifierId, const GridMate::RpcContext&);
		void OnModifierDetached(ModifierId) override;
		bool OnNetOnModifierDetached(ModifierId, const GridMate::RpcContext&);
		void OnSpawned() override;
		bool OnNetOnSpawned(const GridMate::RpcContext&);
		void OnDeath() override;
		bool OnNetOnDeath(const GridMate::RpcContext&);
		//bool OnNewOrder(AZ::EntityId unit,UnitOrder, bool queue,const GridMate::RpcContext&);
	private:
		
		//void SendNewOrder(AZ::EntityId unit, UnitOrder, bool queue) override;

		bool m_isAuthoritative = false;

		GridMate::ReplicaChunkPtr m_chunk;
	};
}