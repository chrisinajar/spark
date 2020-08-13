

#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>

#include <SteamWorksSystemComponent.h>

namespace SteamWorks
{
    class SteamWorksModule
        : public AZ::Module
    {
    public:
        AZ_RTTI(SteamWorksModule, "{B101FB10-703D-4372-8177-1B53F582CFD5}", AZ::Module);
        AZ_CLASS_ALLOCATOR(SteamWorksModule, AZ::SystemAllocator, 0);

        SteamWorksModule()
            : AZ::Module()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            m_descriptors.insert(m_descriptors.end(), {
                SteamWorksSystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<SteamWorksSystemComponent>(),
            };
        }
    };
}

// DO NOT MODIFY THIS LINE UNLESS YOU RENAME THE GEM
// The first parameter should be GemName_GemIdLower
// The second should be the fully qualified name of the class above
AZ_DECLARE_MODULE_CLASS(SteamWorks_d2793a8baa464ba1af9094bed6b761c0, SteamWorks::SteamWorksModule)
