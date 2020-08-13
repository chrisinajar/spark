
#include "spark_precompiled.h"

#include "GameManagerComponent.h"

#include <ISystem.h>
#include <CryAction.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/Entity.h>
#include <AzFramework/Script/ScriptComponent.h>
#include <AzFramework/Components/TransformComponent.h>
#include <AzFramework/Script/ScriptNetBindings.h>
#include <LmbrCentral/Scripting/TagComponentBus.h>
#include <AzCore/Asset/AssetManagerBus.h>
#include <AzFramework/Entity/EntityContextBus.h>
#include <AzFramework/Entity/EntityContext.h>
#include <AzCore/Component/TransformBus.h>

#include <LmbrCentral/Shape/CapsuleShapeComponentBus.h>
#include <AzFramework/Physics/ColliderComponentBus.h>
#include <AzFramework/Physics/PhysicsComponentBus.h>

#include <LmbrCentral/Rendering/ParticleComponentBus.h>

#include <AzFramework/Network/NetworkContext.h>
#include <AzFramework/Network/NetBindingComponent.h>
#include <GridMate/Replica/ReplicaChunk.h>
#include <GridMate/Replica/ReplicaFunctions.h>
#include "Utils/Marshaler.h"
// NetQuery
#include <AzFramework/Network/NetBindingHandlerBus.h>

#include "Components/AbilityComponent.h"
#include "Components/UnitComponent.h"
#include "Components/UnitNavigationComponent.h"
#include "Components/UnitAbilityComponent.h"
#include "Components/UnitAttackComponent.h"
#include "Components/UnitNetSyncComponent.h"
#include "Busses/ShopBus.h"
#include "Busses/LocalizationBus.h"
#include "Components/StaticDataComponent.h"
#include "Components/ModifierComponent.h"
#include "Components/VariableHolderComponent.h"
#include "Components/TagNetSyncComponent.h"
#include "Components/ParticleSystemNetSyncComponent.h"

#include "Busses/UnitBus.h"
#include "Busses/GamePlayerBus.h"
#include "Utils/StringUtils.h"
#include "Utils/FileUtils.h"
#include "Utils/JsonUtils.h"
#include "Utils/Log.h"
#include "Utils/NavigationUtils.h"

#include <AzFramework/Entity/GameEntityContextBus.h>
#include <AzCore/Component/ComponentApplicationBus.h>


#include <AzCore/JSON/stringbuffer.h>
#include <AzCore/JSON/writer.h>

#include <AzCore/std/functional.h>

#include <AzCore/std/containers/vector.h>

#include <PhysicsWrapper/PhysicsWrapperBus.h>

#include <ISystem.h>
#include <CryAction.h>
#include <Cry_Camera.h>
#include <IRenderer.h>
#include <I3DEngine.h>

#include <AzFramework/CommandLine/ConsoleBus.h>
#include <LmbrCentral/Rendering/MaterialOwnerBus.h>


namespace spark
{

//global functions to serialize to the behavior context
int binaryOr(int a, int b)
{
	return a | b;
}

int binaryAnd(int a, int b)
{
	return a & b;
}

int binaryNot(int a)
{
	return ~a;
}

class ConsoleNotificationBusHandler
	: public ConsoleNotificationBus::Handler
	, public AZ::BehaviorEBusHandler
{
public:
	AZ_EBUS_BEHAVIOR_BINDER(ConsoleNotificationBusHandler, "{6A31767C-68FC-4AB9-9209-BBE062EE56A0}", AZ::SystemAllocator, OnCommandFilter);

	bool OnCommandFilter(const char *cmd)
	{
		bool consumed = false;
		CallResult(consumed,FN_OnCommandFilter, cmd);
		return consumed;
	}
};

class GameManagerNotificationBusHandler
	: public GameManagerNotificationBus::Handler
	, public AZ::BehaviorEBusHandler
{
public:
	AZ_EBUS_BEHAVIOR_BINDER(GameManagerNotificationBusHandler, "{85BC464D-27B6-4131-AAB4-78E0945D4D81}", AZ::SystemAllocator,
		OnGameManagerReady, OnGamePhaseChange);

	// used by C++ code to init stuff
	// scripts should use ready event
	// void OnGameManagerActivated (AZ::Entity* gameManager) override
	// {
	// 	Call(OnGameManagerActivated, gameManager->GetId());
	// }

	void OnGameManagerReady (AZ::Entity* gameManager) override
	{
		AZ::EntityId id = gameManager->GetId();
		Call(FN_OnGameManagerReady, id);
	}

	void OnGamePhaseChange (GamePhase::Type newPhase) override
	{
		Call(FN_OnGamePhaseChange, newPhase);
	}
};

class GameManagerReplicaChunk : public GridMate::ReplicaChunkBase
{
public:
	AZ_CLASS_ALLOCATOR(GameManagerReplicaChunk, AZ::SystemAllocator, 0);

	static const char* GetChunkName() { return "GameManagerReplicaChunk"; }

	GameManagerReplicaChunk()
		: m_gamemode("Gamemode")
		, m_gamePhase("Game Phase")
	{
	}

	bool IsReplicaMigratable()
	{
		return true;
	}

	GridMate::DataSet<AZStd::string>::BindInterface<GameManagerComponent, &GameManagerComponent::OnNewGamemode> m_gamemode;
	GridMate::DataSet<uint8>::BindInterface<GameManagerComponent, &GameManagerComponent::OnNewGamePhase> m_gamePhase;
};

void GameManagerComponent::Reflect(AZ::ReflectContext* context)
{
    if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
    {
		serialize->Class<GameManagerComponent, AzFramework::NetBindable, AZ::Component>()
			->Version(0)
			->Field("gamemode", &GameManagerComponent::m_gamemode)
            ;

        if (AZ::EditContext* ec = serialize->GetEditContext())
        {
            ec->Class<GameManagerComponent>("GameManagerComponent", "Game utilities and management")
                ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "spark")
					->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"))
					->DataElement(AZ::Edit::UIHandlers::Default, &GameManagerComponent::m_gamemode, "gamemode name","")
                ;
        }
    }

