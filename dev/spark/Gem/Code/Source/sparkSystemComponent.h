
#pragma once

#include <AzCore/Component/Component.h>
#include <AzFramework/Entity/GameEntityContextBus.h>

#include <spark/sparkBus.h>

namespace spark
{
    class sparkSystemComponent
        : public AZ::Component
        , protected sparkRequestBus::Handler
		, protected AzFramework::GameEntityContextEventBus::Handler
    {
    public:
        AZ_COMPONENT(sparkSystemComponent, "{E9DC6A6B-2808-41CF-B59E-FA27B7D4662F}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:
        ////////////////////////////////////////////////////////////////////////
        // sparkRequestBus interface implementation

        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////

		void OnGameEntitiesStarted() override;
    };
}
