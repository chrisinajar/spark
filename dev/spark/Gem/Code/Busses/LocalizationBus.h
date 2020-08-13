#pragma once


#include <AzCore/Component/ComponentBus.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/string/string.h>

#include <AzCore/JSON/document.h>

namespace spark {

	class LocalizationRequests
		: public AZ::EBusTraits
	{
	public:
		// One handler is supported.
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		// The EBus uses a single address.
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;


		virtual AZStd::string LocalizeString(AZStd::string id) = 0;

		virtual void SetLanguage(AZStd::string language) = 0;
		virtual AZStd::string GetLanguage() = 0;
		virtual AZStd::vector<AZStd::string> GetAvailableLanguages() = 0;

	};
	using LocalizationRequestBus = AZ::EBus<LocalizationRequests>;


	class LocalizationNotifications
		: public AZ::EBusTraits
	{
	public:
		// Multiple handlers. Events received in undefined order.
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		// The EBus uses a single address.
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;


		virtual void OnLanguageChanged(const AZStd::string &language) = 0;
	};
	using LocalizationNotificationBus = AZ::EBus<LocalizationNotifications>;

	AZStd::string GetLocalizatedString(const rapidjson::Value &value);
}