	if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
	{
		behaviorContext->Method("bor", &binaryOr);
		behaviorContext->Method("band", &binaryAnd);
		behaviorContext->Method("bnot", &binaryNot);

		behaviorContext
			->Enum<(int)spark::GamePhase::INIT>("GAME_PHASE_INIT")
			->Enum<(int)spark::GamePhase::WAITING_FOR_PLAYERS>("GAME_PHASE_WAITING_FOR_PLAYERS")
			->Enum<(int)spark::GamePhase::CONFIGURE_GAME>("GAME_PHASE_CONFIGURE_GAME")
			->Enum<(int)spark::GamePhase::HERO_SELECT>("GAME_PHASE_HERO_SELECT")
			->Enum<(int)spark::GamePhase::PRE_GAME>("GAME_PHASE_PRE_GAME")
			->Enum<(int)spark::GamePhase::GAME>("GAME_PHASE_GAME")
			->Enum<(int)spark::GamePhase::POST_GAME>("GAME_PHASE_POST_GAME")
			;

		behaviorContext->EBus<GameManagerRequestBus>("GameManagerRequestBus")
			->Event("GetGamemode", &GameManagerRequestBus::Events::GetGamemode)
			->Event("GetGamePhase", &GameManagerRequestBus::Events::GetGamePhase)
			->Event("SetGamePhase", &GameManagerRequestBus::Events::SetGamePhase)
			->Event("FinishGameConfigure", &GameManagerRequestBus::Events::FinishGameConfigure)
			->Event("GetActualFilename", &GameManagerRequestBus::Events::GetActualFilename)
			->Event("GetEntitiesHavingTag", &GameManagerRequestBus::Events::GetEntitiesHavingTag)
			->Event("GetEntitiesHavingTags", &GameManagerRequestBus::Events::GetEntitiesHavingTags)
			->Event("GetNearestUnit", &GameManagerRequestBus::Events::GetNearestUnit)
			->Event("GetNearestEnemyUnit", &GameManagerRequestBus::Events::GetNearestEnemyUnit)
			->Event("GetUnitsInsideSphere", &GameManagerRequestBus::Events::GetUnitsInsideSphere)
			->Event("GetNearestShop", &GameManagerRequestBus::Events::GetNearestShop)
			->Event("GetShopsInRange", &GameManagerRequestBus::Events::GetShopsInRange)
			->Event("GetAllShops", &GameManagerRequestBus::Events::GetAllShops)
			->Event("GetTeamIdByName", &GameManagerRequestBus::Events::GetTeamIdByName)
			->Event("GetTeamName", &GameManagerRequestBus::Events::GetTeamName)
			->Event("GetNumberTeams", &GameManagerRequestBus::Events::GetNumberTeams)
			->Event("GetTeamColor", &GameManagerRequestBus::Events::GetTeamColor)
			->Event("SetTeamColor", &GameManagerRequestBus::Events::SetTeamColor)
			->Event("GetDistance", &GameManagerRequestBus::Events::GetDistance)
			->Event("GetForwardDirection", &GameManagerRequestBus::Events::GetForwardDirection)
			->Event("CreateAbility", &GameManagerRequestBus::Events::CreateAbility)
			->Event("CreateItem", &GameManagerRequestBus::Events::CreateItem)
			->Event("CreateModifier", &GameManagerRequestBus::Events::CreateModifier)
			->Event("CreateUnit", &GameManagerRequestBus::Events::CreateUnit)
			->Event("CreateUnitWithJson", &GameManagerRequestBus::Events::CreateUnitWithJson)
			->Event("CreateParticleSystem", &GameManagerRequestBus::Events::CreateParticleSystem)
			->Event("RegisterAbility", &GameManagerRequestBus::Events::RegisterAbility)
			->Event("DestroyEntity", &GameManagerRequestBus::Events::DestroyEntity)
			->Event("GetGameManagerEntity", &GameManagerRequestBus::Events::GetGameManagerEntity)
			->Event("IsServer", &GameManagerRequestBus::Events::IsServer)
			;

		behaviorContext->EBus<InfoRequestBus>("InfoRequestBus")
			->Event("GetGameModeJsonString", &InfoRequestBus::Events::GetGameModeJsonString)
			->Event("GetGameModeName", &InfoRequestBus::Events::GetGameModeName)
			->Event("GetAbilityJsonString", &InfoRequestBus::Events::GetAbilityJsonString)
			->Event("GetItemJsonString", &InfoRequestBus::Events::GetItemJsonString)
			->Event("GetUnitJsonString", &InfoRequestBus::Events::GetUnitJsonString)
			->Event("GetHeroList", &InfoRequestBus::Events::GetHeroList)
			;

		behaviorContext->EBus<AbilitiesNotificationBus>("AbilitiesNotificationBus")
			->Handler<AbilitiesNotificationBusHandler>()
			->Event("OnAbilityCreated", &AbilitiesNotificationBus::Events::OnAbilityCreated);

		behaviorContext->EBus<AbilityTypeNotificationBus>("AbilityTypeNotificationBus")
			->Handler<AbilityTypeNotificationBusHandler>()
			->Event("OnAbilityCreated", &AbilityTypeNotificationBus::Events::OnAbilityCreated);

		behaviorContext->EBus<ConsoleNotificationBus>("ConsoleNotificationBus")
			->Handler<ConsoleNotificationBusHandler>()
			->Event("OnCommandFilter", &ConsoleNotificationBus::Events::OnCommandFilter);

		behaviorContext->EBus<GameManagerNotificationBus>("GameManagerNotificationBus")
			->Handler<GameManagerNotificationBusHandler>()
			->Event("OnGameManagerReady", &GameManagerNotificationBus::Events::OnGameManagerReady)
			->Event("OnGamePhaseChange", &GameManagerNotificationBus::Events::OnGamePhaseChange)
			;
	}

	if (auto netContext = azrtti_cast<AzFramework::NetworkContext*>(context))
	{
		netContext->Class<GameManagerComponent>()
			->Chunk<GameManagerReplicaChunk>()
			->Field("Gamemode", &GameManagerReplicaChunk::m_gamemode)
			->Field("Game Phase", &GameManagerReplicaChunk::m_gamePhase)
			;
	}
}

void GameManagerComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
{
    provided.push_back(AZ_CRC("GameManagerService"));
}

void GameManagerComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
{
    incompatible.push_back(AZ_CRC("GameManagerService"));
}

void GameManagerComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
{
    (void)required;
}

void GameManagerComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
{
    (void)dependent;
}

void GameManagerComponent::Init()
{
}

struct ConsoleListener : public IRemoteConsoleListener
{
    void OnConsoleCommand(const char* cmd) {
		AZ_Printf(0, "OnConsoleCommand : %s", cmd);
	};
    void OnGameplayCommand(const char* cmd) {
		AZ_Printf(0, "OnGameplayCommand : %s", cmd);
	};
};

void GameManagerComponent::Activate()
{
	AZ_Printf(0, "GameManagerComponent::Activate() called");
	InfoRequestBus::Handler::BusConnect();
    GameManagerRequestBus::Handler::BusConnect();
    AZ::TickBus::Handler::BusConnect();

	if (AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		SetGamePhase(GamePhase::WAITING_FOR_PLAYERS);
	}

    // let the fans know we're ready
	EBUS_EVENT(GameManagerNotificationBus, OnGameManagerActivated, GetEntity());
}

void GameManagerComponent::Deactivate()
{
    GameManagerRequestBus::Handler::BusDisconnect();
	InfoRequestBus::Handler::BusDisconnect();
    AZ::TickBus::Handler::BusDisconnect();
}

GridMate::ReplicaChunkPtr GameManagerComponent::GetNetworkBinding ()
{
	AZ_Printf(0, "GameManagerComponent::GetNetworkBinding");
	auto replicaChunk = GridMate::CreateReplicaChunk<GameManagerReplicaChunk>();
	replicaChunk->SetHandler(this);
	m_replicaChunk = replicaChunk;

	if (AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		replicaChunk->m_gamemode.Set(m_gamemode);
		replicaChunk->m_gamePhase.Set(m_gamePhase);
	}
	else
	{
		m_gamemode = replicaChunk->m_gamemode.Get();
		SetGamePhase(static_cast<GamePhase::Type>(replicaChunk->m_gamePhase.Get()));
	}

	return m_replicaChunk;
}

void GameManagerComponent::SetNetworkBinding (GridMate::ReplicaChunkPtr chunk)
{
	AZ_Printf(0, "GameManagerComponent::SetNetworkBinding");
	chunk->SetHandler(this);
	m_replicaChunk = chunk;

	// spoon rabit hat man
	GameManagerReplicaChunk* replicaChunk = static_cast<GameManagerReplicaChunk*>(m_replicaChunk.get());

	if (AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		replicaChunk->m_gamemode.Set(m_gamemode);
		replicaChunk->m_gamePhase.Set(m_gamePhase);
	}
	else
	{
		m_gamemode = replicaChunk->m_gamemode.Get();
		SetGamePhase(static_cast<GamePhase::Type>(replicaChunk->m_gamePhase.Get()));
	}
}

void GameManagerComponent::UnbindFromNetwork ()
{
	m_replicaChunk->SetHandler(nullptr);
	m_replicaChunk = nullptr;
}

void GameManagerComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time)
{
	if (m_gamePhase == GamePhase::INIT)
	{
		return;
	}
	ESystemGlobalState eState = gEnv->pSystem->GetSystemGlobalState();
	if (eState == ESYSTEM_GLOBAL_STATE_RUNNING)
	{
		#if !defined(DEDICATED_SERVER)
		AZ::EntityId localPlayer;
		EBUS_EVENT_RESULT(localPlayer, GameNetSyncRequestBus, GetLocalPlayer);
		if (localPlayer.IsValid())
		#endif
		{
			AZ_Printf(0, "GameManagerComponent::OnGameManagerReady");
		    AZ::TickBus::Handler::BusDisconnect();

			EBUS_EVENT(GameManagerNotificationBus, OnGameManagerReady, GetEntity());
		}
	}
}

void GameManagerComponent::OnNewGamemode (const AZStd::string &gamemode, const GridMate::TimeContext&)
{
	SetGamemode(gamemode);
}

void GameManagerComponent::OnNewGamePhase (const uint8 &gamePhase, const GridMate::TimeContext&)
{
	SetGamePhase(static_cast<GamePhase::Type>(gamePhase));
}

void GameManagerComponent::SetGamemode (const AZStd::string &gamemode)
{
	if (m_gamemode == gamemode)
	{
		return;
	}
	m_gamemode = gamemode;
	if (m_replicaChunk && AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		GameManagerReplicaChunk* replicaChunk = static_cast<GameManagerReplicaChunk*>(m_replicaChunk.get());
		replicaChunk->m_gamemode.Set(m_gamemode);
	}
}

void GameManagerComponent::SetGamePhase (GamePhase::Type gamePhase)
{
	if (m_gamePhase == gamePhase)
	{
		return;
	}
	m_gamePhase = gamePhase;
	AZ_Printf(0, "Game phase is changing, sending event");
	EBUS_EVENT(GameManagerNotificationBus, OnGamePhaseChange, gamePhase);
	if (m_replicaChunk && AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId()))
	{
		GameManagerReplicaChunk* replicaChunk = static_cast<GameManagerReplicaChunk*>(m_replicaChunk.get());
		replicaChunk->m_gamePhase.Set(m_gamePhase);
	}
}

