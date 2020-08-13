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
	selectedUnit = nil;
	InventoryPanel = nil;
	NewIndex = nil;
	OldSlot = nil;
}

function DropTarget:OnActivate()
	self.dropTargetHandler = UiDropTargetNotificationBus.Connect(self, self.entityId)
end

function DropTarget:OnDeactivate()
	self.dropTargetHandler:Disconnect()
end

-- On Drop Hover Start
function DropTarget:OnDropHoverStart(draggable)
	self.selectedUnit = SelectionRequestBus.Broadcast.GetMainSelectedUnit()
	if self.selectedUnit == SelectionRequestBus.Broadcast.GetMainSelectedUnit() then		
		UiDraggableBus.Event.SetDragState(draggable, eUiDragState_Valid)
		UiDropTargetBus.Event.SetDropState(self.entityId, eUiDropState_Valid)

		self.InventoryPanel = UiElementBus.Event.GetParent(self.entityId)
		self.NewIndex = UiElementBus.Event.GetIndexOfChildByEntityId(self.InventoryPanel, self.entityId)
	end
end

-- On Drop Hover End
function DropTarget:OnDropHoverEnd(draggable)
	UiDraggableBus.Event.SetDragState(draggable, eUiDragState_Normal)
	UiDropTargetBus.Event.SetDropState(self.entityId, eUiDropState_Normal)
end

-- Swap Inventory Slots
function DropTarget:SwapInventorySlot(index0,index1) 
	if not self.selectedUnit or not self.selectedUnit:IsValid() then return end

	local a = UnitAbilityRequestBus.Event.GetAbilityInSlot(self.selectedUnit,Slot(Slot.Inventory,index0));
	local b = UnitAbilityRequestBus.Event.GetAbilityInSlot(self.selectedUnit,Slot(Slot.Inventory,index1));
	
	UnitAbilityRequestBus.Event.SetAbilityInSlot(self.selectedUnit,Slot(Slot.Inventory,index1),a);
	UnitAbilityRequestBus.Event.SetAbilityInSlot(self.selectedUnit,Slot(Slot.Inventory,index0),b);
end

-- On Drop
function DropTarget:OnDrop(draggable)
	if(self.NewIndex == nil)then return; end

	local ParentSlot = UiElementBus.Event.GetParent(draggable)
	local ParentSlotParent = nil;

	if(ParentSlot and ParentSlot:IsValid()) then
		ParentSlotParent=UiElementBus.Event.GetParent(ParentSlot)
	end

	if(ParentSlotParent and ParentSlotParent:IsValid()) then
		local oldIndex = UiElementBus.Event.GetIndexOfChildByEntityId(ParentSlotParent, ParentSlot)

		--Swap In Game Logic
		self:SwapInventorySlot(self.NewIndex,oldIndex);
		
		--Swap GUI Elements
		if self.selectedUnit == SelectionRequestBus.Broadcast.GetMainSelectedUnit() then
			if (UiElementBus.Event.GetNumChildElements(self.entityId) <= 0) then
				UiElementBus.Event.Reparent(draggable, self.entityId, EntityId())
			elseif (UiElementBus.Event.GetNumChildElements(self.entityId) > 0) then
				local oldslot = UiElementBus.Event.GetChild(self.entityId, 0)
			end			
		end
	end
	self.NewIndex=nil;
end

return DropTarget