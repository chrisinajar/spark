#pragma once

#include <AzCore/Component/Component.h>

#include <RVO2/RVO2Bus.h>

namespace RVO2
{
    class RVO2SystemComponent
        : public AZ::Component
        , protected RVO2RequestBus::Handler
    {
    public:
        AZ_COMPONENT(RVO2SystemComponent, "{A4850B68-4F2A-44E6-8632-1C6DFEC4C93A}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:
        ////////////////////////////////////////////////////////////////////////
        // RVO2RequestBus interface implementation

        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////
    };
}