void GameManagerComponent::SetGameJson (rapidjson::Document &document, const AZStd::vector<AZStd::string> &directories)
{
	m_gameDocument.Swap(document);
	m_directories = directories;
}

AZStd::string GameManagerComponent::GetGamemode () const
{
	return m_gamemode;
}

GamePhase::Type GameManagerComponent::GetGamePhase () const
{
	return m_gamePhase;
}

AZStd::string GameManagerComponent::GetActualFilename(AZStd::string hint)
{
	for (auto directory : m_directories)
	{
		AZStd::string attempt = AZStd::string::format("%s/%s",directory.c_str(),hint.c_str());

		if (FileUtils::FileExists(attempt))
		{
			return attempt;
		}
	}
	return "";
}


UnitId GameManagerComponent::GetNearestUnit(AZ::Vector3)
{
	return UnitId();
}

UnitId GameManagerComponent::GetNearestEnemyUnit(UnitId me, AZ::Vector3 target_position)
{
	if (!me.IsValid())return UnitId();

	TeamId myTeam;
	EBUS_EVENT_ID_RESULT(myTeam, me, UnitRequestBus, GetTeamId);



	AZStd::vector<UnitComponent*> units;
	EBUS_EVENT(UnitsRequestBus, GetAllUnitsComponents, units);

	AZ::Vector3 pos;
	float distance,nearest_distance=std::numeric_limits<float>::max();
	UnitId nearest;
	for (auto uc : units) {
		if (uc->GetEntityId() == me)continue;//don't consider self
		if (uc->GetTeamId() == myTeam)continue;//don't consider teammates
		///@todo check if alive

		EBUS_EVENT_ID_RESULT(pos, uc->GetEntityId(), AZ::TransformBus, GetWorldTranslation);
		distance = pos.GetDistance(target_position);
		if (distance < nearest_distance)
		{
			nearest_distance = distance;
			nearest = uc->GetEntityId();
		}
	}

	return nearest;
}

AZStd::vector<UnitId> GameManagerComponent::GetUnitsInsideSphere(AZ::Vector3 center, float radius)
{
	AZ::EBusAggregateResults<AZ::EntityId> allUnits;
	EBUS_EVENT_ID_RESULT(allUnits,  LmbrCentral::Tag("unit"), LmbrCentral::TagGlobalRequestBus, RequestTaggedEntities);

	AZStd::vector<UnitId> result;

	for (auto id : allUnits.values)
	{
		AZ::Vector3 pos;
		EBUS_EVENT_ID_RESULT(pos, id, AZ::TransformBus, GetWorldTranslation);
		if (pos.GetDistance(center) < radius)
		{
			result.push_back(id);
		}
	}
	return result;
}

AZStd::vector<UnitId> GameManagerComponent::GetEntitiesHavingTag(LmbrCentral::Tag tag)
{
	AZ::EBusAggregateResults<AZ::EntityId> entities;
	EBUS_EVENT_ID_RESULT(entities,  tag, LmbrCentral::TagGlobalRequestBus, RequestTaggedEntities);

	return entities.values;
}

AZStd::vector<UnitId> GameManagerComponent::GetEntitiesHavingTags(AZStd::vector<LmbrCentral::Tag> tags)
{
	AZStd::vector<UnitId> result;

	if (tags.empty())
	{
		AZ_Assert(false, "GetEntitiesHavingTags called with no tags");
		return result;
	}

	AZ::EBusAggregateResults<AZ::EntityId> entities;
	EBUS_EVENT_ID_RESULT(entities,  tags[0], LmbrCentral::TagGlobalRequestBus, RequestTaggedEntities);

	for (auto id : entities.values)
	{
		bool ok=true;
		for (int t=1;t<tags.size();++t)
		{
			ok = false;
			EBUS_EVENT_ID_RESULT(ok, id, LmbrCentral::TagComponentRequestBus, HasTag, tags[t]);
			if (!ok)continue;
		}
		if (ok)
		{
			result.push_back(id);
		}
	}
	return result;
}

ShopId GameManagerComponent::GetNearestShop(UnitId unit)
{
	AZStd::vector<AZ::EntityId> shops;
	EBUS_EVENT(ShopsRequestBus, GetAllShops, shops);

	ShopId nearestShop;
	Number minDistance=999999;

	for (auto shop : shops)
	{
		bool in_range = false;
		EBUS_EVENT_ID_RESULT(in_range, shop, ShopRequestBus, IsInRange, unit);
		if (in_range) {
			Number distance=GetDistance(unit, shop);
			if (distance < minDistance)
			{
				minDistance=distance;
				nearestShop = shop;
			}
		}
	}
	return nearestShop;
}

AZStd::vector<ShopId> GameManagerComponent::GetShopsInRange(UnitId unit)
{
	AZStd::vector<ShopId> shops;
	AZStd::vector<ShopId> shops_in_range;
	EBUS_EVENT(ShopsRequestBus, GetAllShops, shops);

	for (auto shop : shops)
	{
		bool in_range = false;
		EBUS_EVENT_ID_RESULT(in_range, shop, ShopRequestBus, IsInRange, unit);
		if (in_range) {
			shops_in_range.push_back(shop);
		}
	}
	return shops_in_range;
}

AZStd::vector<ShopId> GameManagerComponent::GetAllShops()
{
	AZStd::vector<AZ::EntityId> shops;
	EBUS_EVENT(ShopsRequestBus, GetAllShops, shops);
	return shops;
}

TeamId GameManagerComponent::GetTeamIdByName(AZStd::string name)
{
	for (int i = 0; i < m_teamNames.size(); ++i)
	{
		if (name == m_teamNames[i])return i;
	}

	//if the team name is not registered, just register it 
	//todo add a way to register teams ( also using json )
	auto id = m_teamNames.size();

	m_teamNames.push_back(name);

	const ColorF palette[] = { Col_Grey,Col_Green,Col_Red,Col_Yellow,Col_Blue,Col_Orange,Col_Violet };
	m_teamColors.push_back( LYColorFToAZColor(id < 7 ? palette[id] : ColorF(rand()%255, rand() % 255, rand() % 255,255) ));

	return id;

	//return InvalidTeamId;
}

AZStd::string GameManagerComponent::GetTeamName(TeamId id)
{
	return id < m_teamNames.size() ? m_teamNames[id] : "InvalidTeam";
}
int GameManagerComponent::GetNumberTeams()
{
	return m_teamNames.size();
}
void GameManagerComponent::SetTeamColor(TeamId id, AZ::Color color)
{
	if (id < m_teamNames.size())
	{
		m_teamColors[id] = color;
	}
}
AZ::Color GameManagerComponent::GetTeamColor(TeamId id)
{
	return id < m_teamColors.size() ? m_teamColors[id] : AZ::Color(1.0f,1,1,1);
}

float GameManagerComponent::GetDistance(UnitId unitA, UnitId unitB)
{
	AZ::Vector3 a,b;
	EBUS_EVENT_ID_RESULT(a, unitA, NavigationEntityRequestBus, GetPosition);
	EBUS_EVENT_ID_RESULT(b, unitB, NavigationEntityRequestBus, GetPosition);

	return Distance2D(a,b);
}

AZ::Vector3 GameManagerComponent::GetForwardDirection(UnitId unit)
{

	AZ::Quaternion rot;
	EBUS_EVENT_ID_RESULT(rot, unit, AZ::TransformBus, GetLocalRotationQuaternion);
	
	AZ::Vector3 forward(0.0f, 1.0f, 0.0f);

	forward = rot * forward;
	forward.Normalize();
	return forward;
}

//Ability GameManagerComponent::CreateAbility(AbilityId id)
//{
//	auto it = m_abilitiesInfo.find(id);
//
//	if (it == m_abilitiesInfo.end())//Ability is not registered
//	{
//		return Ability();
//	}
//	AbilityInfo &info = it->second;
//
//	AZ::Entity *a = aznew AZ::Entity(id.c_str());
//	if (a) {
//		//todo handle networking stuff (add only network binding?)
//
//		AbilityComponent* abilityComponent = a->CreateComponent<AbilityComponent>();
//		AzFramework::ScriptComponent *scriptComponent = a->CreateComponent<AzFramework::ScriptComponent>();
//
//		const AZ::Data::AssetType& scriptAssetType = azrtti_typeid<AZ::ScriptAsset>();
// 
//		AZ::Data::AssetId assetId;
//		AZStd::string str = AZStd::string::format("%s/scripts/%s",gEnv->pFileIO->GetAlias("@assets@"), info.filename.c_str());
//		CryLog("Script Path: %s", str.c_str());
//		EBUS_EVENT_RESULT(assetId, AZ::Data::AssetCatalogRequestBus, GetAssetIdByPath, str.c_str(), scriptAssetType, true);
// 
//		if (assetId.IsValid()) {
//			CryLog("Setting Script.");
//			AZ::Data::Asset<AZ::ScriptAsset> scriptAsset(assetId, scriptAssetType);
//			scriptComponent->SetScript(scriptAsset);
//		}
//
//
//		a->Init();
//		a->Activate();
//
//		return a->GetId();
//	}
//
//	return Ability();
//}


