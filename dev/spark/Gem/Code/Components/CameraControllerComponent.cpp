
#include "spark_precompiled.h"

#include "CameraControllerComponent.h"

#include <ISystem.h>
#include <CryAction.h>
#include <Cry_Camera.h>
#include <IRenderer.h>
#include <MathConversion.h>

#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Math/Transform.h>
#include <AzFramework/Input/Channels/InputChannel.h>
#include <AzFramework/Input/Devices/Keyboard/InputDeviceKeyboard.h>
#include <AzFramework/Input/Devices/Mouse/InputDeviceMouse.h>
#include <LmbrCentral/Ai/NavigationComponentBus.h>

#include <PhysicsWrapper/PhysicsWrapperBus.h>
#include <MouseCursorPosition/MouseCursorPositionBus.h>
#include "Busses/UnitBus.h"
#include "Busses/GameManagerBus.h"
#include "Busses/UnitAbilityBus.h"
#include "Busses/VariableBus.h"
#include "Busses/NetSyncBusses.h"
#include "Busses/GameplayBusses.h"
#include "Busses/NavigationManagerBus.h"
#include "Busses/SettingsBus.h"

#include "Utils/Log.h"
#include "Utils/Pathfinder.h"
#include "Utils/NavigationUtils.h"
#include "Utils/StringUtils.h"

using namespace spark;
using namespace AzFramework;
using namespace PhysicsWrapper;



//Utils
Hit PerformRayCast(AZ::Vector2 pos)
{
	auto &cam = GetISystem()->GetViewCamera();

	Vec3 start, end;
	Hit hit;

	// invert the mouse y for the raycast
	float invMouseY = static_cast<float>(GetISystem()->GetIRenderer()->GetHeight()) - pos.GetY();

	//screen to world coordinate conversion
	cam.Unproject(Vec3(pos.GetX(), invMouseY, 0), start);
	cam.Unproject(Vec3(pos.GetX(), invMouseY, 1), end);

	//perform raycast
	PhysicsWrapperRequestBus::BroadcastResult(hit, &PhysicsWrapperRequests::PerformSingleResultRayCast, LYVec3ToAZVec3(start), LYVec3ToAZVec3(end));
	return hit;
}

Hit PerformRayCastAtMousePosition() 
{
	int width = GetISystem()->GetIRenderer()->GetWidth(), height = GetISystem()->GetIRenderer()->GetHeight();
	AZ::Vector2 screen_size((float)width, (float)height);
	AZ::Vector2 mouseCoord;

	//Get mouse coord in pixel space
	MouseCursorPosition::MouseCursorPositionRequestBus::BroadcastResult(mouseCoord, &MouseCursorPosition::MouseCursorPositionRequests::GetSystemCursorPositionNormalized);
	mouseCoord = mouseCoord * screen_size;
	//CryLogAlways("mouse coord : (%f,%f)", mouseCoord.GetX(), mouseCoord.GetY());

	return PerformRayCast(mouseCoord);
}



//CameraControllerComponent implementation

