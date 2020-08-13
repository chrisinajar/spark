#include "spark_precompiled.h"


#include "PlayerIdentitySystemComponent.h"

using namespace spark;

void PlayerIdentitySystemComponent::Reflect (AZ::ReflectContext* reflection)
{
	if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
	{
		serializationContext->Class<PlayerIdentitySystemComponent, AZ::Component>()
			->Version(1)
			;

		if (auto editContext = serializationContext->GetEditContext())
		{
			editContext->Class<PlayerIdentitySystemComponent>("PlayerIdentitySystemComponent", "System component for authenticating identity")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
				->Attribute(AZ::Edit::Attributes::Category, "spark")
				->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"))
				;
		}
	}
}

void PlayerIdentitySystemComponent::Init ()
{

}
void PlayerIdentitySystemComponent::Activate ()
{
#if !defined(DEDICATED_SERVER)
	AZ::u32 retVal = -1;
	// EBUS_EVENT_RESULT(retVal, CloudGemPlayerAccount::CloudGemPlayerAccountRequestBus, InitiateAuth, "chris", "");
	EBUS_EVENT(CloudGemPlayerAccount::CloudGemPlayerAccountRequestBus, GetCurrentUser);

	if (retVal > 0)
	{
		AZ_Printf(0, "Failed to authenticate with cloud gem or player or cogn... i dunno man....");
	}
#endif
}

void PlayerIdentitySystemComponent::Deactivate ()
{

}

