require "scripts/library/timers"

local InventorySlot = 
{
	selectedUnit = nil;
	SlotId = nil;
	AbilityInSlot = nil;
	Slice = nil;
	Properties =
	{
		InventoryPanel = {default = EntityId()},
	},
}

function InventorySlot:OnActivate()
	self.tickBusHandler = TickBus.Connect(self);
	
	CreateTimer(function()
		self:Init()
	end,0.1)
end

function InventorySlot:OnDeactivate()
	self.tickBusHandler:Disconnect()
	self.selectionHandler:Disconnect()
end

function InventorySlot:OnTick(deltaTime, timePoint)
    --self.tickBusHandler:Disconnect()		
	self:Update()
end

function InventorySlot:OnAbilityAttached(abilityId) 

end

function InventorySlot:OnAbilityDetached(abilityId)

end

function InventorySlot:Init()
	self.CanvasEntityId = UiElementBus.Event.GetCanvas(self.entityId)
	if( self.CanvasEntityId ) then
	    self.canvasNotificationHandler = self.canvasNotificationHandler or UiCanvasNotificationBus.Connect(self, self.CanvasEntityId)
	end
	self.selectionHandler = SelectionNotificationBus.Connect(self)
	
	--self.UnitAbilityHandler = UnitAbilityNotificationBus.Connect(self, self.selectedUnit)
	self.SlotId = UiElementBus.Event.GetIndexOfChildByEntityId(self.Properties.InventoryPanel, self.entityId)
	
	self:InitUnit()
	
	if self.Slice == nil or self.Slice ~= UiElementBus.Event.GetChild(self.entityId, 0) then
		self.Slice = UiElementBus.Event.GetChild(self.entityId, 0)
	end
end

function InventorySlot:InitUnit()
	self.selectedUnit = SelectionRequestBus.Broadcast.GetMainSelectedUnit()
	self:OnMainSelectedUnitChanged(self.selectedUnit)
end

function InventorySlot:OnMainSelectedUnitChanged(unit)
	self.selectedUnit = unit
end

-- Update Inventory If Item Is In Slot
function InventorySlot:Update()
	if self.selectedUnit ~= nil and self.selectedUnit == SelectionRequestBus.Broadcast.GetMainSelectedUnit() then
		if self.SlotId ~= nil then
			self.AbilityInSlot = UnitAbilityRequestBus.Event.GetAbilityInSlot(self.selectedUnit,Slot(Slot.Inventory,self.SlotId))
			if self.AbilityInSlot ~= nil and self.AbilityInSlot:IsValid() then
				UiElementBus.Event.SetIsEnabled(self.Slice, true)
			else
				UiElementBus.Event.SetIsEnabled(self.Slice, false)
			end
		end
	end
end


return InventorySlot;