void CameraControllerComponent::Reflect(AZ::ReflectContext* reflection)
{
	if (auto serializationContext = azrtti_cast<AZ::SerializeContext*>(reflection))
	{
		serializationContext->Class<CameraControllerComponent, AZ::Component>()
			->Version(1)
			->Field("Movement scale", &CameraControllerComponent::m_movementScale)
			->Field("selection ring", &CameraControllerComponent::m_selectionRing)
			->Field("move order effect", &CameraControllerComponent::m_moveOrderEffect)
			->Field("attack order effect", &CameraControllerComponent::m_attackOrderEffect)
			;

		if (auto editContext = serializationContext->GetEditContext())
		{
			editContext->Class<CameraControllerComponent>("CameraControllerComponent", "Typical RTS-ish camera controller")
				->ClassElement(AZ::Edit::ClassElements::EditorData, "")
				->Attribute(AZ::Edit::Attributes::Category, "spark")
				->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"))
				->DataElement(nullptr, &CameraControllerComponent::m_movementScale, "Movement scale", "How fast the camera moves")
				->DataElement(nullptr, &CameraControllerComponent::m_selectionRing, "selection ring", "")
				->DataElement(nullptr, &CameraControllerComponent::m_moveOrderEffect, "move order effect", "")
				->DataElement(nullptr, &CameraControllerComponent::m_attackOrderEffect, "attack order effect","");
		}
	}

	if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflection))
	{

		behaviorContext->EBus<SlotNotificationBus>("SlotNotificationBus")
			->Handler<SlotNotificationBusHandler>()
			->Event("OnSlotPressed", &SlotNotificationBus::Events::OnSlotPressed)
			->Event("OnSlotReleased", &SlotNotificationBus::Events::OnSlotReleased);


		behaviorContext->EBus<SelectionRequestBus>("SelectionRequestBus")
			->Event("AddUnitToSelection", &SelectionRequestBus::Events::AddUnitToSelection)
			->Event("UnselectUnit", &SelectionRequestBus::Events::UnselectUnit)
			->Event("SelectUnit", &SelectionRequestBus::Events::SelectUnit)
			->Event("ClearSelection", &SelectionRequestBus::Events::ClearSelection)
			->Event("GetMainSelectedUnit", &SelectionRequestBus::Events::GetMainSelectedUnit)
			->Event("GetSelectionGroup", &SelectionRequestBus::Events::GetSelectedUnits)
			->Event("SetSelectionGroup", &SelectionRequestBus::Events::SetSelectedUnits)
			->Event("RotateToNextSelectedUnit", &SelectionRequestBus::Events::RotateToNextSelectedUnit)
			->Event("RotateToPreviousSelectedUnit", &SelectionRequestBus::Events::RotateToPreviousSelectedUnit)
			->Event("SetMainSelectedUnit", &SelectionRequestBus::Events::SetMainSelectedUnit)
			->Event("SetMainSelectedUnitByIndex", &SelectionRequestBus::Events::SetMainSelectedUnitByIndex);
		
		behaviorContext->EBus<SelectionNotificationBus>("SelectionNotificationBus")
			->Handler<SelectionNotificationBusHandler>();

		behaviorContext->EBus<CameraRequestBus>("SparkCameraRequestBus")
			->Event("SetFocus", &CameraRequestBus::Events::SetFocus)
			->Event("HasFocus", &CameraRequestBus::Events::HasFocus)
			->Event("ConvertWorldToPixelCoordinates", &CameraRequestBus::Events::ConvertWorldToPixelCoordinates)
			->Event("ConvertPixelToWorldCoordinates", &CameraRequestBus::Events::ConvertPixelToWorldCoordinates)
			->Event("TerrainRayCast", &CameraRequestBus::Events::TerrainRayCast)
			->Event("SetCameraMovementSpeed", &CameraRequestBus::Events::SetCameraMovementSpeed)
			->Event("GetCameraMovevementSpeed", &CameraRequestBus::Events::GetCameraMovevementSpeed)
			->Event("GetScreenSize", &CameraRequestBus::Events::GetScreenSize)
			->Event("IsPointVisible", &CameraRequestBus::Events::IsPointVisible)
			->Event("LookAtWorldPosition", &CameraRequestBus::Events::LookAtWorldPosition)
			->Event("GetCameraEntityId", &CameraRequestBus::Events::GetCameraEntityId)
			;
	}

	ReflectGameplayBusses(reflection);
}

void CameraControllerComponent::Init()
{

}

void CameraControllerComponent::Activate()
{
	AzFramework::GameEntityContextEventBus::Handler::BusConnect();
	AzFramework::ApplicationLifecycleEvents::Bus::Handler::BusConnect();
	CameraRequestBus::Handler::BusConnect();
	AZ::TickBus::Handler::BusConnect();
	InputChannelEventListener::Connect();
	
	SlotNotificationBus::Handler::BusConnect();
	SelectionRequestBus::Handler::BusConnect();
}

void CameraControllerComponent::Deactivate()
{
	SelectionRequestBus::Handler::BusDisconnect();
	SlotNotificationBus::Handler::BusDisconnect();

	InputChannelEventListener::Disconnect();
	AZ::TickBus::Handler::BusDisconnect();
	CameraRequestBus::Handler::BusDisconnect();

	AzFramework::ApplicationLifecycleEvents::Bus::Handler::BusDisconnect();
	AzFramework::GameEntityContextEventBus::Handler::BusDisconnect();
}


void CameraControllerComponent::OnGameEntitiesStarted()
{
	EBUS_EVENT(AzFramework::InputSystemCursorRequestBus, SetSystemCursorState, AzFramework::SystemCursorState::ConstrainedAndVisible);

	// selectionRingEntity = DynamicSliceWrapper(selectionRingSlice);

	// for (int i = 0; i < 10; i++)moveCursorEntities.emplace_back(moveCursorSlice);

}


void CameraControllerComponent::SetCameraMovementSpeed(float speed)
{
	m_movementScale = speed;
}

float CameraControllerComponent::GetCameraMovevementSpeed()
{
	return m_movementScale;
}

