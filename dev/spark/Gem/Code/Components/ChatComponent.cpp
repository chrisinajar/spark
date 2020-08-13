
#include "spark_precompiled.h"

#include "Components/ChatComponent.h"

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzFramework/Network/NetBindingHandlerBus.h>

#include <AzFramework/CommandLine/ConsoleBus.h>

#include "Utils/Log.h"



#include <AzFramework/Network/NetworkContext.h>
#include <AzFramework/Network/NetBindingHandlerBus.h>
#include <GridMate/Replica/ReplicaFunctions.h>
#include <GridMate/Replica/RemoteProcedureCall.h>

#include <GridMate/Replica/ReplicaChunk.h>
#include <GridMate/Replica/DataSet.h>
#include <GridMate/Serialize/CompressionMarshal.h>
#include "Utils/Marshaler.h"

using namespace GridMate;


template<>
class GridMate::Marshaler<spark::ChatMessage>
{
public:
	typedef spark::ChatMessage DataType;

	void Marshal(WriteBuffer& wb, const DataType& value) const
	{
		Marshaler<AZStd::string> stringMarshaler;

		stringMarshaler.Marshal(wb, value.GetAuthor());
		stringMarshaler.Marshal(wb, value.GetText());
	}
	void Unmarshal(DataType& value, ReadBuffer& rb) const
	{
		AZStd::string author, message;
		Marshaler<AZStd::string> stringMarshaler;
	
		stringMarshaler.Unmarshal(author, rb);
		stringMarshaler.Unmarshal(message, rb);

		value = spark::ChatMessage(message, author);
	}
};


namespace spark
{

	class ChatComponentChunk
		: public GridMate::ReplicaChunkBase
	{
	public:
		GM_CLASS_ALLOCATOR(ChatComponentChunk);

		ChatComponentChunk()
			: m_onNewMessage("newmsg")
		{}

		bool IsReplicaMigratable() override
		{
			return true;
		}

		static const char* GetChunkName()
		{
			return "ChatComponentChunk";
		}

		GridMate::Rpc<RpcArg<ChatMessage>>::BindInterface<SparkChatComponent, &SparkChatComponent::OnNewMessage> m_onNewMessage;
	};



	AZStd::string ChatMessage::GetText() const
	{
		return message;
	}
	AZStd::string ChatMessage::GetAuthor() const
	{
		return author;
	}

	AZStd::string ChatMessage::ToString() const
	{
		return author + ":" + message;
	}

	void ChatMessageScriptConstructor(ChatMessage* self, AZ::ScriptDataContext& dc)
	{
		if (dc.GetNumArguments() == 2)
		{
			if (dc.IsString(0) && dc.IsString(1))
			{
				AZStd::string msg, sender;
				dc.ReadArg(0, msg);
				dc.ReadArg(1, sender);
				new(self) ChatMessage(msg,sender);
				return;
			}
		}

		dc.GetScriptContext()->Error(AZ::ScriptContext::ErrorType::Error, true, "Invalid arguments passed to ChatMessage().");
		new(self) ChatMessage();
	}


	void ChatMessage::Reflect(AZ::ReflectContext* reflection)
	{
		if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
		{
			serializationContext->Class<ChatMessage>()
				->Version(1)
				->Field("message", &ChatMessage::message)
				->Field("author",  &ChatMessage::author);

		}

		if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
		{
			behaviorContext->Class<ChatMessage>("ChatMessage")
				->Attribute(AZ::Script::Attributes::Storage, AZ::Script::Attributes::StorageType::Value)
				->Attribute(AZ::Script::Attributes::ConstructorOverride, &ChatMessageScriptConstructor)
				->Method("GetText", &ChatMessage::GetText)
				->Method("GetAuthor", &ChatMessage::GetAuthor)
				->Method("ToString", &ChatMessage::ToString);
		}
	}
	 
	class SparkChatNotificationBusHandler
		: public SparkChatNotificationBus::Handler
		, public AZ::BehaviorEBusHandler
	{
	public:
		AZ_EBUS_BEHAVIOR_BINDER(SparkChatNotificationBusHandler, "{6FFC1999-5F3F-495E-8690-815488BF182E}", AZ::SystemAllocator, OnNewMessage);

		void OnNewMessage(ChatMessage msg) override
		{
			Call(FN_OnNewMessage, msg);
		}
	};

