

#include "spark_precompiled.h"

#include "InputMapperSystemComponent.h"

#include <AzCore/Serialization/EditContext.h>
#include <AzFramework/Input/Channels/InputChannel.h>
#include <AzFramework/Input/Devices/Keyboard/InputDeviceKeyboard.h>
#include <AzFramework/Input/Devices/Mouse/InputDeviceMouse.h>
#include <AzCore/std/sort.h>

#include "Busses/SlotBus.h"

#include <MouseCursorPosition/MouseCursorPositionBus.h>

#include "Busses/SettingsBus.h"
#include "Utils/Log.h"

#include <IRenderer.h>
//#include <LyShine/Bus/UiTransformBus.h>

using namespace spark;
using namespace AzFramework;

//Hotkey Implementation

class KeysLibrary
{
	KeysLibrary()
	{
		for (char c = 'A'; c <= 'Z'; ++c)
			Add(AZStd::string::format("%c",c).c_str(), AzFramework::InputChannelId(AZStd::string::format("keyboard_key_alphanumeric_%c",c).c_str()));

		for (char c = '0'; c <= '9'; ++c)
			Add(AZStd::string::format("%c",c).c_str(), AzFramework::InputChannelId(AZStd::string::format("keyboard_key_alphanumeric_%c",c).c_str()));

		for (char c = 1; c <= 12; ++c)
			Add(AZStd::string::format("F%d",c).c_str(), AzFramework::InputChannelId(AZStd::string::format("keyboard_key_function_F%02d",c).c_str()));

		Add("Alt",InputDeviceKeyboard::Key::ModifierAltL);
		Add("Ctrl",InputDeviceKeyboard::Key::ModifierCtrlL);
		Add("Shift",InputDeviceKeyboard::Key::ModifierShiftL);


		Add("Backspace", InputDeviceKeyboard::Key::EditBackspace);
		Add("Enter", InputDeviceKeyboard::Key::EditEnter);
		Add("Space", InputDeviceKeyboard::Key::EditSpace);
	}
public:
	static KeysLibrary& GetIstance()
	{
		static KeysLibrary l;
		return l;
	}

	void Add(AZStd::string str, AzFramework::InputChannelId channelId)
	{
		m_inputMap[channelId] = str;
		AZStd::to_upper(str.begin(), str.end());
		m_reverseInputMap[str] = channelId;
	}

	AZStd::string Get(AzFramework::InputChannelId channelId)
	{
		auto it = m_inputMap.find(channelId);
		if (it != m_inputMap.end())return it->second;
		return "";// channelId.GetName(); if it is not registered, it is not usable -> return invalid
	}
	AzFramework::InputChannelId Get(AZStd::string str)
	{
		AZStd::to_upper(str.begin(), str.end());
		auto it = m_reverseInputMap.find(str);
		if (it != m_reverseInputMap.end())return it->second;
		return AzFramework::InputChannelId();//  if it is not registered -> return invalid
	}
private:
	AZStd::unordered_map<AzFramework::InputChannelId, AZStd::string> m_inputMap;
	AZStd::unordered_map<AZStd::string, AzFramework::InputChannelId> m_reverseInputMap;
};


AZStd::string KeyToString(AzFramework::InputChannelId channelId)
{
	return KeysLibrary::GetIstance().Get(channelId);
}

AzFramework::InputChannelId StringToKey(AZStd::string str)
{
	return KeysLibrary::GetIstance().Get(str);
}

void HotkeyScriptConstructor(Hotkey* self, AZ::ScriptDataContext& dc)
{
	if (dc.GetNumArguments() == 0)
	{
		*self = Hotkey();
		return;
	}
	else if (dc.GetNumArguments() == 1 && dc.IsString(0))
	{
		new(self) Hotkey();

		const char* str = nullptr;
		dc.ReadArg(0, str);

		if (!str)return;

		new(self) Hotkey(str);

		return;
	}

	dc.GetScriptContext()->Error(AZ::ScriptContext::ErrorType::Error, true, "Invalid arguments passed to Hotkey().");
	new(self) Hotkey();
}

