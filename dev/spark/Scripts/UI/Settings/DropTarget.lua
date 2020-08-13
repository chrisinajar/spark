----------------------------------------------------------------------------------------------------
--
-- All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
-- its licensors.
--
-- For complete copyright and license terms please see the LICENSE at the root of this
-- distribution (the "License"). All use of this software is governed by the License,
-- or, if provided, by the license below or the license accompanying this file. Do not
-- remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
--
--
----------------------------------------------------------------------------------------------------

local DropTarget = 
{
	
}

function DropTarget:OnActivate()
	self.dropTargetHandler = UiDropTargetNotificationBus.Connect(self, self.entityId)
end

function DropTarget:OnDeactivate()
	self.dropTargetHandler:Disconnect()
end

-- On Drop Hover Start
function DropTarget:OnDropHoverStart(draggable)
		UiDraggableBus.Event.SetDragState(draggable, eUiDragState_Valid)
		UiDropTargetBus.Event.SetDropState(self.entityId, eUiDropState_Valid)
end

-- On Drop Hover End
function DropTarget:OnDropHoverEnd(draggable)
	UiDraggableBus.Event.SetDragState(draggable, eUiDragState_Normal)
	UiDropTargetBus.Event.SetDropState(self.entityId, eUiDropState_Normal)
	Debug.Log("DROP HOVER END")
end

-- On Drop
function DropTarget:OnDrop(draggable)
	Debug.Log("GET CANVAS POS IS : " .. tostring(UiTransformBus.Event.GetCanvasPosition(draggable)))
	Debug.Log("GET LOCAL POS IS : " .. tostring(UiTransformBus.Event.GetLocalPosition(draggable)))
	local name = tostring(UiElementBus.Event.GetName(draggable))
	
	SettingsRequestBus.Broadcast.SetSettingValue("AdvancedLocation" .. name,tostring(UiTransformBus.Event.GetCanvasPosition(draggable)))
	
	if (UiElementBus.Event.GetNumChildElements(self.entityId) <= 0) then
		--UiElementBus.Event.Reparent(draggable, self.entityId, EntityId())
	end
end

return DropTarget