#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/string/string.h>

namespace spark 
{
	class ChatMessage
	{
	public:
		AZ_TYPE_INFO(ChatMessage, "{CE4D6CBC-2A78-4980-919D-C236EDDCFE7B}");
		static void Reflect(AZ::ReflectContext* reflection);

		ChatMessage() {}
		ChatMessage(AZStd::string message, AZStd::string author) :message(message), author(author) {}

		AZStd::string GetText() const;
		AZStd::string GetAuthor() const;

		AZStd::string ToString() const;

	private:
		AZStd::string message;
		AZStd::string author;

		//todo store timestamp
	};

	class SparkChatRequests
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;

		
		virtual AZStd::vector<ChatMessage> GetMessages() = 0;
		virtual ChatMessage GetLastMessage() = 0;

		virtual void SendChatMessage(ChatMessage) = 0;
	};
	using SparkChatRequestBus = AZ::EBus<SparkChatRequests>;


	class SparkChatNotifications
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;

		
		virtual void OnNewMessage(ChatMessage) {};
	};
	using SparkChatNotificationBus = AZ::EBus<SparkChatNotifications>;

	
}
