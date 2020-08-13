#pragma once


#include "Busses/AudioBus.h"
#include "Utils/CircularVector.h"

namespace spark
{

	class AudioSystemComponent
		: public AZ::Component
		, public AudioRequestBus::Handler
	{
	public:
		AZ_COMPONENT(AudioSystemComponent, "{616A5DF0-68FC-4FDD-BA79-B223F8C61D2F}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

    protected:

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////

		AZ::EntityId PlaySound(AZStd::string);

	private:
		CircularVector<AZ::EntityId> m_entities;
		
	};


}