ItemId GameManagerComponent::CreateItem(ItemTypeId id)
{

	if (!m_gameDocument.HasMember("items") || !m_gameDocument["items"].HasMember(id.c_str()))
	{
		AZ_Error(0, false, "GameManagerComponent::CreateItem  item=\"%s\" undefined", id.c_str());

		//item is not registered
		return ItemId();
	}

	rapidjson::Document info;
	info.CopyFrom(m_gameDocument["items"][id.c_str()],info.GetAllocator());
	rapidjson::Value v;
	v.SetString(id.c_str(), info.GetAllocator());
	info.AddMember("type", v, info.GetAllocator());
	info.AddMember("IsItem", true, info.GetAllocator());

	//make the lua-file always present and always the same for items
	//info.AddMember("lua-file", "Components/item.lua", info.GetAllocator());

	return CreateAbility(info);
}

ModifierId GameManagerComponent::CreateModifier(UnitId caster, AbilityId ability, ModifierTypeId typeId)
{
	//check if modifier is valid 
	//todo check if typeId is registered
	if (typeId.empty())return ModifierId();


	AZ::Entity *m = nullptr;//aznew AZ::Entity(typeId.c_str());
	EBUS_EVENT_RESULT(m, AzFramework::GameEntityContextRequestBus ,CreateGameEntity,typeId.c_str());
	if (m) {
		m->CreateComponent("{65BC817A-ABF6-440F-AD4F-581C40F92795}");
		auto variableComponent = m->CreateComponent<VariableHolderComponent>();
		ModifierComponent* modifierComponent = m->CreateComponent<ModifierComponent>();
		AzFramework::NetBindingComponent *netBindingComponent = m->CreateComponent<AzFramework::NetBindingComponent>();
		auto tsComponent = m->CreateComponent<AzFramework::TransformComponent>();
		// particle effects

		AZ_Assert(variableComponent,   "GameManagerComponent::CreateAbility   invalid VariableHolderComponent");
		AZ_Assert(modifierComponent,   "GameManagerComponent::CreateAbility   invalid ModifierComponent");
		AZ_Assert(netBindingComponent, "GameManagerComponent::CreateAbility   invalid NetBindingComponent");

		modifierComponent->SetModifierTypeId(typeId);
		modifierComponent->SetCaster(caster);
		modifierComponent->SetAbility(ability);

		//set the ability icon as default
		AZStd::string iconTexture;
		EBUS_EVENT_ID_RESULT(iconTexture,ability, AbilityRequestBus, GetIconTexture);
		modifierComponent->SetIconTexture(iconTexture);

		m->Activate();

		EBUS_EVENT(ModifiersNotificationBus, OnModifierCreated, m->GetId(), typeId);
		EBUS_EVENT_ID(typeId,ModifierTypeNotificationBus, OnModifierCreated, m->GetId());

		return m->GetId();
	}

	return ModifierId();
}

AbilityId GameManagerComponent::CreateAbility(AbilityTypeId id)
{

	if (!m_gameDocument.HasMember("abilities") || !m_gameDocument["abilities"].HasMember(id.c_str()))
	{
		AZ_Error(0, false, "GameManagerComponent::CreateAbility  ability=\"%s\" undefined", id.c_str());

		//Ability is not registered
		return AbilityId();
	}

	rapidjson::Document info;
	info.CopyFrom(m_gameDocument["abilities"][id.c_str()],info.GetAllocator());
	rapidjson::Value v;
	v.SetString(id.c_str(), info.GetAllocator());
	info.AddMember("type", v, info.GetAllocator());
	info.AddMember("IsItem", false, info.GetAllocator());

	//make the lua-file always present and always the same for abilities
	//info.AddMember("lua-file", "Components/ability.lua", info.GetAllocator());

	return CreateAbility(info);
}
	

AbilityId GameManagerComponent::CreateAbility(const rapidjson::Value & info)
{
	AZStd::string id = info["type"].GetString();

	AZ::Entity *a = nullptr;// aznew AZ::Entity(id.c_str());
	EBUS_EVENT_RESULT(a, AzFramework::GameEntityContextRequestBus, CreateGameEntity, id.c_str());
	if (a) {
		//todo handle networking stuff (add only network binding?)
		auto variableComponent = a->CreateComponent<VariableHolderComponent>();
		StaticDataComponent* staticDataComponent = a->CreateComponent<StaticDataComponent>();
		AbilityComponent* abilityComponent = a->CreateComponent<AbilityComponent>();
		AzFramework::NetBindingComponent *netBindingComponent = a->CreateComponent<AzFramework::NetBindingComponent>();

		AZ_Assert(variableComponent,   "GameManagerComponent::CreateAbility   invalid VariableHolderComponent");
		AZ_Assert(staticDataComponent, "GameManagerComponent::CreateAbility   invalid StaticDataComponent");
		AZ_Assert(abilityComponent,	   "GameManagerComponent::CreateAbility   invalid AbilityComponent");
		AZ_Assert(netBindingComponent, "GameManagerComponent::CreateAbility   invalid NetBindingComponent");

		staticDataComponent->SetJson(info);
		// staticDataComponent->m_json.CopyFrom(info,staticDataComponent->m_json.GetAllocator());

		if(info.HasMember("name"))abilityComponent->SetAbilityName(info["name"].GetString());
		if(info.HasMember("description"))abilityComponent->SetAbilityDescription(info["description"].GetString());

		abilityComponent->SetAbilityTypeId(id);

		/*if (info.HasMember("lua-file")) {
			AzFramework::ScriptComponent *scriptComponent = a->CreateComponent<AzFramework::ScriptComponent>();

			const AZ::Data::AssetType& scriptAssetType = azrtti_typeid<AZ::ScriptAsset>();

			AZ::Data::AssetId assetId;
			AZStd::string str = AZStd::string::format("scripts/%s", info["lua-file"].GetString());

			CryLog("Script Path: %s", str.c_str());
			EBUS_EVENT_RESULT(assetId, AZ::Data::AssetCatalogRequestBus, GetAssetIdByPath, str.c_str(), scriptAssetType, true);

			if (assetId.IsValid()) {
				CryLog("Setting Script.");
				AZ::Data::Asset<AZ::ScriptAsset> scriptAsset(assetId, scriptAssetType);
				scriptComponent->SetScript(scriptAsset);
			}
		}*/

		

		
		a->Activate();

		int level = 0;	//default level is 0 = not learned, items can be used immediately
		if (info.HasMember("level") && info["level"].IsInt())
		{
			level = info["level"].GetInt();
		}
		else if (info["IsItem"].GetBool())
		{
			level = 1; 
		}

		if(level) abilityComponent->SetLevel(level);

		//set the icon
		if (info.HasMember("icon-file") && info["icon-file"].IsString())abilityComponent->SetIconTexture(info["icon-file"].GetString());
		

		EBUS_EVENT(AbilitiesNotificationBus, OnAbilityCreated, a->GetId(), id);
		EBUS_EVENT_ID(id,AbilityTypeNotificationBus, OnAbilityCreated, a->GetId());


		return a->GetId();
	}

	return AbilityId();
}

void GameManagerComponent::RegisterAbility(AbilityTypeId id, AZStd::string filename)
{
	//trim(filename);
	//rapidjson::Document &document=m_abilitiesInfo[id];

	//if (ends_with(filename, ".lua")) {
	//	AZ_Printf(0,"Registered ability with id:%s and filename:%s", id.c_str(), filename.c_str());

	//	document.SetObject();
	//	document.AddMember("lua-file", filename,document.GetAllocator());
	//}
	//else {
	//	
	//	auto result = ReadTextFile(filename);
	//	if (result.IsSuccess())
 //       {
	//		document.Parse(result.TakeValue().c_str());
 //       }
 //       else
 //       {
 //           // If there is any problem reading the file
 //           AZ_Warning(0, false, "Unable to read engine.json file '%s' (%s).  Defaulting the engine root to '%s'", filename.c_str(), result.GetError().c_str());
 //       }
	//}
}

