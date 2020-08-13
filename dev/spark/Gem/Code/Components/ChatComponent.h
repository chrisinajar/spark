#pragma once

#include "Busses/ChatBus.h"
#include <AzCore/Component/Component.h>
#include <AzCore/std/containers/vector.h>
#include <AzFramework/Network/NetBindable.h>

namespace spark
{

	class SparkChatComponent
		: public AZ::Component
		, protected SparkChatRequestBus::Handler
		, public AzFramework::NetBindable
	{
	public:
		AZ_COMPONENT(SparkChatComponent, "{E75E4C08-5627-4595-9FBC-2B875208FEF6}", AzFramework::NetBindable);

		static void Reflect(AZ::ReflectContext* context);

		////////////////////////////////////////////////////////////////////////
		// AZ::Component interface implementation
		void Init() override;
		void Activate() override;
		void Deactivate() override;

		static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);
		////////////////////////////////////////////////////////////////////////

		//SparkChatRequestBus
		AZStd::vector<ChatMessage> GetMessages();
		ChatMessage GetLastMessage();

		void SendChatMessage(ChatMessage);

		//NetBindable
		GridMate::ReplicaChunkPtr GetNetworkBinding() override;
		void SetNetworkBinding(GridMate::ReplicaChunkPtr chunk) override;
		void UnbindFromNetwork() override;

		bool OnNewMessage(ChatMessage, const GridMate::RpcContext& rc);

	private:

		void OnNewMessageImpl(ChatMessage&);

		AZStd::vector<ChatMessage> m_messages;

		GridMate::ReplicaChunkPtr m_replicaChunk = nullptr;
	};

}
