
#include "spark_precompiled.h"

#include <ISystem.h>
#include <CryAction.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/Entity.h>
#include <AzFramework/Script/ScriptComponent.h>
#include <AzCore/Asset/AssetManagerBus.h>
#include <AzFramework/Entity/EntityContextBus.h>
#include <AzFramework/Entity/EntityContext.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/std/algorithm.h>

#include "GameManagerSystemComponent.h"

#include "Components/AbilityComponent.h"
#include "Components/UnitComponent.h"

#include "Components/GameManagerComponent.h"
#include "Components/CameraControllerComponent.h"
#include "Components/GameNetSyncComponent.h"
#include "Components/ProjectileManagerSystemComponent.h"
#include "Components/VariableManagerComponent.h"
#include <AzFramework/Network/NetBindingComponent.h>

#include "Busses/UnitBus.h"
#include "Utils/StringUtils.h"
#include "Utils/FileUtils.h"


#include <AzFramework/Entity/GameEntityContextBus.h>
#include <AzCore/Component/ComponentApplicationBus.h>

#include "Utils/JsonUtils.h"
#include "Utils/Log.h"

using namespace spark;

AZStd::vector<AZStd::string> GameManagerSystemComponent::s_filenames;
AZStd::vector<AZStd::string> GameManagerSystemComponent::s_directories;

void GameManagerSystemComponent::Reflect(AZ::ReflectContext* context)
{
	if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
	{
		serialize->Class<GameManagerSystemComponent, AZ::Component>()
			->Version(0)
			;

		if (AZ::EditContext* ec = serialize->GetEditContext())
		{
			ec->Class<GameManagerSystemComponent>("GameManagerSystemComponent", "Game utilities and management")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
				//   ->Attribute(AZ::Edit::Attributes::Category, "spark")
				//	->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"));
				//;
				->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
				->Attribute(AZ::Edit::Attributes::AutoExpand, true);
		}
	}

	if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
	{
		behaviorContext->EBus<GameManagerSystemRequestBus>("GameManagerSystemRequestBus")
			->Event("ExecuteConsoleCommand", &GameManagerSystemRequestBus::Events::ExecuteConsoleCommand)
			->Event("LoadGameFile", &GameManagerSystemRequestBus::Events::LoadGameFile)
			->Event("LoadGameMode", &GameManagerSystemRequestBus::Events::LoadGameMode)
			->Event("PlayGame", &GameManagerSystemRequestBus::Events::PlayGame);
	}
}

void GameManagerSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
{
	provided.push_back(AZ_CRC("GameManagerSystemService"));
}

void GameManagerSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
{
	incompatible.push_back(AZ_CRC("GameManagerSystemService"));
}

void GameManagerSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
{
	(void)required;
}

void GameManagerSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
{
	(void)dependent;
}

void GameManagerSystemComponent::Init()
{
	
}

void GameManagerSystemComponent::Activate()
{
	GameManagerSystemRequestBus::Handler::BusConnect();
	GameManagerNotificationBus::Handler::BusConnect();
}

void GameManagerSystemComponent::Deactivate()
{
	GameManagerNotificationBus::Handler::BusDisconnect();
	GameManagerSystemRequestBus::Handler::BusDisconnect();
}






bool GameManagerSystemComponent::LoadGameFile(AZStd::string filename)
{
	//"header" guard
	if (!FileUtils::FileExists(filename))
	{
		// try some file extensions
		if (FileUtils::FileExists(filename + ".json"))
		{
			filename = filename + ".json";
		}
		else if (FileUtils::FileExists(filename + ".txt"))
		{
			filename = filename + ".txt";
		}
	}
	auto found = std::find(s_filenames.begin(), s_filenames.end(), filename);
	if (found != s_filenames.end())
	{
		return false;
	}
	s_filenames.push_back(filename);

	auto result = FileUtils::ReadJsonFile(filename);
	if (!result.IsSuccess())
	{
		sWARNING("Unable to read game file \""+filename +"\" error:"+ result.GetError());
		return false;
	}

	auto document = result.TakeValue();

	//libraries
	auto import = document.FindMember("import");
	if (import != document.MemberEnd() && import->value.IsArray())
	{
		for (auto it = import->value.Begin(); it != import->value.End(); ++it)
		{
			if (it->IsString())
			{
				LoadGameMode(it->GetString());
			}
		}
		document.EraseMember(import);
	}
	//other files
	auto include = document.FindMember("include");
	if (include != document.MemberEnd() && include->value.IsArray())
	{
		for (auto it = include->value.Begin(); it != include->value.End(); ++it)
		{
			if (it->IsString())
			{
				AZStd::string newfile(it->GetString());
				StringUtils::trim(newfile);
				AZStd::size_t found = filename.find_last_of("/\\");
				AZStd::string path = filename.substr(0, found);
				LoadGameFile(AZStd::string::format("%s/%s", path.c_str() , newfile.c_str()));
			}
		}
		document.EraseMember(include);
	}

	JsonUtils::MergeObjects(document,m_gameDocument, document.GetAllocator());
	m_gameDocument.Swap(document);

	sLOG("GameManagerSystemComponent::LoadGameFile file loaded correctly : "+filename);
	sLOG(JsonUtils::ToString(m_gameDocument));

	return true;
}