AZ::Vector3 CameraControllerComponent::ConvertPixelToWorldCoordinates(AZ::Vector3 point)
{
	Vec3 result;

	//screen to world coordinate conversion
	GetISystem()->GetViewCamera().Unproject(AZVec3ToLYVec3(point), result);

	return LYVec3ToAZVec3(result);
}
AZ::Vector3 CameraControllerComponent::ConvertWorldToPixelCoordinates(AZ::Vector3 point)
{
	Vec3 result;

	//world to screen coordinate conversion
	GetISystem()->GetViewCamera().Project(AZVec3ToLYVec3(point), result);

	return LYVec3ToAZVec3(result);
}

AZ::Vector3 CameraControllerComponent::TerrainRayCast(AZ::Vector2 pos)
{
	return PerformRayCast(pos).position;
}

AZ::Vector2 CameraControllerComponent::GetScreenSize()
{
	float width = GetISystem()->GetIRenderer()->GetWidth(), height = GetISystem()->GetIRenderer()->GetHeight();
	return AZ::Vector2{ width,height };
}

bool spark::CameraControllerComponent::IsPointVisible(AZ::Vector3 pos)
{
	return GetISystem()->GetViewCamera().IsPointVisible(AZVec3ToLYVec3(pos));
}

void spark::CameraControllerComponent::LookAtWorldPosition(AZ::Vector3 position)
{
	AZ::Vector3 currentlyLookingAt = TerrainRayCast(GetScreenSize()*0.5f);
	AZ::Vector3 movement = position - currentlyLookingAt;
	movement.SetZ(0);

	AZ::Vector3 currentPosition;
	EBUS_EVENT_ID_RESULT(currentPosition, GetEntityId(), AZ::TransformBus, GetWorldTranslation);

	AZ::Vector3 newPosition = currentPosition + movement;
	EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus, SetWorldTranslation, newPosition);
}

AZ::EntityId CameraControllerComponent::GetCameraEntityId()
{
	return GetEntityId();
}

//####// Selection //####//
void CameraControllerComponent::AddUnitToSelection(AZ::EntityId unit, bool setMainUnit)
{
	if (!unit.IsValid())return;
	

	bool isSelectable=false;

	// find out if unit is selectable
	///@todo check if it's controllable by the player
	EBUS_EVENT_ID_RESULT(isSelectable, unit, UnitRequestBus, UnitRequests::IsSelectable);
	if (!isSelectable) return;

	m_selection.Add(unit,setMainUnit);
	EBUS_EVENT(SelectionNotificationBus, OnSelectionChanged, m_selection.GetVector());
	EBUS_EVENT(SelectionNotificationBus, OnMainSelectedUnitChanged, m_selection.GetMainUnit());
}
void CameraControllerComponent::SelectUnit(AZ::EntityId unit)
{
	if (!unit.IsValid()) {
		sERROR(" SelectUnit called with invalid unit");
		return;
	}
	

	bool isSelectable=false;

	// find out if unit is selectable
	///@todo check if it's controllable by the player
	EBUS_EVENT_ID_RESULT(isSelectable, unit, UnitRequestBus, IsSelectable);
	if (!isSelectable) {
		sERROR(" SelectUnit called but the unit cannot be selected");
		return;
	}
	m_selection.Set(unit);
	EBUS_EVENT(SelectionNotificationBus, OnSelectionChanged, m_selection.GetVector());
	EBUS_EVENT(SelectionNotificationBus, OnMainSelectedUnitChanged, m_selection.GetMainUnit());
}
void CameraControllerComponent::UnselectUnit(AZ::EntityId unit)
{
	m_selection.Remove(unit);
	EBUS_EVENT(SelectionNotificationBus, OnSelectionChanged, m_selection.GetVector());
	EBUS_EVENT(SelectionNotificationBus, OnMainSelectedUnitChanged, m_selection.GetMainUnit());
}
void CameraControllerComponent::ClearSelection()
{
	m_selection.Clear();
	EBUS_EVENT(SelectionNotificationBus, OnSelectionChanged, m_selection.GetVector());
	EBUS_EVENT(SelectionNotificationBus, OnMainSelectedUnitChanged, AZ::EntityId());
}
AZ::EntityId CameraControllerComponent::GetMainSelectedUnit()
{
	return m_selection.GetMainUnit();
}
void CameraControllerComponent::RotateToNextSelectedUnit()
{
	m_selection.NextUnit();
	EBUS_EVENT(SelectionNotificationBus, OnMainSelectedUnitChanged, m_selection.GetMainUnit());
}
void CameraControllerComponent::RotateToPreviousSelectedUnit()
{
	m_selection.PreviousUnit();
	EBUS_EVENT(SelectionNotificationBus, OnMainSelectedUnitChanged, m_selection.GetMainUnit());
}
void CameraControllerComponent::SetMainSelectedUnit(AZ::EntityId unit)
{
	m_selection.SetMainUnit(unit);
	EBUS_EVENT(SelectionNotificationBus, OnMainSelectedUnitChanged, m_selection.GetMainUnit());
}
void CameraControllerComponent::SetMainSelectedUnitByIndex(int index)
{
	m_selection.SetMainUnitByIndex(index);
	EBUS_EVENT(SelectionNotificationBus, OnMainSelectedUnitChanged, m_selection.GetMainUnit());
}
AZStd::vector<AZ::EntityId> CameraControllerComponent::GetSelectedUnits()
{
	return m_selection.GetVector();
}
void CameraControllerComponent::SetSelectedUnits(AZStd::vector<AZ::EntityId> selection)
{
	m_selection.Clear();
	for (auto u : selection)
	{
		m_selection.Add(u);
	}
	m_selection.SetMainUnitByIndex(0);

	EBUS_EVENT(SelectionNotificationBus, OnSelectionChanged, m_selection.GetVector());
	EBUS_EVENT(SelectionNotificationBus, OnMainSelectedUnitChanged, m_selection.GetMainUnit());
}

