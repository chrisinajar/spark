
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>

#include <RVO2SystemComponent.h>

namespace RVO2
{
    class RVO2Module
        : public AZ::Module
    {
    public:
        AZ_RTTI(RVO2Module, "{BBF18580-F1E4-415F-8BB4-D85614358395}", AZ::Module);
        AZ_CLASS_ALLOCATOR(RVO2Module, AZ::SystemAllocator, 0);

        RVO2Module()
            : AZ::Module()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            m_descriptors.insert(m_descriptors.end(), {
                RVO2SystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<RVO2SystemComponent>(),
            };
        }
    };
}

// DO NOT MODIFY THIS LINE UNLESS YOU RENAME THE GEM
// The first parameter should be GemName_GemIdLower
// The second should be the fully qualified name of the class above
AZ_DECLARE_MODULE_CLASS(RVO2_797d43d3996744cab164bc494d757a92, RVO2::RVO2Module)