bool GameManagerSystemComponent::LoadGameMode(AZStd::string gamemodeName)
{
	AZStd::string gamemodePath = AZStd::string::format("%s/gamemode/%s", gEnv->pFileIO->GetAlias("@assets@"), gamemodeName.c_str());
	bool result = LoadGameFile( gamemodePath + "/game.txt");
	if (result)
	{
		s_directories.push_back(gamemodePath);
	}
	return result;
}

void GameManagerSystemComponent::PlayGame()
{
	using namespace rapidjson;


	if (!m_gameDocument.IsObject() || !m_gameDocument.HasMember("gamemode"))
	{
		sWARNING("gamemode was not defined");
		return;
	}

	rapidjson::Value &info = m_gameDocument["gamemode"];

	//load map
	AZStd::string map_name = info.HasMember("map") ? info["map"].GetString() : "emptylevel";

	//todo check for injection attack
	ExecuteConsoleCommand(AZStd::string::format("map %s", map_name.c_str()));


	////create camera
	//const AZ::Data::AssetType& dynamicSliceAssetType = azrtti_typeid<AZ::DynamicPrefabAsset>();
	//{
	//	AZ::Data::AssetId assetId;
	//	AZStd::string str = AZStd::string::format("%s/Slices/Camera.slice", gEnv->pFileIO->GetAlias("@assets@"));
	//	CryLog("dynamicSlice Path: %s", str.c_str());
	//	EBUS_EVENT_RESULT(assetId, AZ::Data::AssetCatalogRequestBus, GetAssetIdByPath, str.c_str(), dynamicSliceAssetType, true);

	//	if (assetId.IsValid()) {
	//		CryLog("Loading dynamicSlice");
	//		AZ::Data::Asset<AZ::DynamicPrefabAsset> dynamicSliceAsset(assetId, dynamicSliceAssetType);
	//		AzFramework::SliceInstantiationTicket ticket;
	//		AZ::Transform transform;
	//		transform.SetFromEulerDegrees(AZ::Vector3(305, 0, 0));
	//		transform.SetPosition(500, 500, 50);
	//		EBUS_EVENT_RESULT(ticket, AzFramework::GameEntityContextRequestBus, InstantiateDynamicSlice, dynamicSliceAsset, transform, nullptr);
	//	}
	//	else
	//	{
	//		AZ_Error(0, false, "cannot instantiate camera slice");
	//	}
	//}

	m_tickCounter = 300;
	AZ::SystemTickBus::Handler::BusConnect();
}

