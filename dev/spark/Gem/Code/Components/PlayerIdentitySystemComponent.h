#pragma once

#include <AzCore/Component/Component.h>
#include <AzFramework/Entity/GameEntityContextBus.h>

#include <CloudGemPlayerAccount/CloudGemPlayerAccountBus.h>

namespace spark
{
	class PlayerIdentitySystemComponent
		: public AZ::Component
		// , public CloudGemPlayerAccount::CloudGemPlayerAccountNotificationBus::Handler
	{
	public:
		AZ_COMPONENT(PlayerIdentitySystemComponent, "{78DF2035-9AC9-41BD-ABDD-45B02BE6F9DF}")

		static void Reflect(AZ::ReflectContext* reflection);

		void Init () override;
		void Activate () override;
		void Deactivate () override;

	};
}
