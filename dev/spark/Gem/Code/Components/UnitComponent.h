#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzFramework/Entity/GameEntityContextBus.h>
#include <AzFramework/Network/NetBindable.h>

#include <AzCore/std/containers/list.h>
#include "Busses/UnitBus.h"
#include "Utils/UnitOrder.h"
#include "Busses/UnitNavigationBus.h"
#include "Utils/SSM.h"
#include "Busses/GameplayBusses.h"
#include "Busses/UnitAbilityBus.h"
#include "Busses/UnitAttackBus.h"
#include "Busses/LocalizationBus.h"
#include "Busses/MapBus.h"

namespace spark
{
	class UnitComponent
		: public AZ::Component
		, public AZ::TickBus::Handler
		, public UnitRequestBus::Handler
		, public UnitsRequestBus::Handler
		, public UnitNavigationNotificationBus::Handler
		, public OnDamageTakenRequestBus::Handler
		, public UnitAttackNotificationBus::Handler
		, public UnitAbilityNotificationBus::Handler
		, public LocalizationNotificationBus::Handler
		, public AzFramework::NetBindable
		, protected AzFramework::GameEntityContextEventBus::Handler
	{
	public:
		AZ_COMPONENT(UnitComponent, "{DB43AF88-B16F-4C75-A549-0428D9C530DE}", AzFramework::NetBindable)

		~UnitComponent() override {};

		static void Reflect(AZ::ReflectContext* reflection);

		void Init() override;
		void Activate() override;
		void Deactivate() override;

		//NetBindable
		GridMate::ReplicaChunkPtr GetNetworkBinding ();
		void SetNetworkBinding (GridMate::ReplicaChunkPtr chunk);
		void UnbindFromNetwork ();
		// handlers
		void OnNewName (const AZStd::string& name, const GridMate::TimeContext& tc);
		void OnNewTeamId (const TeamId& teamId, const GridMate::TimeContext& tc);
		void OnNewQuickBuyItemList (const AZStd::vector<ItemTypeId>& quickBuyItemList, const GridMate::TimeContext& tc);
		void OnNewModifiers (const AZStd::vector<ModifierId>& modifiers, const GridMate::TimeContext& tc);
		void OnNewStatus (const Status& status, const GridMate::TimeContext& tc);
		void OnNewPlayerOwner (const AZ::EntityId& playerOwner, const GridMate::TimeContext& tc);
		void OnNewIsAlive (const bool& isAlive, const GridMate::TimeContext& tc);

		// metadata getters
		bool IsSelectable();
		bool IsFriendly();
		bool IsControllable();

		// for server side verification of player gestures
		// selection is a 100% client side concept
		// bool IsSelectable(const AZ::EntityId &playerId);
		// friendliness is not because netcode should be enforcing this on the back-end
		bool IsFriendly(const AZ::EntityId &playerId);
		bool IsControllable(const AZ::EntityId &playerId);

		bool IsAlive();
		void SetAlive(bool alive);

		AZStd::string GetName();
		void SetName(AZStd::string);

		UnitTypeId GetUnitTypeId();
		void SetUnitTypeId(UnitTypeId);

		AZ::EntityId GetPlayerOwner() override;
		void SetPlayerOwner(AZ::EntityId);

		TeamId GetTeamId();
		void   SetTeamId(TeamId);

		void SetQuickBuyItemList(AZStd::vector<ItemTypeId> list);
		AZStd::vector<ItemTypeId> GetQuickBuyItemList();

		bool IsVisible();
		void SetVisible(bool visible);

		void UpdateVisibility(bool force_update = false);

		// Orders
		UnitOrder GetCurrentOrder() const;
		void NewOrder(UnitOrder c,bool queue=false);

		//modifiers
		AZStd::vector<ModifierId> GetModifiers();
		void SetModifiers(AZStd::vector<ModifierId>);
		void ClearModifiers();
		void AddModifier(ModifierId);
		void RemoveModifier(ModifierId);
		ModifierId FindModifierByTypeId(ModifierTypeId);

		void ApplyDispel(Dispel) override;

		//status
		Status GetStatus() override;
		void SetStatus(Status) override;
		void Kill (Damage damage) override;
		void KillWithAbility (AZ::EntityId, AZ::EntityId) override;

		AZStd::string ToString() override;
		void Destroy() override;

	protected:
		void GetAllUnits(AZStd::vector<AZ::EntityId> &result);
		void GetAllUnitsComponents(AZStd::vector<UnitComponent*> &result);

		void GetUnits(AZStd::vector<AZ::EntityId> &result, const UnitsFilter &filter);
		void GetUnitsEntities(AZStd::vector<AZ::Entity*> &result, const UnitsEntitiesFilter &filter);
		

		void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

		void OnMovingDone() override;
		void OnCastFinished() override;
		void OnAttackingInterrupted() override;

		//Orders 
		void OrderDone();
		void ExecuteCurrentOrder();
		AZStd::list<UnitOrder> m_ordersList;

		//State
		SSM m_stateMachine;
		AZ::EntityId m_target;
		AZ::Vector3 m_targetPostion;
		float m_followDistance;

		//localization 
		void OnLanguageChanged(const AZStd::string &language);

		enum Event{
			E_TRAVERSAL_COMPLETE,
			E_CAST_FINISHED,
			E_ATTACK_INTERRUPTED,
			E_UPDATE,
			E_SET_TARGET
		};

		enum State {
			S_IDLE,
			S_MOVE,
			S_FOLLOW,
			S_ATTACK,
			S_CAST,  
		}m_state=S_IDLE;

		bool OnStateIdle(SSM& hsm, const SSM::Event& e);
		bool OnStateMove(SSM& hsm, const SSM::Event& e);
		bool OnStateAttack(SSM& hsm, const SSM::Event& e);
		bool OnStateFollow(SSM& hsm, const SSM::Event& e);
		bool OnStateCast(SSM& hsm, const SSM::Event& e);

		//attributes
		AZStd::string m_name;
		UnitTypeId m_typeId;
		TeamId m_teamId;
		AZ::EntityId m_playerOwner;

		bool m_visible = true;
		char m_visible_pre = -1;

		AZStd::vector<ItemTypeId> m_quickBuyItemList;

		AZStd::vector<ModifierId> m_modifiers;

		Status m_status = 0;
		bool m_isAlive = true;

		bool m_isDestroying = false;
		bool m_hasSpawned = false;
		
		GridMate::ReplicaChunkPtr m_replicaChunk;
	};
}