AZStd::vector<AZStd::string> GameManagerComponent::GetHeroList() const
{
	const rapidjson::Value &units = m_gameDocument["herolist"];
	AZStd::vector<AZStd::string> heroList;

	for (auto unit = units.MemberBegin(); unit != units.MemberEnd(); ++unit)
	{
		if (unit->value.IsNumber() && unit->value.GetDouble() > 0)
		{
			heroList.push_back(unit->name.GetString());
		}
	}

	return heroList;
}

ParticleSystemId GameManagerComponent::CreateParticleSystem(ParticleSystemTypeId typeId)
{
	ParticleSystemId id;


	if (typeId.empty())return id;


	AZ::Entity *e = nullptr;
	EBUS_EVENT_RESULT(e, AzFramework::GameEntityContextRequestBus, CreateGameEntity, typeId.c_str());
	if (e) {

		auto particleSysComponent = e->CreateComponent("{65BC817A-ABF6-440F-AD4F-581C40F92795}");// particle system component
		auto netBindingComponent = e->CreateComponent<AzFramework::NetBindingComponent>();
		auto tsComponent = e->CreateComponent<AzFramework::TransformComponent>();
		auto psNetSyncComponent = e->CreateComponent<ParticleSystemNetSyncComponent>();


		AZ_Assert(tsComponent, "GameManagerComponent::CreateAbility   invalid TransformComponent");
		AZ_Assert(particleSysComponent, "GameManagerComponent::CreateAbility   invalid ModifierComponent");
		AZ_Assert(netBindingComponent, "GameManagerComponent::CreateAbility   invalid NetBindingComponent");
		AZ_Assert(psNetSyncComponent, "GameManagerComponent::CreateAbility   invalid ParticleSystemNetSyncComponent");

		psNetSyncComponent->m_particleSystemTypeId = typeId;

		e->Activate();

		id = e->GetId();

		LmbrCentral::ParticleEmitterSettings particleSettings;
		particleSettings.m_selectedEmitter = typeId;
		particleSettings.m_sizeScale = 1.0f;
		EBUS_EVENT_ID(id, LmbrCentral::ParticleComponentRequestBus, SetupEmitter, particleSettings.m_selectedEmitter, particleSettings);
		EBUS_EVENT_ID(id, LmbrCentral::ParticleComponentRequestBus, Show);
	}
	return id;
}

UnitId GameManagerComponent::CreateUnit(UnitTypeId id)
{
	return CreateUnitWithJson(id, "{}");
}

