#pragma once

#include <AzCore/Component/Component.h>
#include <AzFramework/Input/Events/InputChannelEventListener.h>
#include <AzCore/std/containers/unordered_map.h>

#include <AzCore/Component/TickBus.h>

#include "Busses/InputMapperBus.h"
#include "Busses/CanvasInputBus.h"
#include "Busses/SettingsBus.h"
#include "Busses/GameManagerBus.h"

namespace spark
{
	class InputMapperSystemComponent
		: public AZ::Component
		, public AzFramework::InputChannelEventListener
		, public InputMapperRequestBus::Handler
		, protected CanvasInputRequestBus::Handler
		, protected SettingsNotificationBus::Handler
		, protected GameManagerNotificationBus::Handler
		, protected AZ::TickBus::Handler
	{
	public:
		AZ_COMPONENT(InputMapperSystemComponent, "{DDEF4CB9-C0FD-49DB-A143-CBFB814AEBE1}")

		InputMapperSystemComponent() :AzFramework::InputChannelEventListener(1) {} //set the priority just above the CameraControllerComponent(=0)

		~InputMapperSystemComponent() override {};

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
		// InputMapperBus::Handler implementation
		Slot InputToSlot(const AzFramework::InputChannel& inputChannel);
		void BindInputToSlot(AzFramework::InputChannelId input, Slot slot);

		void BindHotKeyToSlot(spark::Hotkey hotkey, Slot slot);
		Hotkey GetSlotHotkey(Slot slot);

		void StartHotkeyRegistration();

		int GetMouseButton() { return m_currentMouseButton; }
		int GetMouseEventType() { return m_currentMouseEventType; }
		void ConsumeEvent() { m_eventConsumed = true; }
	protected:
		void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

		bool OnInputChannelEventFiltered(const AzFramework::InputChannel& inputChannel) override;

		AZStd::unordered_map<spark::Hotkey, Slot> m_inputMap;
		AZStd::unordered_map<Slot, spark::Hotkey> m_reverseInputMap;

		spark::Hotkey m_currentHotkey;

		enum
		{
			S_IDLE,
			S_REGISTERING
		}m_state = S_IDLE;


		bool m_saveToSetting = false;
		void LoadKeyBindings();
		void SaveKeyBindings();
		void OnSetDefaultSettings();
		void OnGameManagerActivated(AZ::Entity* gameManager);


		bool HandleCanvasEvent(const AzFramework::InputChannel& inputChannel);
		int m_currentMouseButton = MouseButton::Left;
		int m_currentMouseEventType = MouseEventType::onclick;
		bool m_simulatingMouseButton = false;
		bool m_eventConsumed;
	};

}