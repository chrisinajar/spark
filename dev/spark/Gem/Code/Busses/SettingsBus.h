#pragma once


#include <AzCore/Component/ComponentBus.h>
#include "Utils/Slot.h"

#include <AzCore/std/containers/vector.h>

#include <AzCore/Math/Vector2.h>

namespace spark 
{

	class SettingsRequests
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;


		virtual void SetSettingValue(AZStd::string key,AZStd::string value, AZStd::string settingGroup) = 0;
		virtual AZStd::string GetSettingValue(AZStd::string key, AZStd::string settingGroup) = 0;

		virtual void SetDefaultSettings(AZStd::string settingGroup) = 0;


		virtual void ChangeVideoMode(AZ::Vector2 resolution, bool fullscreen) = 0;

		virtual AZ::Vector2 GetScreenResolution() = 0;
		virtual bool IsFullscreen() = 0;
	};
	using SettingsRequestBus = AZ::EBus<SettingsRequests>;


	class SettingsNotifications
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;

		virtual void OnSettingChanged(AZStd::string key, AZStd::string value) {}
		virtual void OnSetDefaultSettings() {}
	};
	using SettingsNotificationBus = AZ::EBus<SettingsNotifications>;
}
