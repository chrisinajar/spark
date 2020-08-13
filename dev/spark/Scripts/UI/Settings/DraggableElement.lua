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

local DraggableElement = 
{
    Properties = 
    {
	
	},
}

function DraggableElement:OnActivate()
	self.draggableHandler = UiDraggableNotificationBus.Connect(self, self.entityId)
end

function DraggableElement:OnDeactivate()
	self.draggableHandler:Disconnect()
end

-- On Drag Start
function DraggableElement:OnDragStart(position)
	self.originalPosition = UiTransformBus.Event.GetViewportPosition(self.entityId)
	self.originalParent = UiElementBus.Event.GetParent(self.entityId)		
end

-- On Drag
function DraggableElement:OnDrag(position)
    UiTransformBus.Event.SetViewportPosition(self.entityId, position)
end

-- On Drag End
function DraggableElement:OnDragEnd(position)
	
end

return DraggableElement