//####// Input //####//
void CameraControllerComponent::OnSlotPressed(const Slot& slot)
{
	if (!m_inFocus)return;

	//AZ_Printf(0, "CameraControllerComponent::OnSlotPressed -> %s", slot.ToString().c_str());
	sLOG("CameraControllerComponent::OnSlotPressed -> " + slot);

	if (slot == Slot::Queue)
	{
		m_queuing = true;
		return;
	}


	if (m_selection.empty())return;


	//m_state = S_IDLE;//interrupt(if any) the current targeting 

	switch (slot.GetType()) {
	case Slot::Attack:
	{
		AZStd::string quickAttack;
		EBUS_EVENT_RESULT(quickAttack, SettingsRequestBus, GetSettingValue, "QuickAttack", "Game");
		StringUtils::trim(quickAttack);

		if (quickAttack=="true" || quickAttack=="1")
		{
			Hit hit = PerformRayCastAtMousePosition();

			SendAttackOrder(AZ::EntityId(), hit.position);
		}
		else
		{
			m_state = S_POINT_TARGETING | S_UNIT_TARGETING | S_ATTACK_TARGETING;
		}
	}break;
	case Slot::Stop:
		AZ_Printf(0, "I'm trying to send a stop order!");
		SendNewOrder(UnitOrder::StopOrder());
		break;
	case Slot::Inventory:
	case Slot::Ability://do casting stuff	
	{
		AZ::EntityId ability;
		EBUS_EVENT_ID_RESULT(ability, m_selection.GetMainUnit(), UnitAbilityRequestBus, CanCastAbilityInSlot, slot);

		if (ability.IsValid()) {
			//get casting behavior
			CastingBehavior behavior;
			EBUS_EVENT_ID_RESULT(behavior, ability, AbilityRequestBus, GetCastingBehavior);

			AZ_Printf(0, " CastingBehavior is : %d", behavior.GetFlags());

			Hit hit = PerformRayCastAtMousePosition();

			if (m_castContext.ability == ability && m_state & S_UNIT_TARGETING)
			{
				//todo filter is ability can be cast on self
				sLOG("self casting");
				m_castContext.target = m_selection.GetMainUnit();
				m_castContext.behaviorUsed = CastingBehavior(CastingBehavior::TargetingBehavior::UNIT_TARGET);
				SendNewOrder(UnitOrder::CastOrder(m_castContext));
				return;
			}

			m_castContext = CastContext();
			m_castContext.caster = m_selection.GetMainUnit();
			m_castContext.cursorPosition = hit.position;
			m_castContext.isItem = false;
			m_castContext.ability = ability;

			

			bool isSelectable = false;
			EBUS_EVENT_ID_RESULT(isSelectable, hit.entity, UnitRequestBus, UnitRequests::IsSelectable);

			if (isSelectable)m_castContext.target = hit.entity;

			//todo check if is ready(no cooldown) and if there are enough resources(mana/hp/gold...)
			if (behavior.GetFlags() & CastingBehavior::TargetingBehavior::IMMEDIATE)
			{
				m_castContext.behaviorUsed = CastingBehavior(CastingBehavior::TargetingBehavior::IMMEDIATE);
				SendNewOrder(UnitOrder::CastOrder(m_castContext));
			}
			else
			{
				if (behavior.GetFlags() & CastingBehavior::TargetingBehavior::POINT_TARGET)m_state |= S_POINT_TARGETING;
				if (behavior.GetFlags() & CastingBehavior::TargetingBehavior::UNIT_TARGET) m_state |= S_UNIT_TARGETING;
			}

		}
	}
	break;
	}
}
void CameraControllerComponent::OnSlotReleased(const Slot& slot)
{
	if (!m_inFocus)return;

	if (slot == Slot::Queue)
	{
		m_queuing = false;
		return;
	}
}

