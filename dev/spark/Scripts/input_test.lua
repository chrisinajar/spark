
local input_test = {}

function input_test:OnActivate()
	Debug.Log("input_test OnActivate");
	--[[
	self.handler = InputMapperNotificationBus.Connect(self);
	
	--InputMapperRequestBus.Broadcast.StartHotkeyRegistration();
	
	local hotkey=Hotkey("Ctrl+Q");
	local hotkey2=Hotkey("W");
	local hotkey3=Hotkey("E");
	local hotkey4=Hotkey("R");
	local hotkey5=Hotkey("T");
	local hotkey6=Hotkey("Y");
	
	local hotkey7=Hotkey("Z");
	local hotkey8=Hotkey("X");
	local hotkey9=Hotkey("C");
	local hotkey10=Hotkey("V");
	local hotkey11=Hotkey("B");
	local hotkey12=Hotkey("N");
	
	Debug.Log(hotkey:ToString());
	Debug.Log("single keys:");
	local keys=hotkey:GetKeys();
	for i=1, #keys do
	    Debug.Log(KeyToString(keys[i]));
	end
		
	InputMapperRequestBus.Broadcast.BindHotKeyToSlot(hotkey,Slot(Slot.Ability,0));
	InputMapperRequestBus.Broadcast.BindHotKeyToSlot(hotkey2,Slot(Slot.Ability,1));
	InputMapperRequestBus.Broadcast.BindHotKeyToSlot(hotkey3,Slot(Slot.Ability,2));
	InputMapperRequestBus.Broadcast.BindHotKeyToSlot(hotkey4,Slot(Slot.Ability,3));
	InputMapperRequestBus.Broadcast.BindHotKeyToSlot(hotkey5,Slot(Slot.Ability,4));
	InputMapperRequestBus.Broadcast.BindHotKeyToSlot(hotkey6,Slot(Slot.Ability,5));
	
	InputMapperRequestBus.Broadcast.BindHotKeyToSlot(hotkey7,Slot(Slot.Inventory,0));
	InputMapperRequestBus.Broadcast.BindHotKeyToSlot(hotkey8,Slot(Slot.Inventory,1));
	InputMapperRequestBus.Broadcast.BindHotKeyToSlot(hotkey9,Slot(Slot.Inventory,2));
	InputMapperRequestBus.Broadcast.BindHotKeyToSlot(hotkey10,Slot(Slot.Inventory,3));
	InputMapperRequestBus.Broadcast.BindHotKeyToSlot(hotkey11,Slot(Slot.Inventory,4));
	InputMapperRequestBus.Broadcast.BindHotKeyToSlot(hotkey12,Slot(Slot.Inventory,5));
	
	hotkey = InputMapperRequestBus.Broadcast.GetSlotHotkey(Slot(Slot.Ability,2));
	Debug.Log(hotkey:ToString());
	
	Debug.Log("done");
	--]]
end



return input_test;