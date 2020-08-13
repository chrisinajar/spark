
#pragma once

#include <AzCore/EBus/EBus.h>

namespace AZ
{
    class Vector2;
}
namespace MouseCursorPosition
{
    class MouseCursorPositionRequests
        : public AZ::EBusTraits
    {

    public:
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        // Public functions

        /**
        * Returns the system cursor position normalized between [0,0] top left and [1,1] bottom right
        */
        virtual AZ::Vector2 GetSystemCursorPositionNormalized() = 0;
		
        virtual AZ::Vector2 GetSystemCursorPositionPixelSpace() = 0;
    };
    using MouseCursorPositionRequestBus = AZ::EBus<MouseCursorPositionRequests>;
} // namespace MouseCursorPosition
