
#include "PhysicsWrapper_precompiled.h"
#include <platform_impl.h>

#include <AzCore/Memory/SystemAllocator.h>

#include "PhysicsWrapperSystemComponent.h"

#include <IGem.h>

namespace PhysicsWrapper
{
    class PhysicsWrapperModule
        : public CryHooksModule
    {
    public:
        AZ_RTTI(PhysicsWrapperModule, "{DC02DDB8-C3E9-434F-A7B4-87D37D10727F}", CryHooksModule);
        AZ_CLASS_ALLOCATOR(PhysicsWrapperModule, AZ::SystemAllocator, 0);

        PhysicsWrapperModule()
            : CryHooksModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            m_descriptors.insert(m_descriptors.end(), {
                PhysicsWrapperSystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<PhysicsWrapperSystemComponent>(),
            };
        }
    };
}

// DO NOT MODIFY THIS LINE UNLESS YOU RENAME THE GEM
// The first parameter should be GemName_GemIdLower
// The second should be the fully qualified name of the class above
AZ_DECLARE_MODULE_CLASS(PhysicsWrapper_0e4cab78ff7a4b58bb8557eb0bc23fe5, PhysicsWrapper::PhysicsWrapperModule)
