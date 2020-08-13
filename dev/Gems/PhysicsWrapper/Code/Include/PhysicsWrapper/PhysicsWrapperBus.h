
#pragma once

#include <AzCore/EBus/EBus.h>

namespace PhysicsWrapper
{
	struct Hit
	{
		AZ::EntityId entity;
		bool hit = false;
		AZ::Vector3 position;
		AZ::Vector3 normal;
	};

    class PhysicsWrapperRequests
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
		virtual Hit PerformRayCastWithParam(float distance, int collisionFlags) = 0;
		virtual Hit PerformRayCast() = 0;

		virtual Hit PerformSingleResultRayCast(AZ::Vector3 start, AZ::Vector3 end)=0;
        // Put your public methods here
    };
    using PhysicsWrapperRequestBus = AZ::EBus<PhysicsWrapperRequests>;
} // namespace PhysicsWrapper