void Hotkey::Reflect(AZ::ReflectContext* reflection)
{
	if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
	{
		serializationContext->Class<Hotkey>()
			//->Version(1)
			->Field("keys", &Hotkey::m_keys);

		if (auto editContext = serializationContext->GetEditContext())
		{
			editContext->Class<Hotkey>("Hotkey", "")
				->DataElement(nullptr, &Hotkey::m_keys, "keys", "");
		}
	}

	if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
	{
		behaviorContext->Method("KeyToString", &KeyToString);
		behaviorContext->Method("StringToKey", &StringToKey);

		behaviorContext->Class<Hotkey>("Hotkey")
			->Attribute(AZ::Script::Attributes::Storage, AZ::Script::Attributes::StorageType::Value)
			->Attribute(AZ::Script::Attributes::ConstructorOverride, &HotkeyScriptConstructor)
			->Method("HasKey",&Hotkey::HasKey)
			->Method("AddKey",&Hotkey::AddKey)
			->Method("RemoveKey",&Hotkey::RemoveKey)
			->Method("Contains", &Hotkey::Contains)
			->Method("ToString",&Hotkey::ToString)
			->Method("GetKeys",&Hotkey::GetKeys);
	}
}

Hotkey::Hotkey(AZStd::string s)
{
	const char* str = s.c_str();
	AZStd::string keyStr;
	char c;
	do
	{
		c = *str;
		if (c == '+' || c == ' ' || c == ',' || c == '-' || c==0) {
			if (!keyStr.empty())
			{
				auto key=StringToKey(keyStr);
				if (key.GetNameCrc32())//if key is valid
				{
					AddKey(key);
				}
				keyStr.clear();
			}
		}else keyStr += c;
		++str;
	} while (c != 0);
}

void Hotkey::AddKey(AzFramework::InputChannelId key)
{
	if (HasKey(key))return;

	m_keys.push_back(key);

	AZStd::sort(m_keys.begin(),m_keys.end(),
		[](const AzFramework::InputChannelId &a, const AzFramework::InputChannelId &b)->bool
		{
		return (AZ::u32)a.GetNameCrc32() < (AZ::u32)b.GetNameCrc32();
		}
	);
}
void Hotkey::RemoveKey(AzFramework::InputChannelId key)
{
	//AZStd::remove(m_keys.begin(), m_keys.end(), key);
	AZStd::vector<AzFramework::InputChannelId> new_keys;

	for (auto k:m_keys)
	{
		if (k != key)new_keys.push_back(k);
	}
	m_keys = new_keys;
}
bool Hotkey::HasKey(AzFramework::InputChannelId key) const
{
	for (auto k:m_keys)
	{
		if (k == key)return true;
	}
	return false;
}
bool Hotkey::Contains( Hotkey other) const
{
	if (other.m_keys.size() > m_keys.size())return false;
	for (auto k : other.m_keys)
	{
		if (!HasKey(k))return false;
	}
	return true;
}

AZStd::vector<AzFramework::InputChannelId> Hotkey::GetKeys() const
{
	return m_keys;
}

bool Hotkey::operator==(const Hotkey &other) const
{
	if (other.m_keys.size() != m_keys.size())return false;
	for (int i = 0; i < m_keys.size(); ++i)
	{
		if (m_keys[i] != other.m_keys[i])return false;
	}
	return true;
}
AZStd::string Hotkey::ToString() const
{
	AZStd::string s = "Hotkey(";
	for (int i = 0; i < m_keys.size(); ++i)
	{
		s += KeyToString(m_keys[i]);
		if (i != m_keys.size() - 1)s += "+";
	}
	s += ")";
	return s;
}


class InputMapperNotificationBusHandler
	: public InputMapperNotificationBus::Handler
	, public AZ::BehaviorEBusHandler
{
public:
	AZ_EBUS_BEHAVIOR_BINDER(InputMapperNotificationBusHandler, "{A64EF7B2-AA4E-4B7B-AF67-ABD3B5EDD067}", AZ::SystemAllocator, OnHotkeyRegistrationStart,OnHotkeyRegistrationChanged,OnHotkeyRegistrationDone);

	void OnHotkeyRegistrationStart() {
		Call(FN_OnHotkeyRegistrationStart);
	}
	void OnHotkeyRegistrationChanged(const Hotkey &hotkey) {
		Call(FN_OnHotkeyRegistrationChanged,hotkey);
	}
	void OnHotkeyRegistrationDone(const Hotkey &hotkey) {
		Call(FN_OnHotkeyRegistrationDone,hotkey);
	}
};

