#pragma once

#include <AzCore/Component/ComponentBus.h>
#include "Utils/DynamicSliceWrapper.h"

namespace spark {

	using DynamicSliceTypeId = AZStd::string;

	class DynamicSliceManagerRequests
		: public AZ::EBusTraits
	{
	public:
		// One handler is supported.
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
		// The EBus uses a single address.
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;

		virtual DynamicSliceAsset GetDynamicSliceAsset(DynamicSliceTypeId id) = 0;
		virtual DynamicSliceAssetId GetDynamicSliceAssetId(DynamicSliceTypeId id) = 0;

		virtual AzFramework::SliceInstantiationTicket SpawnDynamicSlice(DynamicSliceTypeId id) = 0;
	};
	using DynamicSliceManagerRequestBus = AZ::EBus<DynamicSliceManagerRequests>;

}
