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

local DraggableElementShop = 
{
	IndexTest = nil;
	Child = nil;
	ItemPanel = nil;
}

function DraggableElementShop:OnActivate()
	self.draggableHandler = UiDraggableNotificationBus.Connect(self, self.entityId)
end

function DraggableElementShop:OnDeactivate()
	self.draggableHandler:Disconnect()
end

-- Shop On Drag Start
function DraggableElementShop:OnDragStart(position)  
	self.ItemPanel = UiElementBus.Event.GetParent(self.entityId)
	local ItemSlot = UiElementBus.Event.GetIndexOfChildByEntityId(self.ItemPanel, self.entityId)
	
	self.Child = UiElementBus.Event.GetChild(self.ItemPanel,0)
	local ChildImage = UiElementBus.Event.FindDescendantByName(self.Child, "Image")
		
	-- Set Image In Place Of Drag Element
	local Image = UiElementBus.Event.GetChild(self.ItemPanel,1)
	UiImageBus.Event.SetSpritePathname(Image, UiImageBus.Event.GetSpritePathname(ChildImage))
	
	UiElementBus.Event.SetIsEnabled(Image, true)
	self.IndexTest = ItemSlot

	self.originalPosition = UiTransformBus.Event.GetViewportPosition(self.entityId)
	self.originalParent = UiElementBus.Event.GetParent(self.entityId)	
end

-- Shop On Drag
function DraggableElementShop:OnDrag(position)
    UiTransformBus.Event.SetViewportPosition(self.entityId, position)
end

-- Shop Drag End
function DraggableElementShop:OnDragEnd(position)
	UiTransformBus.Event.SetViewportPosition(self.entityId, self.originalPosition)
end

return DraggableElementShop