class MouseInputNotificationBusHandler
	: public MouseInputNotificationBus::Handler
	, public AZ::BehaviorEBusHandler
{
public:
	AZ_EBUS_BEHAVIOR_BINDER(MouseInputNotificationBusHandler, "{085511D3-969B-4C06-8D19-A029AD2B82DE}", AZ::SystemAllocator, onmousedown, onmouseup, onmousemove);

	bool onmousedown(int button) {
		bool result = false;
		CallResult(result,FN_onmousedown,button);
		return result;
	}
	bool onmouseup(int button) {
		bool result = false;
		Call(FN_onmouseup, button);
		return result;
	}
	void onmousemove(int x, int y) {
		Call(FN_onmousemove, x,y);
	}
};

class EmptyClass
{
public:
	AZ_TYPE_INFO(EmptyClass, "{5B8A4FBB-76AA-4E19-8C3C-507061026819}");
};

class EmptyClass1
{
public:
	AZ_TYPE_INFO(EmptyClass1, "{C98BA84A-5EB8-439D-A9DC-59C7F6CB8C44}");
};


//InputMapperSystemComponent implementation

void InputMapperSystemComponent::Reflect (AZ::ReflectContext* reflection)
{
    if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
    {
        serializationContext->Class<InputMapperSystemComponent, AZ::Component>()
            ->Version(1);


        if (auto editContext = serializationContext->GetEditContext())
        {
			editContext->Class<InputMapperSystemComponent>("InputMapperSystemComponent", "Maps keyboard events to game input")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
				->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
				->Attribute(AZ::Edit::Attributes::AutoExpand, true);
        }
    }

	if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
	{
		Hotkey::Reflect(reflection);

		behaviorContext->EBus<InputMapperRequestBus>("InputMapperRequestBus")
			->Event("InputToSlot", &InputMapperRequestBus::Events::InputToSlot)
			->Event("BindInputToSlot", &InputMapperRequestBus::Events::BindInputToSlot)
			->Event("BindHotKeyToSlot", &InputMapperRequestBus::Events::BindHotKeyToSlot)
			->Event("GetSlotHotkey", &InputMapperRequestBus::Events::GetSlotHotkey)
			->Event("StartHotkeyRegistration", &InputMapperRequestBus::Events::StartHotkeyRegistration);

		behaviorContext->EBus<InputMapperNotificationBus>("InputMapperNotificationBus")
			->Handler<InputMapperNotificationBusHandler>()
			->Event("OnHotkeyRegistrationStart", &InputMapperNotificationBus::Events::OnHotkeyRegistrationStart)
			->Event("OnHotkeyRegistrationChanged", &InputMapperNotificationBus::Events::OnHotkeyRegistrationChanged)
			->Event("OnHotkeyRegistrationDone", &InputMapperNotificationBus::Events::OnHotkeyRegistrationDone);

		behaviorContext->EBus<CanvasInputRequestBus>("CanvasInputRequestBus")
			->Event("GetMouseButton", &CanvasInputRequestBus::Events::GetMouseButton)
			->Event("GetMouseEventType", &CanvasInputRequestBus::Events::GetMouseEventType)
			->Event("ConsumeEvent", &CanvasInputRequestBus::Events::ConsumeEvent);

		behaviorContext->Class<EmptyClass>("MouseButton")
			->Enum<(int)MouseButton::Left>("Left")
			->Enum<(int)MouseButton::Right>("Right")
			->Enum<(int)MouseButton::Middle>("Middle");

		behaviorContext->Class<EmptyClass1>("MouseEventType")
			->Enum<(int)MouseEventType::onmouseup>("onmouseup")
			->Enum<(int)MouseEventType::onmousedown>("onmousedown")
			->Enum<(int)MouseEventType::onclick>("onmouseclick");

		behaviorContext->EBus<MouseInputNotificationBus>("MouseInputNotificationBus")
			->Handler<MouseInputNotificationBusHandler>();

		behaviorContext->EBus<MouseCursorPosition::MouseCursorPositionRequestBus>("MouseCursorPositionRequestBus")
			->Event("GetSystemCursorPositionPixelSpace", &MouseCursorPosition::MouseCursorPositionRequestBus::Events::GetSystemCursorPositionPixelSpace)
			->Event("GetSystemCursorPositionNormalized", &MouseCursorPosition::MouseCursorPositionRequestBus::Events::GetSystemCursorPositionNormalized);
	}
}

