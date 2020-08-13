#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/ComponentBus.h>
#include <AzCore/std/string/string.h>


namespace spark {


	class AudioRequests
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;

		virtual AZ::EntityId PlaySound(AZStd::string) = 0;
	};
	using AudioRequestBus = AZ::EBus<AudioRequests>;

}