bool CameraControllerComponent::OnInputChannelEventFiltered(const AzFramework::InputChannel& inputChannel)
{
	const InputDeviceId deviceId = inputChannel.GetInputDevice().GetInputDeviceId();

	//AZ_Printf(0,"CameraControllerComponent handling event:	channleId: %s   deviceId: %s",inputChannel.GetInputChannelId().GetName(),inputChannel.GetInputDevice().GetInputDeviceId().GetName());

	if (deviceId == InputDeviceKeyboard::Id)
	{
		OnKeyboardEvent(inputChannel);
	}
	else if (deviceId == InputDeviceMouse::Id)
	{
		OnMouseEvent(inputChannel);
	}

	//event not consumed 
	return false;
}

void CameraControllerComponent::OnKeyboardEvent(const InputChannel& inputChannel)
{
	// map key bindings to actions and send them to all the selected units or enter a targetting state

	auto input_type = inputChannel.GetInputChannelId();
	if (input_type == InputDeviceKeyboard::Key::Alphanumeric1 && inputChannel.IsStateBegan())
	{
		AZStd::string log;
		EBUS_EVENT_RESULT(log,VariableManagerRequestBus, ToString);
		AZ_Printf(0, "%s", log.c_str());
	}
	else if (input_type == InputDeviceKeyboard::Key::Alphanumeric2 && inputChannel.IsStateBegan())
	{
		AZStd::string log,str;
		EBUS_EVENT_ID_RESULT(str, m_selection.GetMainUnit(), UnitRequestBus, ToString);
		log += str+"\n";

		EBUS_EVENT_ID_RESULT(str, m_selection.GetMainUnit(), UnitAbilityRequestBus, ToString);
		log += str;
		AZ_Printf(0, "%s", log.c_str());
	}
	else if (input_type == InputDeviceKeyboard::Key::Alphanumeric3 && inputChannel.IsStateBegan())
	{
		sLOG("toggling g_FogOfWar");
		ICVar* var = gEnv->pConsole->GetCVar("g_FogOfWar");

		if (var) {
			var->Set(!var->GetIVal());
		}
	}
	else if (input_type == InputDeviceKeyboard::Key::Alphanumeric4 && inputChannel.IsStateBegan())
	{
		sLOG("toggling g_DebugTiles");
		ICVar* var = gEnv->pConsole->GetCVar("g_DebugTiles");

		if (var) {
			var->Set(!var->GetIVal());
		}
	}
}
void CameraControllerComponent::OnMouseEvent(const InputChannel& inputChannel)
{
	auto input_type = inputChannel.GetInputChannelId();

	if (input_type == InputDeviceMouse::SystemCursorPosition) //onmousemove
	{
		if (m_mouseLeftButtonDown)
		{
			m_boxSelecting = true;
		}
		
		m_mouseEdgeDown = m_mouseEdgeLeft = m_mouseEdgeRight = m_mouseEdgeUp = false;

		if (auto position_data = inputChannel.GetCustomData<InputChannel::PositionData2D>())
		{
			auto position = position_data->m_normalizedPosition;
			auto x = position.GetX();
			auto y = position.GetY();

			m_mousePosition = AZ::Vector2{ x, y };

			m_mouseEdgeLeft = x <= .01;
			m_mouseEdgeRight = x >= .99;
			m_mouseEdgeUp = y <= .01;
			m_mouseEdgeDown = y >= .99;
		}
		return;
	}

	if (!inputChannel.IsStateBegan() && !inputChannel.IsStateEnded())return; //ignore holding down events
	if (input_type != InputDeviceMouse::Button::Left && input_type != InputDeviceMouse::Button::Middle && input_type != InputDeviceMouse::Button::Right)return;//only consider the standard button events from now on


	AZ_Printf(0, "CameraControllerComponent handling event:	channleId: %s   deviceId: %s", inputChannel.GetInputChannelId().GetName(), inputChannel.GetInputDevice().GetInputDeviceId().GetName());

	if (m_state!= S_IDLE && input_type == InputDeviceMouse::Button::Left && inputChannel.IsStateBegan())
	{
		if(m_state & S_POINT_OR_UNIT_TARGETING)
		{
			Hit hit = PerformRayCastAtMousePosition();

			if (m_state & S_ATTACK_TARGETING)
			{
				SendAttackOrder(AZ::EntityId(), hit.position);
			}
			else
			{
				m_castContext.cursorPosition = hit.position;

				bool isSelectable = false;
				EBUS_EVENT_ID_RESULT(isSelectable, hit.entity, UnitRequestBus, UnitRequests::IsSelectable);

				if (isSelectable)
				{
					Status status = 0;
					EBUS_EVENT_ID_RESULT(status, hit.entity, UnitRequestBus, GetStatus);
					if (!(status & CANT_BE_SPELL_TARGETED))
					{
						m_castContext.target = hit.entity;
					}
					else
					{
						isSelectable = false;
					}
				}

				if (m_state == S_UNIT_TARGETING && !isSelectable)
				{
					sWARNING("Ability need to be targeted on a valid unit!");
					m_state = S_IGNORE_MOUSE_RELEASE;
					return;
				}

				if (m_state & S_UNIT_TARGETING && isSelectable)
				{
					m_castContext.behaviorUsed = CastingBehavior(CastingBehavior::TargetingBehavior::UNIT_TARGET);
				}
				else
				{
					m_castContext.behaviorUsed = CastingBehavior(CastingBehavior::TargetingBehavior::POINT_TARGET);
				}

				SendNewOrder(UnitOrder::CastOrder(m_castContext));
			}
		}
		else if(m_state != S_IDLE)
		{
			sWARNING(" targeting type not supported yet!");
		}
		m_state = S_IGNORE_MOUSE_RELEASE;
		return;
	}

	if (inputChannel.IsStateEnded() && m_state == S_IGNORE_MOUSE_RELEASE)
	{
		//reset
		m_state = S_IDLE;
		m_castContext = CastContext();
		return;
	}


	SetFocus(true);

	//handling left button
	if (input_type == InputDeviceMouse::Button::Left && inputChannel.IsStateBegan())//onmousedown left
	{
		m_mouseLeftButtonDown = true;

		float width = GetISystem()->GetIRenderer()->GetWidth(), height = GetISystem()->GetIRenderer()->GetHeight();
		AZ::Vector2 screen_size{ width,height};
		auto mousePos = screen_size*m_mousePosition;

		m_boxStart = PerformRayCast(mousePos).position;

		sLOG("left down at "+mousePos+"     world:"+m_boxStart);
		return;
	}

	if (input_type == InputDeviceMouse::Button::Left && inputChannel.IsStateEnded())//onmouseup left -> select
	{
		sLOG("left up");

		m_mouseLeftButtonDown = false;

		if (m_boxSelecting)
		{
			BoxSelection();

			m_boxSelecting = false;
		}
		else
		{
			Hit hit = PerformRayCastAtMousePosition();

			selectUnit(hit.entity);
		}
		return;
	}
	//left button handled


	//if the left button is pressed but another button is pressed, the box selection is aborted
	m_mouseLeftButtonDown = false;
	m_boxSelecting = false;
	


	if (input_type == InputDeviceMouse::Button::Right && inputChannel.IsStateBegan()) //onmousedown right -> send order
	{
		sLOG("right down");
		Hit hit = PerformRayCastAtMousePosition();
		auto id = hit.entity;

		bool isSelectable = false;
		EBUS_EVENT_ID_RESULT(isSelectable, hit.entity, UnitRequestBus, IsSelectable);

		bool interactable = false;
		EBUS_EVENT_ID_RESULT(interactable, hit.entity, LmbrCentral::TagComponentRequestBus, HasTag, LmbrCentral::Tag("interactable"));

		if (isSelectable || interactable)
		{
			FilterResult result;
			result.Push();
			EBUS_EVENT(OnRightClickedGlobalNotificationBus, OnEntityRightClickedFilter, id);
			result.Pop();

			if (result.action == FilterResult::FILTER_PREVENT)return;

			result.Clear();
			result.Push();
			EBUS_EVENT_ID(id,OnRightClickedNotificationBus, OnRightClickedFilter);
			result.Pop();

			if(result.action==FilterResult::FILTER_PREVENT)return;
			
			EBUS_EVENT(OnRightClickedGlobalNotificationBus, OnEntityRightClicked, id);
			EBUS_EVENT_ID(id, OnRightClickedNotificationBus, OnRightClicked);

			if(isSelectable)SendAttackOrder(id,hit.position);
		}
		else
		{
			auto moveto = hit.position;
			SendMoveOrder(moveto);
		}
	}

	
}

