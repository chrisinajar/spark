
local chat = {}

function chat:OnActivate()
	--Debug.Log("chat OnActivate");
	self.focus = false;
	
	--not required, is set like this by default
	--InputMapperRequestBus.Broadcast.BindHotKeyToSlot(Hotkey("Enter"),Slot(Slot.Chat,0));
	
	self.chatHandler = SparkChatNotificationBus.Connect(self);
	self.slotHandler = SlotNotificationBus.Connect(self)
	
	--SparkChatRequestBus.Broadcast.SendChatMessage(ChatMessage("hello, world!","author"))
	
end

function chat:OnDeactivate()
	self.chatHandler:Disconnect()
end


function chat:OnNewMessage(msg)
	--Debug.Log("chat:OnNewMessage() : "..tostring(msg:ToString()))
end

function chat:OnSlotPressed(slot)
	--Debug.Log("OnSlotPressed : "..tostring(slot:ToString()))
	
	if(slot:GetType() == Slot.Chat) then
		self.focus = not self.focus
		
		--Debug.Log("chat! focus:"..tostring(self.focus));
		
		--SparkCameraRequestBus.Broadcast.SetFocus(not self.focus);
		--Debug.Log("camera focus:"..tostring(SparkCameraRequestBus.Broadcast.HasFocus()))
	end
end


return chat;