UnitId GameManagerComponent::CreateUnitWithJson(UnitTypeId id, AZStd::string json)
{
	UnitId unitId;

	if (!m_gameDocument.IsObject() || !m_gameDocument.HasMember("units") || !m_gameDocument["units"].HasMember(id.c_str()))
	{
		AZ_Error(0, false, "GameManagerComponent::CreateUnitWithJson  UnitTypeId=\"%s\" is undefined", id.c_str());

		//Unit is not registered
		return unitId;
	}

	//get the json of the unit type
	const rapidjson::Value &type_info = m_gameDocument["units"][id.c_str()];


	//parse the json for the unit
	rapidjson::Document info;
	if (info.Parse(json.c_str()).HasParseError())
	{
		//if not correctly parsed, return error ( let them know )
		AZ_Error(0, false, "GameManagerComponent::CreateUnitWithJson  json is not well-formatted (offset : %d)", info.GetErrorOffset());
		return unitId;
	}

	//merge the infos
	JsonUtils::MergeObjects(info, type_info, info.GetAllocator());

	//Set essential variables
	rapidjson::Value type;
	type.SetString(id.c_str(), id.size(), info.GetAllocator());
	info.AddMember("type", type, info.GetAllocator());

	if (!info.HasMember("name"))
	{
		rapidjson::Value name;
		name.SetString(id.c_str(), id.size(), info.GetAllocator());
		info.AddMember("name", name, info.GetAllocator());
	}

	//get spawn position
	AZ::Vector3 position(490, 523, 33);
	if (info.HasMember("position"))JsonUtils::ParseVector3(position, info["position"]);
	if (info.HasMember("pos"))JsonUtils::ParseVector3(position, info["pos"]);

	EBUS_EVENT_RESULT(position, MapRequestBus, GetNearestFreePosition, position);

	// AZ_Printf(0, "spawning unit at (%f,%f,%f)", (float)position.GetX(), (float)position.GetY(), (float)position.GetZ());


	//extract and print the portion of json of the unit
	AZStd::string unitJsonString(JsonUtils::ToString(info));
	AZ_Printf(0, "json for unit is:%s", unitJsonString.c_str());


	//spawn the unit:

	AZ::Entity *e = nullptr;// aznew AZ::Entity(id.c_str());
	EBUS_EVENT_RESULT(e, AzFramework::GameEntityContextRequestBus, CreateGameEntity, id.c_str());

	if (e) {
		AzFramework::NetBindingComponent *netBindingComponent = e->CreateComponent<AzFramework::NetBindingComponent>();
		auto transformComponent = e->CreateComponent(AZ::TransformComponentTypeId);
		auto variableComponent = e->CreateComponent<VariableHolderComponent>();
		StaticDataComponent* staticDataComponent = e->CreateComponent<StaticDataComponent>();
		AZ::Uuid tagComponentUuid("{0F16A377-EAA0-47D2-8472-9EAAA680B169}");
		AZ::Component *tagComponent = e->CreateComponent(tagComponentUuid);
		auto tagNetSyncComponent = e->CreateComponent<TagNetSyncComponent>();

		UnitComponent* unitComponent = e->CreateComponent<UnitComponent>();
		UnitAbilityComponent* unitAbility = e->CreateComponent<UnitAbilityComponent>();
		UnitAttackComponent* unitAttack = e->CreateComponent<UnitAttackComponent>();
		UnitNavigationComponent* unitNav = e->CreateComponent<UnitNavigationComponent>();
		UnitNetSyncComponent* unitNetSync = e->CreateComponent<UnitNetSyncComponent>();

		//components for the hitbox
		//auto shapeComponent
		auto shapeComponent = e->CreateComponent(LmbrCentral::CapsuleShapeComponentTypeId);
		auto collisionComponent = e->CreateComponent(AzFramework::PrimitiveColliderComponentTypeId);
		auto physComponent = e->CreateComponent(AzFramework::RigidPhysicsComponentTypeId);
		//auto physComponent = e->CreateComponent(AzFramework::StaticPhysicsComponentTypeId);


		AZ_Assert(transformComponent, "GameManagerComponent::CreateUnit   invalid TransformComponent");
		AZ_Assert(variableComponent, "GameManagerComponent::CreateUnit   invalid VariableHolderComponent");
		AZ_Assert(staticDataComponent, "GameManagerComponent::CreateUnit   invalid StaticDataComponent");
		AZ_Assert(tagComponent, "GameManagerComponent::CreateUnit   invalid TagComponent");
		AZ_Assert(unitComponent, "GameManagerComponent::CreateUnit   invalid unitComponent");
		AZ_Assert(unitAbility, "GameManagerComponent::CreateUnit   invalid UnitAbilityComponent");
		AZ_Assert(unitAttack, "GameManagerComponent::CreateUnit   invalid UnitAttackComponent");
		AZ_Assert(unitNav, "GameManagerComponent::CreateUnit   invalid UnitNavigationComponent");
		AZ_Assert(netBindingComponent, "GameManagerComponent::CreateUnit   invalid NetBindingComponent");
		AZ_Assert(unitNetSync, "GameManagerComponent::CreateUnit   invalid UnitNetSyncComponent");

		AZ_Assert(shapeComponent, "GameManagerComponent::CreateUnit   invalid shapeComponent");
		AZ_Assert(collisionComponent, "GameManagerComponent::CreateUnit   invalid collisionComponent");
		AZ_Assert(physComponent, "GameManagerComponent::CreateUnit   invalid physComponent");

		staticDataComponent->SetJson(info);

		AzFramework::RigidPhysicsConfig config;
		config.m_specifyMassOrDensity = AzFramework::RigidPhysicsConfig::MassOrDensity::Mass;
		config.m_mass = 0.0f;
		physComponent->SetConfiguration(config);

		unitNav->SetBlocking(BlockingFlag::SPAWN_BLOCKING);//unitNav->SetBlocking(BlockingFlag::MOVEMENT_BLOCKING);

		if (info.HasMember("name") && info["name"].IsString())unitComponent->SetName(info["name"].GetString());
		unitComponent->SetUnitTypeId(id);

		/*if (info.HasMember("lua-file")) {
			AzFramework::ScriptComponent *scriptComponent = e->CreateComponent<AzFramework::ScriptComponent>();

			const AZ::Data::AssetType& scriptAssetType = azrtti_typeid<AZ::ScriptAsset>();

			AZ::Data::AssetId assetId;
			AZStd::string str = AZStd::string::format("scripts/%s", info["lua-file"].GetString());

			CryLog("Script Path: %s", str.c_str());
			EBUS_EVENT_RESULT(assetId, AZ::Data::AssetCatalogRequestBus, GetAssetIdByPath, str.c_str(), scriptAssetType, true);

			if (assetId.IsValid()) {
				CryLog("Setting Script.");
				AZ::Data::Asset<AZ::ScriptAsset> scriptAsset(assetId, scriptAssetType);
				scriptComponent->SetScript(scriptAsset);
			}
		}*/

		if (info.HasMember("playerOwner"))
		{
			AZ::EntityId playerEntity;
			auto playerId = info["playerOwner"].GetInt();
			EBUS_EVENT_ID_RESULT(playerEntity, playerId, GamePlayerIdRequestBus, GetPlayerEntity);
			if (!playerEntity.IsValid())
			{
				AZ_Printf(0, "Got an invalid player entity for player id %d", playerId);
			}
			else
			{
				AZ_Printf(0, "Setting player entity unit owner %d", playerId);
				unitComponent->SetPlayerOwner(playerEntity);
			}
		}

		if (
			(info.HasMember("attack-type") && info["attack-type"].IsString() && AZStd::string(info["attack-type"].GetString()) == "ranged")
			|| (info.HasMember("attack_type") && info["attack_type"].IsString() && AZStd::string(info["attack_type"].GetString()) == "ranged")
			)
		{
			unitAttack->SetMelee(false);

			if (info.HasMember("attack-projectile") && info["attack-projectile"].IsString())
			{
				unitAttack->SetProjectileAsset(info["attack-projectile"].GetString());
			}
			else if (info.HasMember("projectile_model") && info["projectile_model"].IsString())
			{
				unitAttack->SetProjectileAsset(info["projectile_model"].GetString());
			}
		}

		
		float collision_radius = 1.0f;

		if (info.HasMember("collision_radius") && info["collision_radius"].IsNumber())
		{
			collision_radius = (float)info["collision_radius"].GetDouble() * MOVEMENT_SCALE;
		}
		sLOG("unit's collision radius is " + collision_radius);

		LmbrCentral::CapsuleShapeConfig shapeConfig;
		shapeConfig.m_radius = collision_radius;
		shapeConfig.m_height = collision_radius * 4;
		shapeComponent->SetConfiguration(shapeConfig);
		/*EBUS_EVENT_ID(unitId, LmbrCentral::CapsuleShapeComponentRequestsBus, SetRadius, collision_radius);
		EBUS_EVENT_ID(unitId, LmbrCentral::CapsuleShapeComponentRequestsBus, SetHeight, collision_radius * 4);*/


		e->Activate();

		unitId = e->GetId();

		//EBUS_EVENT_ID(unitId, AZ::TransformBus, SetWorldTM, AZ::Transform::CreateTranslation(position));
		EBUS_EVENT_ID(unitId, NavigationEntityRequestBus, SetPosition, position);

		
		//initialize variables outside the "variable" field (new json version) -> too many variables? if yes use a white list/black list
		for (auto var = info.MemberBegin(); var != info.MemberEnd(); ++var)
		{
			if (var->value.IsNumber())
			{
				float value = var->value.GetDouble();
				EBUS_EVENT(VariableManagerRequestBus, SetValue, VariableId(unitId, var->name.GetString()), value);
			}
		}
		
		if (info.HasMember("variables"))//initialize variables
		{
			auto &variables = info["variables"];
			for (auto var = variables.MemberBegin(); var != variables.MemberEnd(); ++var)
			{
				if (var->value.IsNumber())
				{
					float value = var->value.GetDouble();
					EBUS_EVENT(VariableManagerRequestBus, SetValue, VariableId(unitId, var->name.GetString()), value);
				}
			}
		}

		{
			for (int i = 0; i < 16; i++)//attach abilities
			{
				AZStd::string field = AZStd::string::format("ability%d", i);
				const char *s = field.c_str();
				if (!info.HasMember(s))
				{
					field = AZStd::string::format("ability_%d", i);
					s = field.c_str();
				}
				if (info.HasMember(s) && info[s].IsString())
				{
					AbilityTypeId type = info[s].GetString();
					if (!type.empty())
					{
						unitAbility->SetAbilityInSlot(Slot(Slot::Ability, i), CreateAbility(type));
					}
				}
			}

			for (int i = 0; i < 16; i++)//attach items
			{
				AZStd::string field = AZStd::string::format("item%d", i);
				const char *s = field.c_str();
				if (info.HasMember(s) && info[s].IsString())
				{
					unitAbility->SetAbilityInSlot(Slot(Slot::Inventory, i), CreateItem(info[s].GetString()));
				}
			}
		}

		if (info.HasMember("team"))
		{
			auto &team = info["team"];
			TeamId teamId = InvalidTeamId;
			if (team.IsNumber())
			{
				teamId = team.GetInt();
			}
			if (team.IsString())
			{
				teamId = GetTeamIdByName(team.GetString());
			}
			if (teamId != InvalidTeamId)
			{
				EBUS_EVENT_ID(unitId, UnitRequestBus, SetTeamId, teamId);
			}
		}

		//EBUS_EVENT_ID(ticket, GameManagerSliceNotificationBus, OnUnitCreated, unitId);
		EBUS_EVENT(UnitsNotificationBus, OnUnitCreated, unitId, id);


		//find the slice asset
		{
			AZ::Data::AssetId assetId;
			AZStd::string str = AZStd::string::format("slices/%s.dynamicslice", info.HasMember("slice") ? info["slice"].GetString() : "hero_test_001");//AZStd::string::format("%s/Slices/%s.dynamicslice", gEnv->pFileIO->GetAlias("@assets@"), info.HasMember("slice") ? info["slice"].GetString() : "hero_test_001");

			EBUS_EVENT_RESULT(assetId, AZ::Data::AssetCatalogRequestBus, GetAssetIdByPath, str.c_str(), AZ::AzTypeInfo<AZ::DynamicPrefabAsset>::Uuid(), true);

			if (!assetId.IsValid())
			{
				AZ_Error(0, false, "cannot instantiate slice asset");
				return unitId;
			}
			//load the dynamic slice and initialize it
			AZ::Data::Asset<AZ::DynamicPrefabAsset> dynamicSliceAsset;
			dynamicSliceAsset.Create(assetId, true);

			m_slices.emplace_back(dynamicSliceAsset);//prevents the wrapper to get destroyed and not calling the callbacks
			DynamicSliceWrapper &d = m_slices.back();

			d.onInstantiated =
				[unitId, str](AZ::Entity* e)
			{
				//sLOG("dynamic slice OnInstantiated : " + str + "  id:" + e->GetId());

				//AZ::Vector3 scale = AZ::Vector3::CreateOne();
				//EBUS_EVENT_ID_RESULT(scale, e->GetId(), AZ::TransformBus, GetLocalScale);

				//EBUS_EVENT_ID(e->GetId(), AZ::TransformBus, SetWorldTM, AZ::Transform::CreateIdentity());
				EBUS_EVENT_ID(e->GetId(), AZ::TransformBus, SetParentRelative, unitId);

				//EBUS_EVENT_ID(unitId, AZ::TransformBus, SetLocalScale, scale);


				//EBUS_EVENT_ID(unitId, AzFramework::ColliderComponentEventBus, OnColliderChanged);

				AZ::Vector3 position;
				EBUS_EVENT_ID_RESULT(position, unitId, NavigationEntityRequestBus, GetPosition);
				EBUS_EVENT_ID(unitId, NavigationEntityRequestBus, SetPosition, position);

				EBUS_EVENT_ID(unitId, UnitRequestBus, UpdateVisibility, true);
			};

			d.Load();
		}
	}

	return unitId;
}
//AzFramework::SliceInstantiationTicket GameManagerComponent::CreateUnit(UnitTypeId id)
//{
//	return CreateUnitWithJson(id, "{}");
//}
//
//AzFramework::SliceInstantiationTicket GameManagerComponent::CreateUnitWithJson(UnitTypeId id, AZStd::string json)
//{
//	AzFramework::SliceInstantiationTicket ticket;
//
//	if (!m_gameDocument.IsObject() || !m_gameDocument.HasMember("units") || !m_gameDocument["units"].HasMember(id.c_str()))
//	{
//		AZ_Error(0, false, "GameManagerComponent::CreateUnitWithJson  UnitTypeId=\"%s\" is undefined", id.c_str());
//
//		//Unit is not registered
//		return ticket;
//	}
//
//	//get the json of the unit type
//	const rapidjson::Value &type_info = m_gameDocument["units"][id.c_str()];
//
//
//	//parse the json for the unit
//	rapidjson::Document info;
//	if (info.Parse(json.c_str()).HasParseError())
//	{
//		//if not correctly parsed, return error ( let them know )
//		AZ_Error(0, false, "GameManagerComponent::CreateUnitWithJson  json is not well-formatted (offset : %d)", info.GetErrorOffset());
//		return ticket;
//	}
//
//	//merge the infos
//	JsonUtils::MergeObjects(info,type_info, info.GetAllocator());
//
//	rapidjson::Value name;
//	name.SetString(id.c_str(), id.size(),info.GetAllocator());
//
//	//Set essential variables
//	info.AddMember("type", name, info.GetAllocator());
//	info.AddMember("name", name, info.GetAllocator());
//
//	//find the slice asset
//	AZ::Data::AssetId assetId;
//	AZStd::string str = AZStd::string::format("slices/%s.dynamicslice", info.HasMember("slice") ? info["slice"].GetString() : "hero_test_001");//AZStd::string::format("%s/Slices/%s.dynamicslice", gEnv->pFileIO->GetAlias("@assets@"), info.HasMember("slice") ? info["slice"].GetString() : "hero_test_001");
//
//	EBUS_EVENT_RESULT(assetId, AZ::Data::AssetCatalogRequestBus, GetAssetIdByPath, str.c_str(), AZ::AzTypeInfo<AZ::DynamicPrefabAsset>::Uuid(), true);
//
//	if (!assetId.IsValid()) 
//	{
//		AZ_Error(0, false, "cannot instantiate slice asset");
//		return ticket;
//	}
//	
//
//	//load the dynamic slice and initialize it
//	AZ::Data::Asset<AZ::DynamicPrefabAsset> dynamicSliceAsset;
//	dynamicSliceAsset.Create(assetId, true);
//
//	//get spawn position
//	AZ::Vector3 position(490, 523, 33);
//	if(info.HasMember("position"))JsonUtils::ParseVector3(position,info["position"]);
//	if(info.HasMember("pos"))JsonUtils::ParseVector3(position,info["pos"]);
//
//	EBUS_EVENT_RESULT(position, MapRequestBus, GetNearestFreePosition, position);
//
//	// AZ_Printf(0, "spawning unit at (%f,%f,%f)", (float)position.GetX(), (float)position.GetY(), (float)position.GetZ());
//	AZ::Transform transform=AZ::Transform::CreateTranslation(position);
//	
//
//	//extract and print the portion of json of the unit
//	AZStd::string unitJsonString(JsonUtils::ToString(info));
//	AZ_Printf(0, "json for unit is:%s", unitJsonString.c_str());
//
//
//	m_slices.emplace_back(dynamicSliceAsset);//prevents the wrapper to get destroyed and not calling the callbacks
//	DynamicSliceWrapper &d=m_slices.back();
//	ticket = d.GetInstantiationTicket();
//	d.transform=transform;
//
//	d.onPreInstantiate=
//		[unitJsonString, id](AZ::Entity* e) 
//		{
//			StaticDataComponent* s=e->FindComponent<StaticDataComponent>();
//			if (!s)
//			{
//				AZ_Printf(0,"OnSlicePreInstantiate Unit is missing StaticDataComponent, cannot add components to slices during pre instantiate!: %s", id.c_str());
//				s=e->CreateComponent<StaticDataComponent>();
//			}
//
//			AZ_Assert(s, "GameManagerComponent::CreateUnit   invalid StaticDataComponent");
//			s->SetJson(unitJsonString);
//
//			AZ::Uuid tagComponentUuid("{0F16A377-EAA0-47D2-8472-9EAAA680B169}");
//			AZ::Component *tagComponent = e->FindComponent(tagComponentUuid);
//			if(!tagComponent)
//			{
//				AZ_Printf(0,"OnSlicePreInstantiate Unit is missing TagComponent, cannot add components to slices during pre instantiate!: %s", id.c_str());
//				tagComponent=e->CreateComponent(tagComponentUuid);
//			}
//			AZ_Assert(tagComponent, "GameManagerComponent::CreateUnit   invalid TagComponent");
//
//
//			if (s->GetJsonReference().HasMember("lua-file")) {
//				AzFramework::ScriptComponent *scriptComponent = e->FindComponent<AzFramework::ScriptComponent>();
//				
//				if(!scriptComponent)
//				{
//					AZ_Printf(0,"OnSlicePreInstantiate Unit is missing ScriptComponent, cannot add components to slices during pre instantiate!: %s", id.c_str());
//					scriptComponent=e->CreateComponent<AzFramework::ScriptComponent>();
//				}
//
//				AZ_Assert(scriptComponent, "GameManagerComponent::CreateUnitWithJson   invalid scriptComponent");
//
//				const AZ::Data::AssetType& scriptAssetType = azrtti_typeid<AZ::ScriptAsset>();
//
//				AZ::Data::AssetId assetId;
//				AZStd::string str = AZStd::string::format("scripts/%s", s->GetJsonReference()["lua-file"].GetString());
//
//				EBUS_EVENT_RESULT(assetId, AZ::Data::AssetCatalogRequestBus, GetAssetIdByPath, str.c_str(), scriptAssetType, true);
//
//				if (assetId.IsValid()) {
//					AZ::Data::Asset<AZ::ScriptAsset> scriptAsset(assetId, scriptAssetType);
//					scriptComponent->SetScript(scriptAsset);
//				}
//				else
//				{
//					sERROR("failed to load the script asset at " + str);
//				}
//			}
//
//
//			e->InvalidateDependencies();
//			
//		};
//	d.onInstantiated=
//		[unitJsonString, id, ticket, this](AZ::Entity* e) 
//		{
//			EBUS_EVENT_ID(e->GetId(), LmbrCentral::TagComponentRequestBus, AddTag, LmbrCentral::Tag("unit"));
//			bool ok=false;
//			EBUS_EVENT_ID_RESULT(ok,e->GetId(), LmbrCentral::TagComponentRequestBus, HasTag, LmbrCentral::Tag("unit"));
//			AZ_Assert(ok, "GameManagerComponent::CreateUnit  TagComponent not present");
//
//			StaticDataComponent* s=e->FindComponent<StaticDataComponent>();
//			const rapidjson::Value& info = s->GetJsonReference();
//
//			auto *uc=e->FindComponent<UnitComponent>();
//
//			uc->SetUnitTypeId(id);
//			uc->SetName(id);
//			if (info.HasMember("playerOwner"))
//			{
//				AZ::EntityId playerEntity;
//				auto playerId = info["playerOwner"].GetInt();
//				EBUS_EVENT_ID_RESULT(playerEntity, playerId, GamePlayerIdRequestBus, GetPlayerEntity);
//				if (!playerEntity.IsValid())
//				{
//					AZ_Printf(0, "Got an invalid player entity for player id %d", playerId);
//				}
//				else
//				{
//					AZ_Printf(0, "Setting player entity unit owner %d", playerId);
//					uc->SetPlayerOwner(playerEntity);
//				}
//			}
//
//			if (info.HasMember("variables"))//initialize variables
//			{
//				auto &variables = info["variables"];
//				for (auto var = variables.MemberBegin(); var != variables.MemberEnd(); ++var)
//				{
//					if (var->value.IsNumber()) 
//					{
//						float value = var->value.GetDouble();
//						EBUS_EVENT(VariableManagerRequestBus, SetValue, VariableId(e->GetId(),var->name.GetString()), value);
//					}
//				}
//			}
//
//			if (UnitAbilityComponent* unitAbility = e->FindComponent<UnitAbilityComponent>()) {
//				for (int i = 0; i < 16; i++)//attach abilities
//				{
//					AZStd::string field = AZStd::string::format("ability%d", i);
//					const char *s = field.c_str();
//					if (info.HasMember(s) && info[s].IsString())
//					{
//						unitAbility->SetAbilityInSlot(Slot(Slot::Ability, i), CreateAbility(info[s].GetString()));
//					}
//				}
//
//				for (int i = 0; i < 16; i++)//attach items
//				{
//					AZStd::string field = AZStd::string::format("item%d", i);
//					const char *s = field.c_str();
//					if (info.HasMember(s) && info[s].IsString())
//					{
//						unitAbility->SetAbilityInSlot(Slot(Slot::Inventory, i), CreateItem(info[s].GetString()));
//					}
//				}
//			}
//
//			if (info.HasMember("tags") && info["tags"].IsArray())//initialize tags
//			{
//				auto &tags = info["tags"];
//				for (auto tag = tags.Begin(); tag != tags.End(); ++tag)
//				{
//					if (tag->IsString()) 
//					{
//						EBUS_EVENT_ID(e->GetId(), LmbrCentral::TagComponentRequestBus, AddTag, LmbrCentral::Tag(tag->GetString()));
//					}
//				}
//
//			}
//
//			auto entityId = e->GetId();
//
//			if (info.HasMember("team"))
//			{
//				auto &team = info["team"];
//				TeamId teamId = InvalidTeamId;
//				if (team.IsNumber())
//				{
//					teamId = team.GetInt();
//				}
//				if (team.IsString())
//				{
//					teamId = GetTeamIdByName(team.GetString());
//				}
//				if (teamId != InvalidTeamId)
//				{
//					EBUS_EVENT_ID(entityId, UnitRequestBus, SetTeamId, teamId);
//				}
//			}
//
//			EBUS_EVENT_ID(ticket, GameManagerSliceNotificationBus, OnUnitCreated, entityId);
//			EBUS_EVENT(UnitsNotificationBus, OnUnitCreated, entityId, id);
//
//			ok = false;
//			EBUS_EVENT_ID_RESULT(ok,entityId, LmbrCentral::MaterialOwnerRequestBus, IsMaterialOwnerReady);
//			sWARNING("IsMaterialOwnerReady:" + ok);
//
//			EBUS_EVENT_ID(entityId, LmbrCentral::MaterialOwnerRequestBus, SetMaterialParamColor, "diffuse", AZ::Color(1.0f, 0.0f, 0.f, 1.0f),1);
//		};
//
//	d.Load();
//
//	return ticket;
//}




