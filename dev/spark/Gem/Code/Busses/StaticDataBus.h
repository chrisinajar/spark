#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/ComponentBus.h>

#include "Utils/CommonTypes.h"
#include <AzCore/JSON/document.h>

namespace spark
{
	
	class StaticDataRequests
		: public AZ::ComponentBus
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;
		

		virtual AZStd::string GetJsonString() = 0;
		virtual AZStd::string GetValue(AZStd::string) = 0;
		virtual AZStd::string GetSpecialValue(AZStd::string) = 0;
		virtual AZStd::string GetSpecialValueLevel(AZStd::string,int level) = 0;

		//for c++ use only
		virtual const rapidjson::Value* GetJson() const = 0;
	};
	using StaticDataRequestBus = AZ::EBus<StaticDataRequests>;

}
