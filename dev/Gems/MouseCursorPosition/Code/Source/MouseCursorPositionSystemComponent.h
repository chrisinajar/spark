
#pragma once

#include <AzCore/Component/Component.h>

#include <MouseCursorPosition/MouseCursorPositionBus.h>

namespace MouseCursorPosition
{
    class MouseCursorPositionSystemComponent
        : public AZ::Component
        , protected MouseCursorPositionRequestBus::Handler
    {
    public:
        AZ_COMPONENT(MouseCursorPositionSystemComponent, "{924337BB-559B-4173-AEC2-6250D6CA04F8}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:
        ////////////////////////////////////////////////////////////////////////
        // MouseCursorPositionRequestBus interface implementation
        AZ::Vector2 GetSystemCursorPositionNormalized() override;
		AZ::Vector2 GetSystemCursorPositionPixelSpace() override;
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////
    };
}
