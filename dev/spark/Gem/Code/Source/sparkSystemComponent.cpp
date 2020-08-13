
#include "spark_precompiled.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

#include "sparkSystemComponent.h"

#include "Busses/ProjectileManagerBus.h"
#include "Utils/Log.h"

namespace spark
{
    void sparkSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<sparkSystemComponent, AZ::Component>()
                ->Version(0)
                ;

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<sparkSystemComponent>("spark", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }
    }

    void sparkSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC("sparkService"));
    }

    void sparkSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC("sparkService"));
    }

    void sparkSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        (void)required;
    }

    void sparkSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        (void)dependent;
    }

    void sparkSystemComponent::Init()
    {
    }

    void sparkSystemComponent::Activate()
    {
		AzFramework::GameEntityContextEventBus::Handler::BusConnect();
        sparkRequestBus::Handler::BusConnect();
    }

    void sparkSystemComponent::Deactivate()
    {
        sparkRequestBus::Handler::BusDisconnect();
		AzFramework::GameEntityContextEventBus::Handler::BusDisconnect();
    }

	void sparkSystemComponent::OnGameEntitiesStarted() {
		sLOG("sparkSystemComponent OnGameEntitiesStarted");
		EBUS_EVENT(ProjectileManagerRequestBus, ResetProjectileManager);
	}
}