void InputMapperSystemComponent::Init ()
{
	AZ_Printf(0, "InputMapperSystemComponent::Init()");

}

void InputMapperSystemComponent::Activate ()
{
	AZ_Printf(0, "InputMapperSystemComponent::Activate()");

	InputMapperRequestBus::Handler::BusConnect();
	CanvasInputRequestBus::Handler::BusConnect();
	GameManagerNotificationBus::Handler::BusConnect();

	InputChannelEventListener::Connect();

	AZ::TickBus::Handler::BusConnect();
}

void InputMapperSystemComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time)
{
	AZ::TickBus::Handler::BusDisconnect();

	//we need to create references to textures before the shaders load, so they can be automatically binded from ShaderTemplate.cpp 
	IRenderer* renderer = gEnv->pRenderer;
	if (renderer)
	{
		renderer->DownLoadToVideoMemory(nullptr, 1, 1, eTF_R8, eTF_R8, 1, true, 2, 0, "FogOfWarTex");
		renderer->DownLoadToVideoMemory(nullptr, 1, 1, eTF_R8G8B8A8, eTF_R8G8B8A8, 1, true, 2, 0, "MiniMapTex");
	}
	//

	m_saveToSetting = false;
	OnSetDefaultSettings();
	LoadKeyBindings();
	m_saveToSetting = true;
	SaveKeyBindings();
}

void InputMapperSystemComponent::Deactivate ()
{
	InputChannelEventListener::Disconnect();
	GameManagerNotificationBus::Handler::BusDisconnect();
	CanvasInputRequestBus::Handler::BusDisconnect();
	InputMapperRequestBus::Handler::BusDisconnect();
}


void InputMapperSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
{
    provided.push_back(AZ_CRC("InputMapperService"));
}

void InputMapperSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
{
    incompatible.push_back(AZ_CRC("InputMapperService"));
}

void InputMapperSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
{
    (void)required;
}

void InputMapperSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
{
    (void)dependent;
	dependent.push_back(AZ_CRC("SettingsService"));
}







Slot InputMapperSystemComponent::InputToSlot(const AzFramework::InputChannel& inputChannel)
{
	/*auto it = m_inputMap.find(inputChannel.GetInputChannelId());
	if (it != m_inputMap.end()) {
		return it->second;
	}*/
	return Slot();
}
void InputMapperSystemComponent::BindInputToSlot(AzFramework::InputChannelId input, Slot slot)
{
	/*m_inputMap[input] = slot;
	m_reverseInputMap[slot] = input;*/
	Hotkey h;
	h.AddKey(input);

	BindHotKeyToSlot(h, slot);
}

