
#pragma once

#include <AzCore/Component/Component.h>

#include <PhysicsWrapper/PhysicsWrapperBus.h>

namespace PhysicsWrapper
{
    class PhysicsWrapperSystemComponent
        : public AZ::Component
        , protected PhysicsWrapperRequestBus::Handler
    {
    public:
        AZ_COMPONENT(PhysicsWrapperSystemComponent, "{99FF65C0-01A6-4DC5-9748-2D448E4E64BA}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:
        ////////////////////////////////////////////////////////////////////////
        // PhysicsWrapperRequestBus interface implementation
		Hit PerformRayCastWithParam(float distance, int collisionFlags);
		Hit PerformRayCast();
		Hit PerformSingleResultRayCast(AZ::Vector3 start, AZ::Vector3 end);
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////
    };
}