void CameraControllerComponent::OnApplicationConstrained(AzFramework::ApplicationLifecycleEvents::Event /*lastEvent*/)
{
	m_inWindowFocus = false;
}
void CameraControllerComponent::OnApplicationUnconstrained(AzFramework::ApplicationLifecycleEvents::Event /*lastEvent*/)
{
	m_inWindowFocus = true;
}
void CameraControllerComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time)
{
	if (!m_inWindowFocus)return;

	AZ::Transform entityTransform;
	EBUS_EVENT_ID_RESULT(entityTransform, GetEntityId(), AZ::TransformBus, GetWorldTM);

	float x = 0.f;
	float y = 0.f;

	if (m_movingUp || m_mouseEdgeUp)
	{
		y += m_movementScale * deltaTime;
	}
	if (m_movingDown || m_mouseEdgeDown)
	{
		y -= m_movementScale * deltaTime;
	}
	if (m_movingRight || m_mouseEdgeRight)
	{
		x += m_movementScale * deltaTime;
	}
	if (m_movingLeft || m_mouseEdgeLeft)
	{
		x -= m_movementScale * deltaTime;
	}

	if (!m_boxSelecting && (y != 0.f || x != 0.f)) //don't move the camera while box selecting
	{
		auto new_position = entityTransform.GetPosition() + AZ::Vector3{ x, y, 0.f };
		entityTransform.SetPosition(new_position);

		EBUS_EVENT_ID(GetEntityId(), AZ::TransformBus, SetWorldTM, entityTransform);
	}

	

	auto *pPersistentDebug = gEnv->pGame->GetIGameFramework()->GetIPersistentDebug();

	if (pPersistentDebug)
	{
		pPersistentDebug->Begin("selectionRings", true);

		for (auto u : m_selection)
		{
			bool visible = false;
			EBUS_EVENT_ID_RESULT(visible, u, UnitRequestBus, IsVisible);

			if (visible)
			{
				ColorF color = Col_Gray;
				if (u == m_selection.GetMainUnit())color = Col_White;

				AZ::Vector3 pos;
				EBUS_EVENT_ID_RESULT(pos, u, AZ::TransformBus, GetWorldTranslation);
				pPersistentDebug->AddPlanarDisc(AZVec3ToLYVec3(pos), 0.7, 1, color, 0.1);
			}
		}

		if (m_state & S_ATTACK_TARGETING)
		{
			Hit hit = PerformRayCastAtMousePosition();

			float radius = 4;
			pPersistentDebug->Begin("attackRing", true);
			pPersistentDebug->AddPlanarDisc(AZVec3ToLYVec3(hit.position), radius-0.2 , radius, Col_Red, 0.1);
		}

		if (m_boxSelecting)
		{
			using pathfinder::vec2;

			int width = GetISystem()->GetIRenderer()->GetWidth(), height = GetISystem()->GetIRenderer()->GetHeight();
			vec2 screen_size{ width,height };
			AZ::Vector3 s = ConvertWorldToPixelCoordinates(m_boxStart);


			vec2 start(s.GetX(), s.GetY());
			vec2 end = screen_size * vec2{ m_mousePosition.GetX(),m_mousePosition.GetY() };
			vec2 size = end - start;

			size.x = abs(size.x);
			size.y = abs(size.y);

			start.x = min(start.x, end.x);
			start.y = min(start.y, end.y);

			pPersistentDebug->Begin("selectionBox", true);
			auto color = Col_White;
			color.a = 0.4;

			pPersistentDebug->Add2DRect(start.x, start.y, size.x, size.y, color, 0.05);
		}
	}
}