void spark::InputMapperSystemComponent::BindHotKeyToSlot(spark::Hotkey hotkey, Slot slot)
{
	sLOG("InputMapperSystemComponent::BindHotKeyToSlot  binding " + hotkey + "->" + slot);

	if (slot.GetType() == Slot::Queue) {
		if (hotkey.GetKeys().size() != 1)
		{
			sWARNING("queue hotkey must have only a key! binding ignored");
			return;
		}
	}
	else 
	{
		auto queueIt = m_reverseInputMap.find(Slot(Slot::Queue));
		if (queueIt != m_reverseInputMap.end())
		{
			if(hotkey.Contains(queueIt->second))
			{
				sWARNING("hotkeys must not include the queue key! binding ignored");
				return;
			}
		}
	}

	//clear previous binding:
	{
		Slot toClear;
		{
			auto it = m_inputMap.find(hotkey);
			if (it != m_inputMap.end())
			{
				toClear = it->second;
			}
		}
		{
			auto it = m_reverseInputMap.find(slot);
			if (it != m_reverseInputMap.end())
			{
				m_inputMap.erase(it->second);
			}
		}
		if (toClear.IsValid())
		{
			m_reverseInputMap.erase(toClear);
		}
	}

	m_inputMap[hotkey] = slot;
	m_reverseInputMap[slot] = hotkey;

	if (!m_saveToSetting)return;

	AZStd::string key,value;
	{
		if (slot.GetType() < Slot::NUM_TYPES)
		{
			const char* types[] = { "invalid","ability","inventory","attack","queue","stop","shop","chat" };
			key = AZStd::string::format("%s%d",  types[slot.GetType()] , slot.GetIndex());
		}
		else
		{
			key = AZStd::string::format("Slot(%d,%d)", slot.GetType(), slot.GetIndex());
		}
	}
	{
		auto keys = hotkey.GetKeys();
		for (int i = 0; i < keys.size(); ++i)
		{
			value += KeyToString(keys[i]);
			if (i != keys.size() - 1)value += "+";
		}
	}

	EBUS_EVENT(SettingsRequestBus, SetSettingValue, key, value, "Hotkey");
}

void spark::InputMapperSystemComponent::LoadKeyBindings()
{
	AZStd::vector<const char*> types= {"invalid","ability","inventory","attack","queue","stop","shop","chat"};

	//todo handle deserialization of custom slot types
	for (int t=1;t<types.size();++t)
	{
		for (int i = 0; i < 16; ++i)
		{
			AZStd::string value, key = AZStd::string::format("%s%d",types[t], i);
			EBUS_EVENT_RESULT(value, SettingsRequestBus, GetSettingValue, key, "Hotkey");
			if (!value.empty())
			{
				Hotkey h(value);
				if (h.IsValid()) {
					auto old = m_saveToSetting;
					m_saveToSetting = false;
					BindHotKeyToSlot(h, Slot((Slot::Type)t, i));
					m_saveToSetting = old;
				}
			}
		}
	}
}

void spark::InputMapperSystemComponent::SaveKeyBindings()
{
	for (auto &it : m_inputMap)
	{
		auto &slot = it.second;
		auto &hotkey = it.first;

		AZStd::string key,value;
		{
			const char* types[] = { "invalid","ability","inventory","attack","queue","stop","shop","chat" };
			key=AZStd::string::format("%s%d", slot.GetType() < Slot::NUM_TYPES ? types[slot.GetType()] : "unknown", slot.GetIndex());
		}
		{
			auto keys = hotkey.GetKeys();
			for (int i = 0; i < keys.size(); ++i)
			{
				value += KeyToString(keys[i]);
				if (i != keys.size() - 1)value += "+";
			}
		}

		EBUS_EVENT(SettingsRequestBus, SetSettingValue, key, value, "Hotkey");
	}
}

void spark::InputMapperSystemComponent::OnSetDefaultSettings()
{
	m_inputMap.clear();
	m_reverseInputMap.clear();
	AZ_Printf(0, "InputMapperSystemComponent::OnSetDefaultSettings()");

	BindHotKeyToSlot(Hotkey("Enter"), Slot(Slot::Chat));

	BindHotKeyToSlot(Hotkey("SHIFT"), Slot(Slot::Queue));
	BindHotKeyToSlot(Hotkey("A"), Slot(Slot::Attack));
	BindHotKeyToSlot(Hotkey("S"), Slot(Slot::Stop));


	BindHotKeyToSlot(Hotkey("q"), Slot(Slot::Ability,0));
	BindHotKeyToSlot(Hotkey("w"), Slot(Slot::Ability,1));
	BindHotKeyToSlot(Hotkey("e"), Slot(Slot::Ability,2));
	BindHotKeyToSlot(Hotkey("r"), Slot(Slot::Ability,3));
	BindHotKeyToSlot(Hotkey("d"), Slot(Slot::Ability,4));
	BindHotKeyToSlot(Hotkey("t"), Slot(Slot::Ability,5));

	BindHotKeyToSlot(Hotkey("z"), Slot(Slot::Inventory,0));
	BindHotKeyToSlot(Hotkey("x"), Slot(Slot::Inventory,1));
	BindHotKeyToSlot(Hotkey("c"), Slot(Slot::Inventory,2));
	BindHotKeyToSlot(Hotkey("v"), Slot(Slot::Inventory,3));
	BindHotKeyToSlot(Hotkey("b"), Slot(Slot::Inventory,4));
	BindHotKeyToSlot(Hotkey("n"), Slot(Slot::Inventory,5));
}

