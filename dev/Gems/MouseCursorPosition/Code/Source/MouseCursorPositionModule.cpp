
#include "StdAfx.h"
#include <platform_impl.h>

#include <AzCore/Memory/SystemAllocator.h>

#include "MouseCursorPositionSystemComponent.h"

#include <IGem.h>

namespace MouseCursorPosition
{
    class MouseCursorPositionModule
        : public CryHooksModule
    {
    public:
        AZ_RTTI(MouseCursorPositionModule, "{CEF083F9-A26A-4200-9464-67131766321B}", CryHooksModule);
        AZ_CLASS_ALLOCATOR(MouseCursorPositionModule, AZ::SystemAllocator, 0);

        MouseCursorPositionModule()
            : CryHooksModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            m_descriptors.insert(m_descriptors.end(), {
                MouseCursorPositionSystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<MouseCursorPositionSystemComponent>(),
            };
        }
    };
}

// DO NOT MODIFY THIS LINE UNLESS YOU RENAME THE GEM
// The first parameter should be GemName_GemIdLower
// The second should be the fully qualified name of the class above
AZ_DECLARE_MODULE_CLASS(MouseCursorPosition_7c18089fe13b4ef9aa7c36beacaecc87, MouseCursorPosition::MouseCursorPositionModule)