void GameManagerSystemComponent::CreateGameManager()
{
	bool gameManagerAlreadyExists = false;
	EBUS_EVENT_RESULT(gameManagerAlreadyExists, GameManagerRequestBus, GameManagerAlreadyExists);

	if (gameManagerAlreadyExists)
	{
		AZ_Printf(0, "GameManager already exists");
	}
	else
	{
		AZ_Printf(0, "GameManager is missing : creating the default one");

		rapidjson::Value &info = m_gameDocument["gamemode"];

		//create game manager
		AZ::Entity *gameManager = aznew AZ::Entity("GameManager");
		if (gameManager) {

			GameManagerComponent* gm = gameManager->CreateComponent<GameManagerComponent>();
			gameManager->CreateComponent<ProjectileManagerSystemComponent>();
			gameManager->CreateComponent<GameNetSyncComponent>();
			gameManager->CreateComponent<AzFramework::NetBindingComponent>();


			const AZ::Data::AssetType& scriptAssetType = azrtti_typeid<AZ::ScriptAsset>();

			////script for showing cursor
			//AzFramework::ScriptComponent *cursor = gameManager->CreateComponent<AzFramework::ScriptComponent>();
			//{
			//	AZ::Data::AssetId assetId;
			//	AZStd::string str = AZStd::string::format("%s/scripts/ShowCursor.lua", gEnv->pFileIO->GetAlias("@assets@"));
			//	CryLog("Script Path: %s", str.c_str());
			//	EBUS_EVENT_RESULT(assetId, AZ::Data::AssetCatalogRequestBus, GetAssetIdByPath, str.c_str(), scriptAssetType, true);
			//	//EBUS_EVENT( AZ::Data::AssetCatalogRequestBus, UnregisterAsset, assetId);
			//	//EBUS_EVENT_RESULT(assetId, AZ::Data::AssetCatalogRequestBus, GetAssetIdByPath, str.c_str(), scriptAssetType, true);	

			//	if (assetId.IsValid()) {
			//		CryLog("Setting Script.");
			//		AZ::Data::Asset<AZ::ScriptAsset> scriptAsset(assetId, scriptAssetType);
			//		cursor->SetScript(scriptAsset);
			//	}
			//}

			//script for handling modifiers
			AzFramework::ScriptComponent *modifiers = gameManager->CreateComponent<AzFramework::ScriptComponent>();
			{
				AZ::Data::AssetId assetId;
				AZStd::string str = AZStd::string::format("%s/scripts/modifier.lua", gEnv->pFileIO->GetAlias("@assets@"));
				CryLog("Script Path: %s", str.c_str());
				EBUS_EVENT_RESULT(assetId, AZ::Data::AssetCatalogRequestBus, GetAssetIdByPath, str.c_str(), scriptAssetType, true);

				if (assetId.IsValid()) {
					CryLog("Setting Script.");
					AZ::Data::Asset<AZ::ScriptAsset> scriptAsset(assetId, scriptAssetType);
					modifiers->SetScript(scriptAsset);
				}
			}

			//script for handling game mode
			if (info.HasMember("lua-file")) {
				AzFramework::ScriptComponent *scriptComponent = gameManager->CreateComponent<AzFramework::ScriptComponent>();
				AZ::Data::AssetId assetId;
				AZStd::string str = AZStd::string::format("%s/scripts/%s", gEnv->pFileIO->GetAlias("@assets@"), info["lua-file"].GetString());
				CryLog("Script Path: %s", str.c_str());
				EBUS_EVENT_RESULT(assetId, AZ::Data::AssetCatalogRequestBus, GetAssetIdByPath, str.c_str(), scriptAssetType, true);

				if (assetId.IsValid()) {
					CryLog("Setting Script.");
					AZ::Data::Asset<AZ::ScriptAsset> scriptAsset(assetId, scriptAssetType);
					scriptComponent->SetScript(scriptAsset);
				}
			}


			////script for loading the HUD canvas
			//AzFramework::ScriptComponent *canvas = gameManager->CreateComponent<AzFramework::ScriptComponent>();
			//{
			//	AZ::Data::AssetId assetId;
			//	AZStd::string str = AZStd::string::format("%s/scripts/LoadHUD.lua", gEnv->pFileIO->GetAlias("@assets@"));
			//	CryLog("Script Path: %s", str.c_str());
			//	EBUS_EVENT_RESULT(assetId, AZ::Data::AssetCatalogRequestBus, GetAssetIdByPath, str.c_str(), scriptAssetType, true);

			//	if (assetId.IsValid()) {
			//		CryLog("Setting Script.");
			//		AZ::Data::Asset<AZ::ScriptAsset> scriptAsset(assetId, scriptAssetType);
			//		canvas->SetScript(scriptAsset);
			//	}
			//}

			//gm->m_gameDocument.Swap(m_gameDocument);

			gameManager->Init();
			gameManager->Activate();
		}
	}
}

void GameManagerSystemComponent::OnSystemTick()
{
	--m_tickCounter;
	if (m_tickCounter <= 0)
	{
		CreateGameManager();
		AZ::SystemTickBus::Handler::BusDisconnect();
	}
}


void GameManagerSystemComponent::ExecuteConsoleCommand(AZStd::string cmd)
{
	//todo check for injection attack
	gEnv->pConsole->ExecuteString(cmd.c_str(), false, true);
}



struct DependeciesGraphNode
{
	AZStd::vector<ItemTypeId> dependencies;
	AZStd::vector<ItemTypeId> required_for;
};