//Unit GameManagerComponent::CreateUnit(UnitId id)
//{
//
//	if (!m_gameDocument.HasMember("units") || !m_gameDocument["units"].HasMember(id.c_str()))
//	{
//		AZ_Error(0, false, "GameManagerComponent::CreateUnit  unit=\"%s\" is undefined", id.c_str());
//
//		//Unit is not registered
//		return Unit();
//	}
//
//	rapidjson::Value &info = m_gameDocument["units"][id.c_str()];
//
//	rapidjson::StringBuffer buffer;
//	buffer.Clear();
//	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
//	info.Accept(writer);
//	AZ_Printf(0, "json for unit is:%s",buffer.GetString());
//	buffer.Clear();
//
//	AZ::Entity *e = aznew AZ::Entity(id.c_str());
//	if (e) {
//		//todo handle networking stuff (add only network binding?)
//
//		UnitComponent* unitComponent = e->CreateComponent<UnitComponent>();
//		UnitAbilityComponent* unitAbility = e->CreateComponent<UnitAbilityComponent>();
//		UnitAttackComponent* unitAttack = e->CreateComponent<UnitAttackComponent>();
//		UnitNavigationComponent* unitNav = e->CreateComponent<UnitNavigationComponent>();
//		UnitNetSyncComponent* unitNet = e->CreateComponent<UnitNetSyncComponent>();
//		e->CreateComponent<AzFramework::NetBindingComponent>();
//
//		for (int i = 0; i < 16; i++)
//		{
//			AZStd::string field = AZStd::string::format("ability%d", i);
//			const char *s = field.c_str();
//			if (info.HasMember(s) && info[s].IsString())
//			{
//				unitAbility->SetAbilityInSlot(Slot(Slot::Ability,i),CreateAbility(info[s].GetString()));
//			}
//		}
//
//		for (int i = 0; i < 16; i++)
//		{
//			AZStd::string field = AZStd::string::format("item%d", i);
//			const char *s = field.c_str();
//			if (info.HasMember(s) && info[s].IsString())
//			{
//				unitAbility->SetAbilityInSlot(Slot(Slot::Inventory,i),CreateAbility(info[s].GetString()));
//			}
//		}
//
//		if (info.HasMember("lua-file")) {
//			AzFramework::ScriptComponent *scriptComponent = e->CreateComponent<AzFramework::ScriptComponent>();
//
//			const AZ::Data::AssetType& scriptAssetType = azrtti_typeid<AZ::ScriptAsset>();
// 
//			AZ::Data::AssetId assetId;
//			AZStd::string str = AZStd::string::format("%s/scripts/%s",gEnv->pFileIO->GetAlias("@assets@"), info["lua-file"].GetString());
//
//			CryLog("Script Path: %s", str.c_str());
//			EBUS_EVENT_RESULT(assetId, AZ::Data::AssetCatalogRequestBus, GetAssetIdByPath, str.c_str(), scriptAssetType, true);
// 
//			if (assetId.IsValid()) {
//				CryLog("Setting Script.");
//				AZ::Data::Asset<AZ::ScriptAsset> scriptAsset(assetId, scriptAssetType);
//				scriptComponent->SetScript(scriptAsset);
//			}
//		}
//
//		e->Init();
//		e->Activate();
//
//		return e->GetId();
//	}
//
//	return Unit();
//}

