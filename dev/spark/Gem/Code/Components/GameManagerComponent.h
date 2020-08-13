#pragma once

#include "Busses/GameManagerBus.h"
#include "Busses/InfoBus.h"
#include "Busses/MapBus.h"

#include <AzCore/std/containers/map.h>
#include <AZCore/JSON/document.h>
#include <AZCore/Component/TickBus.h>
#include <AzFramework/Network/NetBindable.h>

#include <Utils/DynamicSliceWrapper.h>
#include <AzCore/std/containers/vector.h>
#include <GridMate/Session/Session.h>

namespace spark
{
	class GameManagerSystemComponent;

	class GameManagerComponent
		: public AZ::Component
		, public GameManagerRequestBus::Handler
		, public InfoRequestBus::Handler
		, public AzFramework::NetBindable
		, public AZ::TickBus::Handler
	{
	public:

		AZ_COMPONENT(GameManagerComponent, "{6DC26025-E793-43B0-B427-F9D608719600}", AzFramework::NetBindable);

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

		GridMate::ReplicaChunkPtr GetNetworkBinding ();
		void SetNetworkBinding (GridMate::ReplicaChunkPtr chunk);
		void UnbindFromNetwork ();
		void OnNewGamemode (const AZStd::string &gamemode, const GridMate::TimeContext&);
		void OnNewGamePhase (const uint8 &gamePhase, const GridMate::TimeContext&);

		void SetGamemode (const AZStd::string &gamemode);
		void SetGamePhase (GamePhase::Type gamePhase) override;

		void SetGameJson (rapidjson::Document &document, const AZStd::vector<AZStd::string> &directories);

		void StartGame();

		AZStd::string GetGamemode () const override;
		GamePhase::Type GetGamePhase () const override;

    protected:
        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////

		void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

		//GameManagerRequestBus
		bool GameManagerAlreadyExists() override
		{
			return true;
		}
		bool IsServer() override;

		AZStd::string GetActualFilename(AZStd::string hint) override;

		UnitId	GetNearestUnit(AZ::Vector3) override;
		UnitId  GetNearestEnemyUnit(UnitId me, AZ::Vector3) override;

		AZStd::vector<UnitId> GetUnitsInsideSphere(AZ::Vector3 center, float radius) override;
		AZStd::vector<UnitId> GetEntitiesHavingTag(LmbrCentral::Tag) override;
		AZStd::vector<UnitId> GetEntitiesHavingTags(AZStd::vector<LmbrCentral::Tag>) override;

		ShopId GetNearestShop(UnitId) override;
		AZStd::vector<ShopId> GetShopsInRange(UnitId) override;
		AZStd::vector<ShopId> GetAllShops() override;

		TeamId GetTeamIdByName(AZStd::string name) override;
		AZStd::string GetTeamName(TeamId) override;
		int GetNumberTeams() override;
		void SetTeamColor(TeamId, AZ::Color) override;
		AZ::Color GetTeamColor(TeamId) override;

		float   GetDistance(UnitId, UnitId) override;
		AZ::Vector3 GetForwardDirection(UnitId unit) override;

		
		void RegisterUnit(UnitTypeId, AZStd::string filename) override; //deprecated
		void RegisterAbility(AbilityTypeId, AZStd::string filename) override; //deprecated

		AbilityId CreateAbility(AbilityTypeId) override;
		ItemId    CreateItem(ItemTypeId) override;
		ModifierId CreateModifier(UnitId, AbilityId, ModifierTypeId) override;

		ParticleSystemId CreateParticleSystem(ParticleSystemTypeId) override;

		UnitId CreateUnit(UnitTypeId) override;
		UnitId CreateUnitWithJson(UnitTypeId, AZStd::string json) override;


		void DestroyEntity(AZ::EntityId) override;

		void FinishGameConfigure() override;

		AZ::EntityId GetGameManagerEntity() override;

		//utility functions:
		AbilityId CreateAbility(const rapidjson::Value &info);


		//InfoRequestBus
		AZStd::string GetGameModeJsonString();
		AZStd::string GetGameModeName();

		AZStd::string GetItemJsonString(ItemTypeId);
		AZStd::string GetAbilityJsonString(AbilityTypeId);
		AZStd::string GetUnitJsonString(UnitTypeId);

		rapidjson::Value* GetGameModeJson();
		rapidjson::Value* GetAbilityJson(AbilityTypeId);
		rapidjson::Value* GetItemJson(ItemTypeId);
		rapidjson::Value* GetUnitJson(UnitTypeId);

		AZStd::vector<AZStd::string> GetHeroList() const;
		
	private:
		GridMate::ReplicaChunkPtr m_replicaChunk;

		AZStd::vector<DynamicSliceWrapper> m_slices;

		AZStd::string m_gamemode;
		GamePhase::Type m_gamePhase = GamePhase::INIT;

		rapidjson::Document m_gameDocument;

		AZStd::vector<AZStd::string> m_directories;

		AZStd::vector<AZStd::string> m_teamNames;
		AZStd::vector<AZ::Color> m_teamColors;
	};


}