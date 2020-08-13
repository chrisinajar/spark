#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/std/containers/unordered_map.h>

#include "Busses/SettingsBus.h"

namespace spark
{
	class SettingsSystemComponent
		: public AZ::Component
		, public SettingsRequestBus::Handler
	{
	public:
		AZ_COMPONENT(SettingsSystemComponent, "{9D6D5D03-1228-4CFA-8498-C6D185061E69}")


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
		// SettingsRequestBus::Handler implementation
		void SetSettingValue(AZStd::string key,AZStd::string value, AZStd::string settingGroup);
		AZStd::string GetSettingValue(AZStd::string key, AZStd::string settingGroup);

		AZStd::string m_group;
		void SetDefaultSettings(AZStd::string settingGroup);


		void ChangeVideoMode(AZ::Vector2 resolution, bool fullscreen);
		
		AZ::Vector2 GetScreenResolution();
		bool IsFullscreen();

	private:
		AZStd::unordered_map<AZStd::string, AZStd::string> m_settings;

		AZ::Vector2 m_resolution = AZ::Vector2{ 800,600 };
		bool m_fullscreen = false;
	};

}
