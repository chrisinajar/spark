local HotKeys = 
{
	Text = nil;
	Button = nil;
	ButtonHoveredIndex = nil;
	abilityCount = nil;
	currentlSlot = nil;
	Properties =
	{
		AbilityKeyBinds = 
		{
			AbilityKeyBindPanel = {default = EntityId()},
			AbilityButton = {default = {EntityId(),EntityId(),EntityId(),EntityId(),EntityId(),EntityId()}},
		},
		
		ItemKeyBinds = 
		{
			ItemKeyBindPanel = {default = EntityId()},
			ItemButton = {default = {EntityId(),EntityId(),EntityId(),EntityId(),EntityId(),EntityId()}},
		},
		
	},
}
function HotKeys:OnTick(deltaTime, timePoint)
    --self.tickBusHandler:Disconnect()

    -- Get the canvas entityId
    -- This is done after the OnActivate when the canvas is fully initialized
    self.canvasEntityId = UiElementBus.Event.GetCanvas(self.entityId)
	
	if( self.canvasEntityId ) then
		if self.canvasNotificationHandler then
			self.canvasNotificationHandler:Disconnect()
			self.canvasNotificationHandler = nil
		end
	    self.canvasNotificationHandler = UiCanvasNotificationBus.Connect(self, self.canvasEntityId)			
	end
	
	self:Init()
	
end
function HotKeys:OnActivate()
	self.handler = InputMapperNotificationBus.Connect(self);	
	self.canvasNotificationHandler = nil
	
	self.tickBusHandler = TickBus.Connect(self);
end

function HotKeys:OnDeactivate()
	self.tickBusHandler:Disconnect()
	self.canvasNotificationHandler:Disconnect()
end

function HotKeys:Init()
	for i=0,5 do
		key = InputMapperRequestBus.Broadcast.GetSlotHotkey(Slot(Slot.Ability,i))
		self.Button = self.Properties.AbilityKeyBinds.AbilityButton[i]
		self.Text = UiElementBus.Event.FindChildByName(self.Button, "Text")
		local keys=key:GetKeys();
		for j=1, #keys do
			if j == 2 then
				UiTextBus.Event.SetText(self.Text, KeyToString(keys[2]) .. "\n" .. KeyToString(keys[1]))				
			else 
				UiTextBus.Event.SetText(self.Text, KeyToString(keys[j]))
			end
			
		end	
	end
	
	for i=0,5 do
		key = InputMapperRequestBus.Broadcast.GetSlotHotkey(Slot(Slot.Inventory,i))
		self.Button = self.Properties.ItemKeyBinds.ItemButton[i]
		self.Text = UiElementBus.Event.FindChildByName(self.Button, "Text")
		local keys=key:GetKeys();
		for j=1, #keys do
			if j == 2 then
				UiTextBus.Event.SetText(self.Text, KeyToString(keys[2]) .. "\n" .. KeyToString(keys[1]))				
			else 
				UiTextBus.Event.SetText(self.Text, KeyToString(keys[j]))
			end
			
		end	
	end
	
end

function HotKeys:OnDeactivate()
	self.tickBusHandler:Disconnect()
	self.canvasNotificationHandler:Disconnect()
end

function HotKeys:OnAction(entityId, actionName) 

	if actionName == "KeyBindOptionPressed" then
		if UiElementBus.Event.GetIndexOfChildByEntityId(self.Properties.AbilityKeyBinds.AbilityKeyBindPanel, entityId) then
			self.ButtonHoveredIndex = UiElementBus.Event.GetIndexOfChildByEntityId(self.Properties.AbilityKeyBinds.AbilityKeyBindPanel, entityId)
			self.currentlSlot = "Slot.Ability"
		elseif UiElementBus.Event.GetIndexOfChildByEntityId(self.Properties.ItemKeyBinds.ItemKeyBindPanel, entityId) then
			self.ButtonHoveredIndex = UiElementBus.Event.GetIndexOfChildByEntityId(self.Properties.ItemKeyBinds.ItemKeyBindPanel, entityId)
			self.currentlSlot = "Slot.Inventory"
		end
		InputMapperRequestBus.Broadcast.StartHotkeyRegistration();
	end
end

function HotKeys:OnHotkeyRegistrationStart()

end

function HotKeys:OnHotkeyRegistrationChanged(hotkey)

end

function HotKeys:OnHotkeyRegistrationDone(hotkey)
	if self.currentlSlot == "Slot.Ability" then
		InputMapperRequestBus.Broadcast.BindHotKeyToSlot(hotkey,Slot(Slot.Ability,self.ButtonHoveredIndex));
	elseif self.currentlSlot == "Slot.Inventory" then
		InputMapperRequestBus.Broadcast.BindHotKeyToSlot(hotkey,Slot(Slot.Inventory,self.ButtonHoveredIndex));
	end
	
	self:UpdateHotKeys(self.currentlSlot)
end

function HotKeys:UpdateHotKeys(CurrentSlot)	
	if CurrentSlot == "Slot.Ability" then
		Newkey = InputMapperRequestBus.Broadcast.GetSlotHotkey(Slot(Slot.Ability,self.ButtonHoveredIndex))
		self.Button = UiElementBus.Event.GetChild(self.Properties.AbilityKeyBinds.AbilityKeyBindPanel, self.ButtonHoveredIndex)
	elseif CurrentSlot == "Slot.Inventory" then
		Newkey = InputMapperRequestBus.Broadcast.GetSlotHotkey(Slot(Slot.Inventory,self.ButtonHoveredIndex))
		self.Button = UiElementBus.Event.GetChild(self.Properties.ItemKeyBinds.ItemKeyBindPanel, self.ButtonHoveredIndex)
	end
	self.Text = UiElementBus.Event.FindChildByName(self.Button, "Text")
	local keys=Newkey:GetKeys();
	for h=1, #keys do
		Debug.Log(KeyToString(keys[h]));
		if h == 2 then
			UiTextBus.Event.SetText(self.Text, KeyToString(keys[2]) .. "\n" .. KeyToString(keys[1]))
		else 
			UiTextBus.Event.SetText(self.Text, KeyToString(keys[h]))
		end
				
	end	
end

return HotKeys;