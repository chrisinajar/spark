

#include "spark_precompiled.h"




#include "LocalizationSystemComponent.h"

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <ISystem.h>
#include <ILocalizationManager.h>

#include "Utils/FileUtils.h"
#include "Utils/Log.h"

namespace spark
{


	AZStd::string GetLocalizationFilename()
	{
		return  AZStd::string::format("%s/Localization.json", gEnv->pFileIO->GetAlias("@user@"));
		//return "@user@/Localization.txt";// AZStd::string::format("%s/userdata/Localization.txt", gEnv->pFileIO->GetAlias("@assets@"));
	}




	class LocalizationNotificationBusHandler
		: public LocalizationNotificationBus::Handler
		, public AZ::BehaviorEBusHandler
	{
	public:
		AZ_EBUS_BEHAVIOR_BINDER(LocalizationNotificationBusHandler, "{09C458F6-AFD6-44AD-927E-E887BB649887}", AZ::SystemAllocator, OnLanguageChanged);

		void OnLanguageChanged(const AZStd::string &language) override
		{
			Call(FN_OnLanguageChanged, language);
		}
	};



	//LocalizationSystemComponent implementation

	void LocalizationSystemComponent::Reflect(AZ::ReflectContext* reflection)
	{
		if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
		{
			serializationContext->Class<LocalizationSystemComponent, AZ::Component>()
				->Version(1);


			if (auto editContext = serializationContext->GetEditContext())
			{
				editContext->Class<LocalizationSystemComponent>("LocalizationSystemComponent", "")
					->ClassElement(AZ::Edit::ClassElements::EditorData, "")
					->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
					->Attribute(AZ::Edit::Attributes::AutoExpand, true);
			}
		}

		if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
		{

			behaviorContext->EBus<LocalizationRequestBus>("LocalizationRequestBus")
				->Event("LocalizeString", &LocalizationRequestBus::Events::LocalizeString)
				->Event("GetLanguage", &LocalizationRequestBus::Events::GetLanguage)
				->Event("SetLanguage", &LocalizationRequestBus::Events::SetLanguage)
				->Event("GetAvailableLanguages", &LocalizationRequestBus::Events::GetAvailableLanguages);

			behaviorContext->EBus<LocalizationNotificationBus>("LocalizationNotificationBus")
				->Handler<LocalizationNotificationBusHandler>();
		}
	}

	void LocalizationSystemComponent::Init()
	{
		
	}

	void LocalizationSystemComponent::Activate()
	{
		LocalizationRequestBus::Handler::BusConnect();

		AZ::TickBus::Handler::BusConnect();
		//ILocalizationManager* pLocMan = GetISystem()->GetLocalizationManager();
		//if (pLocMan)
		//{
		//	AZStd::string localizationXml("libs/localization/localization.xml");
		//	if (pLocMan->InitLocalizationData(localizationXml.c_str()))
		//	{
		//	//	/*const bool initLocSuccess = pLocMan->LoadLocalizationDataByTag("init");
		//	//	AZ_Error("Localization", initLocSuccess, "LoadLocalizationData() failed to load localization file=%s", localizationXml.c_str());*/
		//	}
		//}
	}

	void LocalizationSystemComponent::Deactivate()
	{
		LocalizationRequestBus::Handler::BusDisconnect();
	}


	void LocalizationSystemComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time)
	{
		AZ::TickBus::Handler::BusDisconnect();


		ILocalizationManager* pLocMan = GetISystem()->GetLocalizationManager();
		if (pLocMan)
		{
			AZStd::string localizationXml("libs/localization/localization.xml");


			sLOG("LOADING LOCALIZATION DATA");

			if (pLocMan->InitLocalizationData(localizationXml.c_str()))
			{
				const bool initLocSuccess = pLocMan->LoadLocalizationDataByTag("init");
				AZ_Error("Localization", initLocSuccess, "LoadLocalizationData() failed to load localization file=%s", localizationXml.c_str());
			}

			sLOG("supported languages:");
			for (unsigned int i = 0; i < ILocalizationManager::EPlatformIndependentLanguageID::ePILID_MAX_OR_INVALID; i++)
			{
				//if (pLocMan->IsLanguageSupported((ILocalizationManager::EPlatformIndependentLanguageID)i))
				{
					sLOG(pLocMan->LangNameFromPILID((ILocalizationManager::EPlatformIndependentLanguageID)i));
				}
			}

			sLOG("current language=" + pLocMan->GetLanguage());
			sLOG("string count =" + pLocMan->GetLocalizedStringCount());

			//for (int index = 0; index < 20; index++)
			//{
			//	SLocalizedInfoGame info;
			//	if (pLocMan->GetLocalizedInfoByIndex(index, info))
			//	{
			//		sLOG(index + "=" + info.sUtf8TranslatedText.c_str());
			//	}

			//}
		}
	}


	void LocalizationSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
	{
		provided.push_back(AZ_CRC("LocalizationService"));
	}

	void LocalizationSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
	{
		incompatible.push_back(AZ_CRC("LocalizationService"));
	}

	void LocalizationSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
	{
		(void)required;
	}

	void LocalizationSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
	{
		(void)dependent;
	}


	
	AZStd::string LocalizationSystemComponent::LocalizeString(AZStd::string id)
	{
		string locText;
		GetISystem()->GetLocalizationManager()->LocalizeString(id.c_str(), locText);
		return locText.c_str();
	}

	AZStd::string LocalizationSystemComponent::GetLanguage()
	{
		return GetISystem()->GetLocalizationManager()->GetLanguage();
	}

	void LocalizationSystemComponent::SetLanguage(AZStd::string language)
	{
		//todo check if language is avaiable

		gEnv->pConsole->ExecuteString(AZStd::string::format("sys_localization_folder Localization/%s_xml", language.c_str()).c_str());
		gEnv->pConsole->ExecuteString(AZStd::string::format("g_language %s", language.c_str()).c_str());
		//GetISystem()->GetLocalizationManager()->SetLanguage(language.c_str());

		EBUS_EVENT(LocalizationNotificationBus, OnLanguageChanged, language);
	}

	AZStd::vector<AZStd::string> LocalizationSystemComponent::GetAvailableLanguages()
	{
		AZStd::vector<AZStd::string> l;
		return l;
	}


	AZStd::string GetLocalizatedString(const rapidjson::Value &value)
	{
		if (value.IsString())return value.GetString();

		AZ_Warning(0, value.IsObject(), "GetLocalizatedString called with no json object");
		if (!value.IsObject())return "";

		AZStd::string language;
		EBUS_EVENT_RESULT(language, LocalizationRequestBus, GetLanguage);

		auto it=value.FindMember(language.c_str());
		if (it != value.MemberEnd() && it->value.IsString())
		{
			return it->value.GetString();
		}

		it = value.FindMember("english");
		if (it != value.MemberEnd() && it->value.IsString())
		{
			return it->value.GetString();
		}

		return "";
	}
}
