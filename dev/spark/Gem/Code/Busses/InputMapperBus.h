#pragma once


#include <AzCore/Component/ComponentBus.h>
#include "Utils/Slot.h"

#include <AzCore/std/containers/vector.h>

namespace spark 
{
	//todo write in a new file in utils
	class Hotkey
	{
		AZStd::vector<AzFramework::InputChannelId> m_keys;
	public:
		AZ_TYPE_INFO(Hotkey, "{3E9C60C4-72EB-4359-87B8-2A9633C81214}");

		Hotkey() {}
		Hotkey(AZStd::string str);
		static void Reflect(AZ::ReflectContext* reflection);

		void AddKey(AzFramework::InputChannelId key);
		void RemoveKey(AzFramework::InputChannelId key);
		bool HasKey(AzFramework::InputChannelId key) const;
		bool Contains(Hotkey other) const;

		AZStd::vector<AzFramework::InputChannelId> GetKeys() const;
		bool operator==(const Hotkey &other) const;

		operator bool() const { return !m_keys.empty(); }
		bool IsValid()  const { return !m_keys.empty(); }

		AZStd::string ToString() const;
	};


	class InputMapperRequests
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;


		virtual Slot InputToSlot(const AzFramework::InputChannel& inputChannel) = 0;
		virtual void BindInputToSlot(AzFramework::InputChannelId input, Slot slot) = 0;

		virtual void BindHotKeyToSlot(Hotkey hotkey, Slot slot) = 0;

		virtual Hotkey GetSlotHotkey(Slot slot) = 0;
		///@todo unbinding

		virtual void StartHotkeyRegistration() = 0;
	};
	using InputMapperRequestBus = AZ::EBus<InputMapperRequests>;


	class InputMapperNotifications
		: public AZ::EBusTraits
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		// EBusTraits overrides
		static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
		static const AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;

		virtual void OnHotkeyRegistrationStart() {}
		virtual void OnHotkeyRegistrationChanged(const Hotkey &hotkey) {}
		virtual void OnHotkeyRegistrationDone(const Hotkey &hotkey) {}
	};
	using InputMapperNotificationBus = AZ::EBus<InputMapperNotifications>;
}