	void SparkChatComponent::Reflect(AZ::ReflectContext* reflection)
	{
		ChatMessage::Reflect(reflection);

		if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
		{
			serializationContext->Class<SparkChatComponent, AZ::Component>()
				->Version(1);

			if (auto editContext = serializationContext->GetEditContext())
			{
				editContext->Class<SparkChatComponent>("SparkChatComponent", "")
					->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					->Attribute(AZ::Edit::Attributes::Category, "spark")
					->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"));
			}
		}

		if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
		{
			behaviorContext->EBus<SparkChatRequestBus>("SparkChatRequestBus")
				->Event("GetMessages", &SparkChatRequestBus::Events::GetMessages)
				->Event("GetLastMessage", &SparkChatRequestBus::Events::GetLastMessage)
				->Event("SendChatMessage", &SparkChatRequestBus::Events::SendChatMessage)
				;

			behaviorContext->EBus<SparkChatNotificationBus>("SparkChatNotificationBus")
				->Handler<SparkChatNotificationBusHandler>()
				->Event("OnNewMessage", &SparkChatNotificationBus::Events::OnNewMessage)
				;
		}

		AzFramework::NetworkContext* netContext = azrtti_cast<AzFramework::NetworkContext*>(reflection);
		if (netContext)
		{
			netContext->Class<SparkChatComponent>()
				->Chunk<ChatComponentChunk>()
				->RPC<ChatComponentChunk, SparkChatComponent>("newmsg", &ChatComponentChunk::m_onNewMessage);
		}
	}

	void SparkChatComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
	{
		provided.push_back(AZ_CRC("SparkChatService"));
	}

	void SparkChatComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
	{
		incompatible.push_back(AZ_CRC("SparkChatService"));
	}

	void SparkChatComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
	{
		(void)required;
	}

	void SparkChatComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
	{
		(void)dependent;
	}

	void SparkChatComponent::Init()
	{
		AZ_Printf(0, "SparkChatComponent::Init()");
	}

	void SparkChatComponent::Activate()
	{

		SparkChatRequestBus::Handler::BusConnect();

		AZ_Printf(0, "SparkChatComponent::Activate()");
	}

	void SparkChatComponent::Deactivate()
	{
		SparkChatRequestBus::Handler::BusDisconnect();
	}

	//NetBindable
	GridMate::ReplicaChunkPtr SparkChatComponent::GetNetworkBinding()
	{
		auto replicaChunk = GridMate::CreateReplicaChunk<ChatComponentChunk>();
		replicaChunk->SetHandler(this);
		m_replicaChunk = replicaChunk;

		return m_replicaChunk;
	}

	void SparkChatComponent::SetNetworkBinding(GridMate::ReplicaChunkPtr chunk)
	{
		chunk->SetHandler(this);
		m_replicaChunk = chunk;
	}

	void SparkChatComponent::UnbindFromNetwork()
	{
		if (m_replicaChunk)
		{
			m_replicaChunk->SetHandler(nullptr);
			m_replicaChunk = nullptr;
		}
	}

	AZStd::vector<ChatMessage> SparkChatComponent::GetMessages()
	{
		return m_messages;
	}
	ChatMessage SparkChatComponent::GetLastMessage()
	{
		if (!m_messages.empty())
		{
			return *m_messages.rbegin();
		}

		return ChatMessage();
	}

	void SparkChatComponent::SendChatMessage(ChatMessage msg)
	{
		sLOG("SendChatMessage -> " + msg);

		if (m_replicaChunk)
		{
			ChatComponentChunk* replicaChunk = static_cast<ChatComponentChunk*>(m_replicaChunk.get());
			replicaChunk->m_onNewMessage(msg);
		}
		else
		{
			OnNewMessageImpl(msg);
		}
	}

	bool SparkChatComponent::OnNewMessage(ChatMessage msg, const GridMate::RpcContext& rc)
	{
		OnNewMessageImpl(msg);
		return true;
	}

	void SparkChatComponent::OnNewMessageImpl(ChatMessage &msg)
	{
		sLOG("OnNewMessage -> " + msg);

		bool consumed = false;
		EBUS_EVENT_RESULT(consumed, ConsoleNotificationBus, OnCommandFilter, msg.GetText().c_str());
		if (consumed)return;

		m_messages.push_back(msg);

		EBUS_EVENT(SparkChatNotificationBus, OnNewMessage, msg);
	}
}
