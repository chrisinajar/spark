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

local DropTargetShop = 
{
	selectedUnit = nil;
	Slot = nil;
	Items = {};
    Properties = 
    {
	
	},
}

function DropTargetShop:OnTick(deltaTime, timePoint)
    --self.tickBusHandler:Disconnect()
	self.CanvasEntityId = UiElementBus.Event.GetCanvas(self.entityId)
	if( self.CanvasEntityId ) then
		if self.canvasNotificationHandler then
			self.canvasNotificationHandler:Disconnect()
			self.canvasNotificationHandler = nil
		end
	    self.canvasNotificationHandler = UiCanvasNotificationBus.Connect(self, self.CanvasEntityId)
	end
end

function DropTargetShop:OnActivate()
	self.dropTargetHandler = UiDropTargetNotificationBus.Connect(self, self.entityId)
	self.tickBusHandler = TickBus.Connect(self);
end

function DropTargetShop:OnDeactivate()
	self.tickBusHandler:Disconnect()
	self.canvasNotificationHandler:Disconnect()
	self.dropTargetHandler:Disconnect()
end

-- On Drop Shop Hover Start
function DropTargetShop:OnDropHoverStart(draggable)
	self.selectedUnit = SelectionRequestBus.Broadcast.GetMainSelectedUnit()
	if self.selectedUnit == SelectionRequestBus.Broadcast.GetMainSelectedUnit() then
		for i=0,9 do 
			local Slot = UiElementBus.Event.GetChild(self.entityId,i)
			local Child = UiElementBus.Event.GetChild(Slot,0)
		end
			UiDraggableBus.Event.SetDragState(draggable, eUiDragState_Valid)
			UiDropTargetBus.Event.SetDropState(self.entityId, eUiDropState_Valid)
	end
end

-- On Drop Shop Hover End
function DropTargetShop:OnDropHoverEnd(draggable)
	UiDraggableBus.Event.SetDragState(draggable, eUiDragState_Normal)
	UiDropTargetBus.Event.SetDropState(self.entityId, eUiDropState_Normal)
end

-- On Shop Drop
function DropTargetShop:OnDrop(draggable)	
	local QuickBuyList = vector_basic_string_char_char_traits_char()
	local ItemText = UiElementBus.Event.FindDescendantByName(draggable, "Text")
	local Text = UiTextBus.Event.GetText(ItemText)
	
	-- Detect Drop For Quick Buy Items On Drop
	for i=0,9 do 
		local Slot = UiElementBus.Event.GetChild(self.entityId,i)
		local Child = UiElementBus.Event.GetChild(Slot,0)
		local QBItemText = UiElementBus.Event.FindChildByName(Child, "Text")
		local QBText = UiTextBus.Event.GetText(QBItemText)
		if QBText == nil or QBText == "TEST" then
				UiTextBus.Event.SetText(QBItemText, tostring(Text))
			break
		end
	end
	
	local Items = self.Items
	for i=0, 5 do		
		local ItemRef = require "scripts.ui.maingame.shopitems"
		self.Items[i] = ItemRef.Items[i]
	end

	QuickBuyList:push_back(Text);
	UnitRequestBus.Event.SetQuickBuyItemList(self.selectedUnit,QuickBuyList);
	
	local itemList = UnitRequestBus.Event.GetQuickBuyItemList(self.selectedUnit);
end

return DropTargetShop