void CalculateItemsDependencies(rapidjson::Document &document)
{
	sLOG("CalculateItemsDependencies called");

	if (!document.IsObject() || !document.HasMember("items"))return;

	auto &allocator = document.GetAllocator();
	auto &items = document["items"];

	AZStd::unordered_map<ItemTypeId, DependeciesGraphNode> graph;

	//get all node
	for (auto item = items.MemberBegin(); item != items.MemberEnd(); ++item)
	{
		graph[item->name.GetString()] = DependeciesGraphNode();
	}

	//check and set dependencies and "required_for"( if the items in the dependency list exist )
	for (auto item = items.MemberBegin(); item != items.MemberEnd(); ++item)
	{
		if (!item->value.IsObject() || !item->value.HasMember("dependencies"))continue;
		
		auto &dependencies = item->value["dependencies"];
		if (!dependencies.IsArray())continue;
			
		AZStd::vector<ItemTypeId> dependencies_vector;
		for (auto itr = dependencies.Begin(); itr != dependencies.End(); ++itr)
		{
			if (itr->IsString() && graph.find(itr->GetString()) != graph.end())
			{
				dependencies_vector.push_back(itr->GetString());
				graph[itr->GetString()].required_for.push_back(item->name.GetString());
			}
		}	

		graph[item->name.GetString()].dependencies = dependencies_vector;
	}

	//update the json
	for (auto item = items.MemberBegin(); item != items.MemberEnd(); ++item)
	{
		//sLOG(item->name.GetString()+" is "+JsonUtils::ToString(item->value));

		if (!item->value.IsObject())continue;
		rapidjson::Value dep(rapidjson::kArrayType);
		for (auto d : graph[item->name.GetString()].dependencies)
		{
			rapidjson::Value str;
			str.SetString(d.c_str(),allocator);
			dep.PushBack(str,allocator);
		}
		rapidjson::Value req(rapidjson::kArrayType);
		for (auto d : graph[item->name.GetString()].required_for)
		{
			rapidjson::Value str;
			str.SetString(d.c_str(),allocator);
			req.PushBack(str,allocator);
		}
		item->value.RemoveMember("dependencies");
		item->value.RemoveMember("required_for");
		item->value.AddMember("dependencies", dep, allocator);
		item->value.AddMember("required_for", req, allocator);
	}


}




void CalculateExtends(rapidjson::Value &object, rapidjson::Value &parent, rapidjson::Document &document)
{
	if (!object.IsObject())return;

	AZStd::vector<AZStd::string> extends;

	auto extendsJson = object.FindMember("extends");
	if (extendsJson != object.MemberEnd())
	{
		if (extendsJson->value.IsArray())
		{
			for (auto it = extendsJson->value.Begin(); it != extendsJson->value.End(); ++it)
			{
				if (it->IsString())
				{
					extends.push_back(it->GetString());
				}
			}
		}
		else if (extendsJson->value.IsString())
		{
			extends.push_back(extendsJson->value.GetString());
		}
	}
	if (extendsJson != object.MemberEnd())
	{
		object.EraseMember(extendsJson); //so we don't create extend loops
	}

	for (auto e : extends)
	{
		auto path = StringUtils::SplitString(e, "/");

		rapidjson::Value* root = &parent; 
		rapidjson::Value* value = JsonUtils::FindValue(path, *root); //try the relative path

		root = &document;
		if (!value)value = JsonUtils::FindValue(path, *root); //try the absolute path

		if (value)
		{
			path.pop_back();
			auto value_parent = JsonUtils::FindValue(path, *root);
			
			CalculateExtends(*value, *value_parent, document);

			JsonUtils::MergeObjects(object, *value, document.GetAllocator());
		}
	}
};

void CalculateExtends(rapidjson::Document &document)
{
	sLOG("CalculateExtends called");

	/* syntax is
		"something_else":{
			...
		},
		"something" : {
			"extends" : "something_else",
			...
		}

	*/

	JsonUtils::ForeachMemberWithParent(document, [&document](rapidjson::Value &member, rapidjson::Value &parent)
	{
		CalculateExtends(member, parent, document);
	});

}

void GameManagerSystemComponent::OnGameManagerActivated(AZ::Entity* gameManager)
{

	AZ_Printf(0,"GameManagerSystemComponent:OnGameManagerActivated called");
	AZ_Error("GameManagerSystemComponent", gameManager, " OnGameManagerActivated with null entity pointer");

	GameManagerComponent* gm = gameManager->FindComponent<GameManagerComponent>();
	AZ_Error("GameManagerSystemComponent", gm, " OnGameManagerActivated called, but the entity is without a GameManagerComponent");

	AZ::SystemTickBus::Handler::BusDisconnect();

	if (!m_gameDocument.IsObject() || m_gameDocument.MemberCount()==0)// !gm->m_gameDocument.IsObject() || gm->m_gameDocument.ObjectEmpty())
	{
		LoadGameMode(gm->GetGamemode());
	}


	CalculateExtends(m_gameDocument);
	CalculateItemsDependencies(m_gameDocument);
	
	AZStd::reverse(s_directories.begin(), s_directories.end());//imported gamemodes are inserted before, so we need to reverse the vector to have the priorities right

	sLOG("final gamemode json is : "+JsonUtils::ToString(m_gameDocument));

	gm->SetGameJson(m_gameDocument, s_directories);

	m_gameDocument.SetObject();//clear the document and all for the next gamemode
	s_filenames.clear();
	s_directories.clear();

	gm->StartGame();
}

