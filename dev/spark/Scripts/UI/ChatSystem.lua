local ChatSystem = 
{
	textEntered = nil;
	spawnTicket = nil;
	Properties =
	{		
		textField = {default = EntityId()},
		contentArea = {default = EntityId()},
	},
}

function ChatSystem:OnTick(deltaTime, timePoint)
	self.CanvasEntityId = UiElementBus.Event.GetCanvas(self.entityId)
	if self.CanvasEntityId then
		if self.canvasNotificationHandler then
			self.canvasNotificationHandler:Disconnect()
			self.canvasNotificationHandler = nil
		end
	    self.canvasNotificationHandler = UiCanvasNotificationBus.Connect(self, self.CanvasEntityId)
	end	
end

function ChatSystem:OnActivate()
	self.tickBusHandler = TickBus.Connect(self);
	self.chatHandler = SparkChatNotificationBus.Connect(self);
	self.spawnTicket = nil
end

function ChatSystem:OnAction(entityId, actionName) 
	if actionName == "Send" or actionName == "EnterSend" then
		--Send Text
		self.textEntered = UiTextInputBus.Event.GetText(self.Properties.textField)
		self:PutTextIntoChat(tostring(self.textEntered))
	end
end

function ChatSystem:OnDeactivate()
	self.tickBusHandler:Disconnect()
	self.canvasNotificationHandler:Disconnect()
end

function ChatSystem:UpdateChat()
	local Children = UiElementBus.Event.GetChildren(self.Properties.contentArea)
	if Children ~= nil then
		for i = 1, #Children do 
			Debug.Log(tostring(UiElementBus.Event.GetName(Children[i])))
			local Offset = UiTransform2dBus.Event.GetOffsets(Children[i])
			Offset.top = Offset.top - 15
			UiTransform2dBus.Event.SetOffsets(Children[i], Offset)
		end
	end
end

function ChatSystem:PutTextIntoChat(msg)
	SparkChatRequestBus.Broadcast.SendChatMessage(ChatMessage(msg,"Bard"))	
	
	local ChatText = UiElementBus.Event.FindDescendantByName(self.Properties.textField,"ChatText")
	local PlaceholderText = UiElementBus.Event.FindDescendantByName(self.Properties.textField,"PlaceholderText")
	UiTextBus.Event.SetText(ChatText,"")
	UiElementBus.Event.SetIsEnabled(PlaceholderText,false)
end

function ChatSystem:OnNewMessage(msg)
	self.spawnerHandler = UiSpawnerNotificationBus.Connect(self, self.Properties.contentArea)
	self:UpdateChat()
	self.spawnTicket = UiSpawnerBus.Event.Spawn(self.Properties.contentArea)
end

function ChatSystem:OnTopLevelEntitiesSpawned(ticket, ids)
	if ticket == self.spawnTicket then
		local Author = UiElementBus.Event.FindDescendantByName(ids[1],"Author")
		local Message = UiElementBus.Event.FindDescendantByName(ids[1],"Message")
		local LastMessage = SparkChatRequestBus.Broadcast.GetLastMessage()
		
		UiTextBus.Event.SetText(UiElementBus.Event.GetChild(Author,0), LastMessage:GetAuthor())
		UiTextBus.Event.SetText(UiElementBus.Event.GetChild(Message,0), LastMessage:GetText())		
	end
end

function ChatSystem:OnSpawnEnd(ticket)
	if ticket == self.spawnTicket then
		self.spawnTicket = nil
	end	
end

function ChatSystem:OnSpawnFailed(ticket)
	if ticket == self.spawnTicket then
		self.spawnerHandler:Disconnect()
		self.spawnTicket = nil
	end	
end

return ChatSystem;