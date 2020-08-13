#pragma once

#include <AzCore/EBus/EBus.h>

namespace RVO2
{
    class RVO2Requests
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////

        // Put your public methods here
    };
    using RVO2RequestBus = AZ::EBus<RVO2Requests>;
} // namespace RVO2
