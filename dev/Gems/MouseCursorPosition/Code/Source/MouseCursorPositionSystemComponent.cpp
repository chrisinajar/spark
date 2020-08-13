
#include "StdAfx.h"

 
#include <IRenderer.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

#include "MouseCursorPositionSystemComponent.h"
#include <AzCore/Math/Vector2.h>
#include <AzFramework/Input/Devices/Mouse/InputDeviceMouse.h>
#include <AzCore/RTTI/BehaviorContext.h>


namespace MouseCursorPosition
{
    void MouseCursorPositionSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
			serialize->Class<MouseCursorPositionSystemComponent, AZ::Component>()
				->Version(0);

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<MouseCursorPositionSystemComponent>("MouseCursorPosition", "Exposes getting the mouse cursor position via EBus")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }

        // Expose an EBus to request a normalized cursor position using behavior context
        if (AZ::BehaviorContext* bc = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            bc->EBus<MouseCursorPositionRequestBus>("CursorPositionRequestBus")
                ->Event("GetSystemCursorPositionNormalized", &MouseCursorPositionRequestBus::Events::GetSystemCursorPositionNormalized)
                ;
        }
    }

    void MouseCursorPositionSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC("MouseCursorPositionService"));
    }

    void MouseCursorPositionSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC("MouseCursorPositionService"));
    }

    void MouseCursorPositionSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        (void)required;
    }

    void MouseCursorPositionSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        (void)dependent;
    }

    void MouseCursorPositionSystemComponent::Init()
    {
    }

    AZ::Vector2 MouseCursorPositionSystemComponent::GetSystemCursorPositionNormalized()
    {
        AZ::Vector2 result;

        AzFramework::InputSystemCursorRequestBus::EventResult(result, AzFramework::InputDeviceMouse::Id,
            &AzFramework::InputSystemCursorRequests::GetSystemCursorPositionNormalized);

        return result;
    }

	AZ::Vector2 MouseCursorPositionSystemComponent::GetSystemCursorPositionPixelSpace()
	{
		int width = GetISystem()->GetIRenderer()->GetWidth(), height = GetISystem()->GetIRenderer()->GetHeight();
		AZ::Vector2 screen_size((float)width, (float)height);

		return GetSystemCursorPositionNormalized() * screen_size;
	}

    void MouseCursorPositionSystemComponent::Activate()
    {
        MouseCursorPositionRequestBus::Handler::BusConnect();
    }

    void MouseCursorPositionSystemComponent::Deactivate()
    {
        MouseCursorPositionRequestBus::Handler::BusDisconnect();
    }
}
