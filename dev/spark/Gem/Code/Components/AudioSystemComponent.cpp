
#include "spark_precompiled.h"

#include "AudioSystemComponent.h"

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

#include <AzFramework/Entity/GameEntityContextBus.h>
#include <LmbrCentral/Audio/AudioTriggerComponentBus.h>
#include <AzFramework/Components/TransformComponent.h>

#include "Utils/Log.h"

namespace spark
{

	void AudioSystemComponent::Reflect(AZ::ReflectContext* context)
	{
		if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<AudioSystemComponent, AZ::Component>()
                ->Version(0)
                ;

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<AudioSystemComponent>("AudioSystemComponent", "spark audio layer")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }

		if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
		{
			behaviorContext->EBus<AudioRequestBus>("AudioRequestBus")
				->Event("PlaySound", &AudioRequestBus::Events::PlaySound);
		}
	}

	void AudioSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
	{
		provided.push_back(AZ_CRC("AudioSystemService"));
	}

	void AudioSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
	{
		incompatible.push_back(AZ_CRC("AudioSystemService"));
	}

	void AudioSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
	{
		(void)required;
	}

	void AudioSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
	{
		(void)dependent;
	}

	void AudioSystemComponent::Init()
	{
	}

	void AudioSystemComponent::Activate()
	{
		/*int i = 2;
		while (i--)
		{
			AZ::Entity *entity =  aznew AZ::Entity(nullptr);
			if (entity)
			{
				AZ::Uuid audioTriggerComponentUuid("{8CBBB54B-7435-4D33-844D-E7F201BD581A}");
				AZ::Uuid audioProxyComponentUuid("{0EE6EE0F-7939-4AB8-B0E3-F9B3925D61EE}");

				entity->CreateComponent(audioTriggerComponentUuid);
				entity->CreateComponent(audioProxyComponentUuid);

				entity->Init();
				entity->Activate();

				m_entities.push_back(entity->GetId());
			}
		}*/

		AudioRequestBus::Handler::BusConnect();
	}

	void AudioSystemComponent::Deactivate()
	{
		AudioRequestBus::Handler::BusDisconnect();
	}

	AZ::EntityId AudioSystemComponent::PlaySound(AZStd::string soundId)
	{
		sLOG("AudioSystemComponent::PlaySound " + soundId);

		if (m_entities.empty())
		{
			int i = 30;
			sLOG("adding " + i + " entities for playing sounds");
			while (i--)
			{
				AZ::Entity *entity = aznew AZ::Entity(nullptr);
				if (entity)
				{
					AZ::Uuid audioTriggerComponentUuid("{8CBBB54B-7435-4D33-844D-E7F201BD581A}");
					AZ::Uuid audioProxyComponentUuid("{0EE6EE0F-7939-4AB8-B0E3-F9B3925D61EE}");
					
					entity->CreateComponent(AZ::TransformComponentTypeId);
					entity->CreateComponent(audioProxyComponentUuid);
					entity->CreateComponent(audioTriggerComponentUuid);
					

					entity->Init();
					entity->Activate();

					m_entities.push_back(entity->GetId());
				}
			}
		}

		AZ::EntityId entityId = m_entities.GetNext();

		if (entityId.IsValid())
		{
			sLOG("AudioSystemComponent::PlaySound sending audio to " + entityId);
			EBUS_EVENT_ID(entityId, LmbrCentral::AudioTriggerComponentRequestBus, ExecuteTrigger, soundId.c_str());
		}

		return entityId;
	}


}