void spark::InputMapperSystemComponent::OnGameManagerActivated(AZ::Entity * gameManager)
{
	AZ_Printf(0,"InputMapperSystemComponent::OnGameManagerActivated()");
	//m_saveToSetting = false;
	//OnSetDefaultSettings();
	//LoadKeyBindings();
	//m_saveToSetting = true;
	//SaveKeyBindings();
}

Hotkey spark::InputMapperSystemComponent::GetSlotHotkey(Slot slot)
{
	auto it = m_reverseInputMap.find(slot);
	if (it != m_reverseInputMap.end()) {
		return it->second;
	}
	return Hotkey();
}

void InputMapperSystemComponent::StartHotkeyRegistration()
{
	m_currentHotkey = Hotkey();
	m_state = S_REGISTERING;
	AZ_Printf(0,"InputMapperSystemComponent::StartHotkeyRegistration()");

	EBUS_EVENT(InputMapperNotificationBus, OnHotkeyRegistrationStart);
}


bool InputMapperSystemComponent::OnInputChannelEventFiltered (const AzFramework::InputChannel& inputChannel)
{
	if (m_simulatingMouseButton)return true;

	if (inputChannel.GetInputDevice().GetInputDeviceId() == AzFramework::InputDeviceMouse::Id)
	{
		auto input_type = inputChannel.GetInputChannelId();

		if (input_type == InputDeviceMouse::SystemCursorPosition) //onmousemove
		{
			AZ::Vector2 mousePos;
			EBUS_EVENT_RESULT(mousePos, MouseCursorPosition::MouseCursorPositionRequestBus, GetSystemCursorPositionPixelSpace);
				
			EBUS_EVENT(MouseInputNotificationBus, onmousemove, (int)mousePos.GetX(), (int)mousePos.GetY());
		}
		else if(inputChannel.IsStateBegan() || inputChannel.IsStateEnded())
		{
			int button = -1;
			if (input_type == InputDeviceMouse::Button::Left)button = MouseButton::Left;
			else if (input_type == InputDeviceMouse::Button::Middle)button = MouseButton::Middle;
			else if (input_type == InputDeviceMouse::Button::Right)button = MouseButton::Right;

			if (button >= 0)
			{
				bool consumed=false;
				if (inputChannel.IsStateBegan())
				{
					EBUS_EVENT_RESULT(consumed, MouseInputNotificationBus, onmousedown, button);
				}
				else
				{
					EBUS_EVENT_RESULT(consumed, MouseInputNotificationBus, onmouseup, button);
				}
				if (consumed)return true;
			}
		}
	}

	if (inputChannel.GetInputDevice().GetInputDeviceId() == AzFramework::InputDeviceMouse::Id && (inputChannel.IsStateEnded() || inputChannel.IsStateBegan()))
	{
		if(HandleCanvasEvent(inputChannel))return true;
	}




	if (inputChannel.GetInputDevice().GetInputDeviceId() != InputDeviceKeyboard::Id) return false; //consider only keyboard events from now on
	if (!inputChannel.IsStateBegan() && !inputChannel.IsStateEnded())return false; //ignore holding down events



	{//filter the queue key ( we don't want it to mess with the other hotkeys ) 
		//N.B. -is not modificable with the registration method
		//	   -must contain only a key
		//     -other combination should not include the queue key (in the registration phase it is ingnored, not so in the loading from the settings or other methods)
		Hotkey h;
		h.AddKey(inputChannel.GetInputChannelId());
		auto it = m_inputMap.find(h);
		if (it != m_inputMap.end() && it->second.GetType() == Slot::Queue)
		{
			sLOG("queue key event");

			if (inputChannel.IsStateBegan())
			{
				EBUS_EVENT(SlotNotificationBus, OnSlotPressed, it->second);
			}
			else
			{
				EBUS_EVENT(SlotNotificationBus, OnSlotReleased, it->second);
			}
			return true;
		}
	}


	if (m_state == S_REGISTERING)
	{
		if (inputChannel.IsStateBegan())
		{
			m_currentHotkey.AddKey(inputChannel.GetInputChannelId());
			EBUS_EVENT(InputMapperNotificationBus, OnHotkeyRegistrationChanged, m_currentHotkey);
		}
		if (inputChannel.IsStateEnded())
		{
			EBUS_EVENT(InputMapperNotificationBus, OnHotkeyRegistrationDone, m_currentHotkey);
			m_currentHotkey = Hotkey();
			m_state = S_IDLE;
		}

		return true;
	}
	

	if (m_state == S_IDLE && inputChannel.IsStateEnded())
	{
		bool handled = false;
		auto it = m_inputMap.find(m_currentHotkey);
		if (m_currentHotkey.HasKey(inputChannel.GetInputChannelId()) && it != m_inputMap.end())
		{
			EBUS_EVENT(SlotNotificationBus, OnSlotReleased, it->second);
			handled = true;
		}
		m_currentHotkey.RemoveKey(inputChannel.GetInputChannelId());
		AZ_Printf(0, "currentHotkey cleared : %s", m_currentHotkey.ToString().c_str());

		if (handled)return true;
	}

	if (m_state == S_IDLE && inputChannel.IsStateBegan())
	{
		m_currentHotkey.AddKey(inputChannel.GetInputChannelId());

		AZ_Printf(0, "currentHotkey is %s", m_currentHotkey.ToString().c_str());
		auto it = m_inputMap.find(m_currentHotkey);
		if (it != m_inputMap.end()) {

			EBUS_EVENT(SlotNotificationBus, OnSlotPressed, it->second);
			return true;
		}

	}
	return false;
}




