#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>

#include "Busses/LocalizationBus.h"

namespace spark
{
	class LocalizationSystemComponent
		: public AZ::Component
		, public LocalizationRequestBus::Handler
		, public AZ::TickBus::Handler
	{
	public:
		AZ_COMPONENT(LocalizationSystemComponent, "{42E853E0-03C6-4FC5-85A4-A292D668F17F}")


		static void Reflect(AZ::ReflectContext* context);

		static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
		static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
		static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
		static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

		////////////////////////////////////////////////////////////////////////
		// AZ::Component interface implementation
		void Init() override;
		void Activate() override;
		void Deactivate() override;
		////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////
		AZStd::string LocalizeString(AZStd::string id);

		AZStd::string GetLanguage();
		void SetLanguage(AZStd::string language);
		AZStd::vector<AZStd::string> GetAvailableLanguages();

	private:
		void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
	};

}
