

#include "spark_precompiled.h"




#include "SettingsSystemComponent.h"

#include <AzCore/Serialization/EditContext.h>

#include "Busses/SlotBus.h"

#include "Utils/FileUtils.h"



#include <AzCore/JSON/stringbuffer.h>
#include <AzCore/JSON/writer.h>
#include <AzCore/JSON/document.h>

#include <ISystem.h>
#include <CryAction.h>
#include <IRenderer.h>

#include "Utils/Log.h"

using namespace spark;




AZStd::string GetSettingsFilename(AZStd::string settingGroup)
{
	return  AZStd::string::format("%s/%s.json", gEnv->pFileIO->GetAlias("@user@"), settingGroup.c_str());
	//return "@user@/settings.txt";// AZStd::string::format("%s/userdata/settings.txt", gEnv->pFileIO->GetAlias("@assets@"));
}




class SettingsNotificationBusHandler
	: public SettingsNotificationBus::Handler
	, public AZ::BehaviorEBusHandler
{
public:
	AZ_EBUS_BEHAVIOR_BINDER(SettingsNotificationBusHandler, "{FE652636-F89A-40A1-AE23-B2507F74AC89}", AZ::SystemAllocator, OnSettingChanged);

	void OnSettingChanged(AZStd::string key, AZStd::string value) {
		Call(FN_OnSettingChanged,key,value);
	}
};



//SettingsSystemComponent implementation

void SettingsSystemComponent::Reflect (AZ::ReflectContext* reflection)
{
    if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
    {
        serializationContext->Class<SettingsSystemComponent, AZ::Component>()
            ->Version(1);


        if (auto editContext = serializationContext->GetEditContext())
        {
			editContext->Class<SettingsSystemComponent>("SettingsSystemComponent", "")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
				->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
				->Attribute(AZ::Edit::Attributes::AutoExpand, true);
        }
    }

	if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
	{
		

		behaviorContext->EBus<SettingsRequestBus>("SettingsRequestBus")
			->Event("SetSettingValue", &SettingsRequestBus::Events::SetSettingValue)
			->Event("GetSettingValue", &SettingsRequestBus::Events::GetSettingValue)
			->Event("ChangeVideoMode", &SettingsRequestBus::Events::ChangeVideoMode)
			->Event("GetScreenResolution", &SettingsRequestBus::Events::GetScreenResolution)
			->Event("IsFullscreen", &SettingsRequestBus::Events::IsFullscreen);

		behaviorContext->EBus<SettingsNotificationBus>("SettingsNotificationBus")
			->Handler<SettingsNotificationBusHandler>()
			->Event("OnSettingChanged", &SettingsNotificationBus::Events::OnSettingChanged);
	}	
}

void SettingsSystemComponent::Init ()
{
	AZ_Printf(0, "SettingsSystemComponent::Init()");
}

void SettingsSystemComponent::Activate ()
{
	AZ_Printf(0, "SettingsSystemComponent::Activate()");
	SettingsRequestBus::Handler::BusConnect();
}

void SettingsSystemComponent::Deactivate ()
{
	SettingsRequestBus::Handler::BusDisconnect();
}



void SettingsSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
{
    provided.push_back(AZ_CRC("SettingsService"));
}

void SettingsSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
{
    incompatible.push_back(AZ_CRC("SettingsService"));
}

void SettingsSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
{
    (void)required;
}

void SettingsSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
{
    (void)dependent;
}




void spark::SettingsSystemComponent::SetSettingValue(AZStd::string key, AZStd::string value, AZStd::string settingGroup)
{
	//AZ_Printf(0, "SettingsSystemComponent::SetSettingValue %s", key.c_str());
	//AZ_Printf(0, "filename is %s", GetSettingsFilename().c_str());

	auto outcome=FileUtils::ReplaceInCfgFile(GetSettingsFilename(settingGroup),key,value);
	if (!outcome.IsSuccess())
	{
		sDEBUG("SettingsSystemComponent] error setting value: " + outcome.GetError());
		//AZ_Warning("SettingsSystemComponent", false, "error storing setting value: %s", outcome.GetError().c_str());
		return;
	}

	EBUS_EVENT(SettingsNotificationBus, OnSettingChanged, key, value);
}


AZStd::string spark::SettingsSystemComponent::GetSettingValue(AZStd::string key, AZStd::string settingGroup)
{
	//AZ_Printf(0, "SettingsSystemComponent::GetSettingValue %s", key.c_str());
	
	auto outcome= FileUtils::GetValueForKeyInCfgFile(GetSettingsFilename(settingGroup), key);
	if (!outcome.IsSuccess())
	{
		sDEBUG("SettingsSystemComponent] error getting value: " + outcome.GetError());
		//AZ_Warning("SettingsSystemComponent", false, "error storing getting value: %s", outcome.GetError().c_str());
	}
	return outcome.GetValueOr("");
}



void spark::SettingsSystemComponent::SetDefaultSettings(AZStd::string settingGroup)
{
	FileUtils::WriteTextFile(GetSettingsFilename(settingGroup), "");
	EBUS_EVENT(SettingsNotificationBus, OnSetDefaultSettings);
}


void SettingsSystemComponent::ChangeVideoMode(AZ::Vector2 resolution,bool fullscreen)
{
	m_resolution = resolution;
	m_fullscreen = fullscreen;

#if !defined(_RELEASE)
	ICVar* fullscreenCVar = gEnv->pConsole->GetCVar("r_fullscreen");
	fullscreenCVar->Set(m_fullscreen);
	ICVar* screenWidthCVar = gEnv->pConsole->GetCVar("r_width");
	screenWidthCVar->Set(m_resolution.GetX());
	ICVar* screenHeightCVar = gEnv->pConsole->GetCVar("r_height");
	screenHeightCVar->Set(m_resolution.GetY());
#else
	gEnv->pRenderer->ChangeResolution(m_resolution.GetX(), m_resolution.GetY(), 32, 80, m_fullscreen, false);
#endif
}

AZ::Vector2 SettingsSystemComponent::GetScreenResolution()
{
	float width = GetISystem()->GetIRenderer()->GetWidth(), height = GetISystem()->GetIRenderer()->GetHeight();
	return AZ::Vector2{ width,height };
}

bool SettingsSystemComponent::IsFullscreen()
{
	return m_fullscreen;
}
