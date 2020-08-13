local Party = 
{
	PartySize = 1;
	Properties =
	{
		Friend1 = {default = EntityId()},
		Friend2 = {default = EntityId()},
		Friend3 = {default = EntityId()},
		Friend4 = {default = EntityId()},
	},
	
}

function Party:OnTick(deltaTime, timePoint)
	self.tickBusHandler:Disconnect()
	
	self.CanvasEntityId = UiElementBus.Event.GetCanvas(self.entityId)
	
	self.canvasNotificationHandler = UiCanvasNotificationBus.Connect(self, self.CanvasEntityId)
	
	UiElementBus.Event.SetIsEnabled(UiElementBus.Event.FindDescendantByName(self.Properties.Friend1, "Friend"), false)
	UiElementBus.Event.SetIsEnabled(UiElementBus.Event.FindDescendantByName(self.Properties.Friend2, "Friend"), false)
	UiElementBus.Event.SetIsEnabled(UiElementBus.Event.FindDescendantByName(self.Properties.Friend3, "Friend"), false)
	UiElementBus.Event.SetIsEnabled(UiElementBus.Event.FindDescendantByName(self.Properties.Friend4, "Friend"), false)
end

function Party:OnActivate()
	

	self.tickBusHandler = TickBus.Connect(self);
	
	
	
	
	
end

function Party:OnDeactivate()
	self.tickBusHandler:Disconnect()
end

function Party:AddFriendToParty()

end

function Party:KickFromParty()

end

function Party:FriendJoinedParty()
	self:UpdateParty();
end

function Party:UpdateParty()

end

return Party;