//####// internal utility functions //####//

void CameraControllerComponent::SendNewOrder(UnitOrder order)
{
	///@TODO: Check if each units is controllable by us

	bool only_main = m_only_main;

	if (order.type == order.CAST) only_main = true;

	//clean the state
	m_state = S_IDLE; 
	m_castContext = CastContext();

	if (only_main)
	{
		//EBUS_EVENT_ID(unit, UnitRequestBus, UnitRequests::NewOrder, order, queue);
		EBUS_EVENT(GameNetSyncRequestBus, SendNewOrder, m_selection.GetMainUnit(), order, m_queuing);
	}
	else
	{
		for (auto u : m_selection)
		{
			EBUS_EVENT(GameNetSyncRequestBus, SendNewOrder, u, order, m_queuing);
		}
	}
}

void CameraControllerComponent::SendMoveOrder(AZ::Vector3 position)
{
	AZ_Printf(0, "I'm trying to send a move order! %d",m_queuing);

	DynamicSliceWrapper dynamicSlice(m_moveOrderEffect);
	dynamicSlice.transform = AZ::Transform::CreateTranslation(position);
	dynamicSlice.Load();

	SendNewOrder(UnitOrder::MoveOrder(position));
}

void CameraControllerComponent::SendAttackOrder(AZ::EntityId target, AZ::Vector3 position)
{
	AZ_Printf(0, "I'm trying to send an attack order!");

	DynamicSliceWrapper dynamicSlice(m_attackOrderEffect);
	dynamicSlice.transform = AZ::Transform::CreateTranslation(position);
	dynamicSlice.Load();

	if (!target.IsValid())
	{
		EBUS_EVENT_RESULT(target, GameManagerRequestBus, GetNearestEnemyUnit, m_selection.GetMainUnit(), position);
		if (!target.IsValid())
		{
			//AZ_Printf(0, "There is no enemy unit to attack!");

			SendNewOrder(UnitOrder::AttackMoveOrder(position));

			return;
		}


		AZ::Vector3 target_pos;
		EBUS_EVENT_ID_RESULT(target_pos, target, NavigationEntityRequestBus, GetPosition);

		float distance = Distance2D(position, target_pos);

		if (distance > m_attackOrderMaxDistance)
		{
			sLOG("The nearest unit is too far!");
			return;
		}
	}

	Status status;
	EBUS_EVENT_ID_RESULT(status, target, UnitRequestBus, GetStatus);

	if (status & CANT_BE_ATTACKED) {
		sLOG("the target can't be attacked!");
		return;
	}

	
	SendNewOrder(UnitOrder::AttackOrder(target));
}