//LyShine for some strange reason( console games D: ) don't handle mouse buttons different from the left one.
//Our solution consist in simulating a left click and providing a getter ( GetMouseButton ) to distinguish the mouse buttons
//Remember to consume the events, so they don't propagate to other components.
bool InputMapperSystemComponent::HandleCanvasEvent(const AzFramework::InputChannel & inputChannel)
{
	if (m_simulatingMouseButton)return true;

	auto input_type = inputChannel.GetInputChannelId();

	int button = -1;
	if (input_type == InputDeviceMouse::Button::Left)button = MouseButton::Left;
	else if (input_type == InputDeviceMouse::Button::Middle)button = MouseButton::Middle;
	else if (input_type == InputDeviceMouse::Button::Right)button = MouseButton::Right;

	if (button < 0)return false;

	m_currentMouseButton = button;

	if (inputChannel.IsStateEnded())m_currentMouseEventType = MouseEventType::onclick;
	else if (inputChannel.IsStateBegan())m_currentMouseEventType = MouseEventType::onmousedown;

	if (button == 0)
	{
		sWARNING("InputMapperSystemComponent::HandleCanvasEvent is getting called with left click");
		return false;//lyshine already handle left clicks
	}



	const AzFramework::InputDevice* mouseDevice = AzFramework::InputDeviceRequests::FindInputDevice(AzFramework::InputDeviceMouse::Id);
    if (!mouseDevice || !mouseDevice->IsConnected())
    {
        return false;
    }
	AzFramework::InputChannel *channel = const_cast<AzFramework::InputChannel *>(mouseDevice->GetInputChannelsById().at(InputDeviceMouse::Button::Left));

	if (!channel)
	{
		return false;
	}

	m_simulatingMouseButton = true;
	m_eventConsumed = false;

	auto old_state = channel->IsActive();//we need to store the state of the left button, in case it's already pressed

	AZ_Printf(0,"InputMapperSystemComponent::HandleCanvasEvent simulating left click");
	channel->UpdateState(false);
	channel->UpdateState(true); //generate the click event in LyShyne

	//reset the previous state
	channel->UpdateState(old_state);

	m_simulatingMouseButton = false;

	m_currentMouseButton = 1;
	m_currentMouseEventType = MouseEventType::onclick;

	AZ_Printf(0,"InputMapperSystemComponent::HandleCanvasEvent event consumed = %d", m_eventConsumed);

	return m_eventConsumed;
}