void GameManagerComponent::RegisterUnit(UnitTypeId, AZStd::string filename)
{
}

bool GameManagerComponent::IsServer()
{
	return AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId());
}

AZStd::string GameManagerComponent::GetAbilityJsonString(AbilityTypeId id)
{
	if (!m_gameDocument.IsObject() || !m_gameDocument.HasMember("abilities") || !m_gameDocument["abilities"].HasMember(id.c_str()))
	{
		return "";
	}

	return JsonUtils::ToString(m_gameDocument["abilities"][id.c_str()]);
}

AZStd::string GameManagerComponent::GetGameModeJsonString()
{
	return JsonUtils::ToString(m_gameDocument);
}

AZStd::string GameManagerComponent::GetGameModeName()
{
	if (m_gameDocument.IsObject() && m_gameDocument.HasMember("gamemode") && m_gameDocument["gamemode"].HasMember("name") && m_gameDocument["gamemode"]["name"].IsString())
	{
		return JsonUtils::ToString(m_gameDocument["gamemode"]["name"]);
	}

	return AZStd::string();	
}

AZStd::string GameManagerComponent::GetItemJsonString(ItemTypeId id)
{
	if (!m_gameDocument.IsObject() || !m_gameDocument.HasMember("items") || !m_gameDocument["items"].HasMember(id.c_str()))
	{
		return "";
	}

	return JsonUtils::ToString(m_gameDocument["items"][id.c_str()]);
}

AZStd::string GameManagerComponent::GetUnitJsonString(UnitTypeId id)
{
	if (!m_gameDocument.IsObject() || !m_gameDocument.HasMember("units") || !m_gameDocument["units"].HasMember(id.c_str()))
	{
		return "";
	}

	return JsonUtils::ToString(m_gameDocument["units"][id.c_str()]);
}

rapidjson::Value* GameManagerComponent::GetGameModeJson()
{
	if (!m_gameDocument.IsObject())
	{
		return nullptr;
	}

	return &m_gameDocument;
}
rapidjson::Value* GameManagerComponent::GetAbilityJson(AbilityTypeId id)
{
	if (!m_gameDocument.IsObject() || !m_gameDocument.HasMember("abilities") || !m_gameDocument["abilities"].HasMember(id.c_str()))
	{
		return nullptr;
	}

	return &m_gameDocument["abilities"][id.c_str()];
}
rapidjson::Value* GameManagerComponent::GetItemJson(ItemTypeId id)
{
	if (!m_gameDocument.IsObject() || !m_gameDocument.HasMember("items") || !m_gameDocument["items"].HasMember(id.c_str()))
	{
		return nullptr;
	}

	return &m_gameDocument["items"][id.c_str()];
}
rapidjson::Value* GameManagerComponent::GetUnitJson(UnitTypeId id)
{
	if (!m_gameDocument.IsObject() || !m_gameDocument.HasMember("units") || !m_gameDocument["units"].HasMember(id.c_str()))
	{
		return nullptr;
	}

	return &m_gameDocument["units"][id.c_str()];
}


void GameManagerComponent::StartGame()
{
	AZ_Printf(0,"GameManagerComponent::StartGame() called");
}

void GameManagerComponent::DestroyEntity(AZ::EntityId id)
{
	sLOG("destroying entity :" + id);
	EBUS_EVENT(AzFramework::GameEntityContextRequestBus, DestroyGameEntity, id);
}

void GameManagerComponent::FinishGameConfigure ()
{
	if (m_gamePhase != GamePhase::CONFIGURE_GAME)
	{
		return;
	}
	SetGamePhase(GamePhase::HERO_SELECT);
}

AZ::EntityId GameManagerComponent::GetGameManagerEntity ()
{
	return GetEntityId();
}

}