void CameraControllerComponent::selectUnit(AZ::EntityId unit)
{
	if (!unit.IsValid())return;
	

	bool isSelectable=false;

	// find out if unit is selectable
	///@todo check if it's controllable by the player
	EBUS_EVENT_ID_RESULT(isSelectable, unit, UnitRequestBus, UnitRequests::IsSelectable);
	if (!isSelectable) return;


	if (m_queuing)
	{
		if (!m_selection.Contains(unit)) {
			AZ_Printf(0, "Added unit to the selection");
			//m_selection.Add(unit);
			EBUS_EVENT(SelectionRequestBus, AddUnitToSelection, unit,false);
		}
		else
		{
			AZ_Printf(0, "Removed unit to the selection");
			//m_selection.Remove(unit);
			EBUS_EVENT(SelectionRequestBus, UnselectUnit, unit);
		}
	}
	else
	{
		//m_selection.Set(unit);
		EBUS_EVENT(SelectionRequestBus, SelectUnit, unit);
	}		
}

void spark::CameraControllerComponent::BoxSelection()
{
	float width = GetISystem()->GetIRenderer()->GetWidth(), height = GetISystem()->GetIRenderer()->GetHeight();
	AZ::Vector2 screen_size{ width,height };
	AZ::Vector2 end = screen_size * m_mousePosition;
	
	AZ::Vector3 point = PerformRayCast(end).position;

	//create a camera-space aabb ( a world-space frustum )
	AZ::Aabb aabb = AZ::Aabb::CreateFromPoint(ConvertWorldToPixelCoordinates(m_boxStart) + AZ::Vector3(0, 0, -500));
	aabb.AddPoint(ConvertWorldToPixelCoordinates(point) + AZ::Vector3(0, 0, 500));
	

	AZStd::vector<UnitId> units,new_selection; 
	EBUS_EVENT(UnitsRequestBus, GetAllUnits, units);

	
	for (auto u : units)
	{
		AZ::Vector3 pos;
		EBUS_EVENT_ID_RESULT(pos, u, AZ::TransformBus, GetWorldTranslation);
		
		if (aabb.Contains(ConvertWorldToPixelCoordinates(pos))) {
			
			bool isSelectable = false;

			// find out if unit is selectable
			///@todo check if it's controllable by the player
			EBUS_EVENT_ID_RESULT(isSelectable, u, UnitRequestBus, IsSelectable);
			if (isSelectable)
			{
				new_selection.push_back(u);
			}
		}
	}
	//raycast anyway on the corner, so it's easier to select a single unit
	{
		auto hit = PerformRayCastAtMousePosition();
		bool isSelectable = false;

		// find out if unit is selectable
		///@todo check if it's controllable by the player
		EBUS_EVENT_ID_RESULT(isSelectable, hit.entity, UnitRequestBus, IsSelectable);
		if (isSelectable)
		{
			new_selection.push_back(hit.entity);
		}
	}


	if (!m_queuing && !new_selection.empty())
	{
		m_selection.Clear();
	}

	for (auto u : new_selection)
	{
		m_selection.Add(u);
	}

	EBUS_EVENT(SelectionRequestBus, SetSelectedUnits,m_selection.GetVector());
	
}
