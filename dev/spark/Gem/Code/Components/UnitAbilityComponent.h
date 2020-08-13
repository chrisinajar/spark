#pragma once


#include <AzCore/Component/Component.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/containers/unordered_map.h>
#include <AzFramework/Network/NetBindable.h>

#include "Busses/UnitAbilityBus.h"

#include "Utils/SSM.h"
#include "Busses/UnitNavigationBus.h"

namespace spark
{
	typedef AZStd::unordered_map<Slot, AZ::EntityId> UnityAbilityMap;

	class UnitAbilityComponent
		: public AZ::Component
		, protected UnitAbilityRequestBus::Handler
		, protected CastContextRequestBus::Handler
		, protected AZ::TickBus::Handler
		, protected UnitNavigationNotificationBus::Handler
		, public AzFramework::NetBindable
	{
	public:
		AZ_COMPONENT(UnitAbilityComponent, "{78D0B4EF-97DB-4AB7-80A2-5EEF3C66E969}", AzFramework::NetBindable)

		~UnitAbilityComponent() override {};

		static void Reflect(AZ::ReflectContext* reflection);

		void Init() override;
		void Activate() override;
		void Deactivate() override;

		AZ::EntityId GetAbilityInSlot(Slot slot);
		bool HasAbilityInSlot(Slot slot);
		void SetAbilityInSlot(Slot slot, AZ::EntityId ability);
		void ClearAbilitySlot(Slot slot);
		void ClearAllAbilities();

		AbilityId HasAbilityType(AbilityTypeId);
		bool HasAbility(AbilityId);
		Slot GetAbilitySlot(AbilityId);
		void DetachAbility(AbilityId);

		AZStd::vector<AbilityId> GetAbilities();

		bool CanCastAbility(AZ::EntityId ability) override; 
		AZ::EntityId CanCastAbilityInSlot(Slot slot) override; 
		void CastAbility(CastContext);
		void InterruptCasting();

		bool IsItem();
		UnitId GetCaster();
		UnitId GetCursorTarget();
		AZ::Vector3 GetCursorPosition();
		CastingBehavior GetBehaviorUsed();


		AZStd::string ToString();

		GridMate::ReplicaChunkPtr GetNetworkBinding() override;
		void SetNetworkBinding(GridMate::ReplicaChunkPtr chunk) override;
		void UnbindFromNetwork() override;
		void OnNewAbilityList (const UnityAbilityMap& abilities, const GridMate::TimeContext& tc);
		void SyncAbilityList ();
	private:
		//AZStd::array<AZ::EntityId, 16> m_abilities;
		UnityAbilityMap m_abilities;
		//AZStd::vector<AZ::EntityId> m_abilities;
		GridMate::ReplicaChunkPtr m_replicaChunk = nullptr;
		
		CastContext m_castContext;

	//State machine
		void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
		void OnMovingDone() override;
		void OnDestinationUnreachable() override;

		enum {
			S_IDLE,
			S_FORESWING,
			S_BACKSWING,
			S_MOVING
		};
		enum {
			E_UPDATE,
			E_STOP,
			E_TRAVERSAL_COMPLETE,
			E_DESTINATION_UNREACHABLE
		};
		SSM m_stateMachine;
		float m_elapsedTime;
		bool  m_casting = false;
		bool OnStateIdle	 (SSM& m, const SSM::Event& e);
		bool OnStateForeswing(SSM& m, const SSM::Event& e);
		bool OnStateBackswing(SSM& m, const SSM::Event& e);
		bool OnStateMoving	 (SSM& m, const SSM::Event& e);

		void OnCastFinished();

		AZ::Vector3 GetTargetPosition();
		float GetDistanceFromTarget();

		float GetMaxRange();

		float GetCastPoint();
		float GetBackswingPoint();

		float GetBaseCastPoint();
		float GetBaseBackswingPoint();

		float GetIncreasedCastSpeed();
		void  SetIncreasedCastSpeed(float value);

		//float m_maxRange;
		//float m_baseCastPoint;
		//float m_baseBackswingPoint;
		//float m_ICS;
	};

}