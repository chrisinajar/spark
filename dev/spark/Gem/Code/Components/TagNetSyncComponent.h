#pragma once


#include <AzCore/Component/Component.h>
#include <AzCore/Component/ComponentBus.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Math/Vector3.h>
#include <LmbrCentral/Ai/NavigationComponentBus.h>
#include <AzFramework/Network/NetBindable.h>

#include <LmbrCentral/Scripting/TagComponentBus.h>

namespace spark
{
	class TagNetSyncComponent
		: public AZ::Component
		, protected LmbrCentral::TagComponentNotificationsBus::Handler
		, public AZ::TickBus::Handler
		, public AzFramework::NetBindable
	{
	public:
		AZ_COMPONENT(TagNetSyncComponent, "{17779D8B-9F7C-453C-BB40-2B2E3A339807}", AzFramework::NetBindable)

		~TagNetSyncComponent() override {};

		static void Reflect(AZ::ReflectContext* reflection);

		void Init() override;
		void Activate() override;
		void Deactivate() override;

		void Despawn();
		// Called during network binding on the master. Implementations should create and return a new binding.
		GridMate::ReplicaChunkPtr GetNetworkBinding() override;
		void SetNetworkBinding(GridMate::ReplicaChunkPtr chunk) override;
		void UnbindFromNetwork() override;

		void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        void OnTagAdded(const LmbrCentral::Tag&) override;
        void OnTagRemoved(const LmbrCentral::Tag&) override;

		void OnNewTags (const LmbrCentral::Tags &tags, const GridMate::TimeContext&);
		void SyncTags ();
	private:
		GridMate::ReplicaChunkPtr m_replicaChunk = nullptr;
		bool m_needsUpdate = false;
		LmbrCentral::Tags m_tags;
	};
}
