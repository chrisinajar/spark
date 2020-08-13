#pragma once


#include <AzCore/Component/ComponentBus.h>
#include "Utils/CommonTypes.h"
#include <AzCore/JSON/rapidjson.h>

namespace spark {

	class InfoRequests
		: public AZ::EBusTraits
	{
	public:
		// One handler is supported.
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		// The EBus uses a single address.
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;


		virtual AZStd::string GetGameModeJsonString() = 0;
		virtual AZStd::string GetGameModeName() = 0;

		virtual AZStd::string GetAbilityJsonString(AbilityTypeId) = 0;
		virtual AZStd::string GetItemJsonString(ItemTypeId) = 0;
		virtual AZStd::string GetUnitJsonString(UnitTypeId) = 0;

		//C++ only
		virtual rapidjson::Value* GetGameModeJson() = 0;
		virtual rapidjson::Value* GetAbilityJson(AbilityTypeId) = 0;
		virtual rapidjson::Value* GetItemJson(ItemTypeId) = 0;
		virtual rapidjson::Value* GetUnitJson(UnitTypeId) = 0;

		virtual AZStd::vector<AZStd::string> GetHeroList() const = 0;
	};
	using InfoRequestBus = AZ::EBus<InfoRequests>;

}
