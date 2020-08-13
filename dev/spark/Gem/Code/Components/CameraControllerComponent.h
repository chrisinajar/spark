#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzFramework/Input/Events/InputChannelEventListener.h>
#include <AzFramework/Entity/GameEntityContextBus.h>

#include "Busses/SlotBus.h"
#include "Busses/SelectionBus.h"
#include "Utils/CircularVector.h"
#include "Utils/UnitsGroup.h"
#include "Busses/AbilityBus.h"
#include "Utils/UnitOrder.h"
#include "Busses/CameraBus.h"
#include "Utils/DynamicSliceWrapper.h"

namespace spark
{
    class CameraControllerComponent
        : public AZ::Component
        , protected AzFramework::InputChannelEventListener
        , protected AZ::TickBus::Handler
		, protected SlotNotificationBus::Handler
		, protected SelectionRequestBus::Handler
		, protected CameraRequestBus::Handler
		, public AzFramework::ApplicationLifecycleEvents::Bus::Handler
        , protected AzFramework::GameEntityContextEventBus::Handler
    {
    public:
        AZ_COMPONENT(CameraControllerComponent, "{8E77EA6F-D2D5-41A1-AC61-459C32B11EEA}")

        ~CameraControllerComponent() override {};

        static void Reflect(AZ::ReflectContext* reflection);

        void Init() override;
        void Activate() override;
        void Deactivate() override;

    protected:
		void OnGameEntitiesStarted() override;
		void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
		void OnApplicationConstrained(AzFramework::ApplicationLifecycleEvents::Event /*lastEvent*/) override;
        void OnApplicationUnconstrained(AzFramework::ApplicationLifecycleEvents::Event /*lastEvent*/) override;

		//Input
		void OnSlotPressed(const Slot&);
		void OnSlotReleased(const Slot&);

        bool OnInputChannelEventFiltered(const AzFramework::InputChannel& inputChannel) override;
        void OnKeyboardEvent(const AzFramework::InputChannel& inputChannel);
        void OnMouseEvent(const AzFramework::InputChannel& inputChannel);

		//Camera Bus
		void SetFocus(bool focus)
		{
			m_inFocus = focus;
			if (!focus)
			{
				m_state = S_IDLE;
			}
		}
		bool HasFocus()
		{
			return m_inFocus;
		}
		void SetCameraMovementSpeed(float speed);
		float GetCameraMovevementSpeed();

		AZ::Vector3 ConvertPixelToWorldCoordinates(AZ::Vector3);
		AZ::Vector3 ConvertWorldToPixelCoordinates(AZ::Vector3);

		AZ::Vector3 TerrainRayCast(AZ::Vector2);
		AZ::Vector2 GetScreenSize();
		bool IsPointVisible(AZ::Vector3);
		void LookAtWorldPosition(AZ::Vector3);

		AZ::EntityId GetCameraEntityId();

		//Selection Bus
		void AddUnitToSelection(AZ::EntityId, bool setMainUnit);
		void SelectUnit(AZ::EntityId) ;
		void UnselectUnit(AZ::EntityId);
		void ClearSelection();
		void RotateToNextSelectedUnit();
		void RotateToPreviousSelectedUnit();
		void SetMainSelectedUnit(AZ::EntityId);
		void SetMainSelectedUnitByIndex(int);

		AZ::EntityId GetMainSelectedUnit();
		AZStd::vector<AZ::EntityId>   GetSelectedUnits();
		void						  SetSelectedUnits(AZStd::vector<AZ::EntityId>);

    private:
        
		void SendNewOrder(UnitOrder);

        void SendMoveOrder(AZ::Vector3 position);
		void SendAttackOrder(AZ::EntityId target, AZ::Vector3 position);

		void selectUnit(AZ::EntityId unit);
		
		void BoxSelection();

		UnitsGroup m_selection;

		enum {
			S_IDLE = 0,
			S_POINT_TARGETING = BIT(1),
			S_UNIT_TARGETING = BIT(2),
			S_POINT_OR_UNIT_TARGETING = S_POINT_TARGETING | S_UNIT_TARGETING,
			S_VECTOR_TARGETING = BIT(3),
			S_IGNORE_MOUSE_RELEASE = BIT(4),
			S_ATTACK_TARGETING = BIT(5),
		};
		int m_state = S_IDLE;

		AZ::EntityId m_currentAbility;
		CastContext m_castContext;

		bool m_queuing = false;
		bool m_only_main = false;

        bool m_movingUp = false;
        bool m_movingDown = false;
        bool m_movingLeft = false;
        bool m_movingRight = false;

        bool m_mouseEdgeDown = false;
        bool m_mouseEdgeLeft = false;
        bool m_mouseEdgeRight = false;
        bool m_mouseEdgeUp = false;

        float m_movementScale = 5.f;

		float m_attackOrderMaxDistance = 20.0f;

		AZ::Vector2 m_mousePosition; //normalized
		AZ::Vector3	m_boxStart;		 //in world-coordinates (for scrolling, but it's not used)

		bool m_mouseLeftButtonDown = false;
		bool m_boxSelecting = false;

		bool m_inFocus = true;
		bool m_inWindowFocus = true;

		DynamicSliceAsset m_selectionRing;
		DynamicSliceAsset m_moveOrderEffect;
		DynamicSliceAsset m_attackOrderEffect;

    };
}

