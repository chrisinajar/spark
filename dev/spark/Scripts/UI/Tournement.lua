local Tournement = 
{
	CurrentOpenPanel = nil;
	Properties =
	{
		Panels =
		{
			Active = {default = EntityId()},
			Create = {default = EntityId()},
			Join = {default = EntityId()},
			View = {default = EntityId()},
		},
		Active = 
		{
			ActiveTournementBox = {default = EntityId()},
		},
		Create = 
		{
			TournementName = {default = EntityId()},
			TournementFormat = {default = EntityId()},
			TournementPlayerCount = {default = EntityId()},
			TournementRounds = {default = EntityId()},
			TournementPrivacy = {default = EntityId()},			
		},
		Join = 
		{
			JoinTournementBox = {default = EntityId()},
		},
	},
}

function Tournement:OnTick(deltaTime, timePoint)
    -- self.tickBusHandler:Disconnect()
   
    -- Get the canvas entityId
    -- This is done after the OnActivate when the canvas is fully initialized
    self.CanvasEntityId = UiElementBus.Event.GetCanvas(self.entityId)
	
	if( self.CanvasEntityId ) then
		if self.canvasNotificationHandler then
			self.canvasNotificationHandler:Disconnect()
			self.canvasNotificationHandler = nil
		end
	    self.canvasNotificationHandler = UiCanvasNotificationBus.Connect(self, self.CanvasEntityId)			
	end
end

function Tournement:OnActivate()	
	self.tickBusHandler = TickBus.Connect(self)	
	
	self.CurrentOpenPanel = self.Properties.Panels.Active
end

function Tournement:OnDeactivate()
	self.tickBusHandler:Disconnect()
	self.canvasNotificationHandler:Disconnect()
end

function Tournement:CreateTournement()

end

function Tournement:JoinTournement()

end

function Tournement:ViewActiveTournements()

end

function Tournement:ViewBracket()

end

function Tournement:OnAction(entityId, actionName)
	if actionName == "Active" then
		if UiElementBus.Event.IsEnabled(self.Properties.Panels.Active) == false then
			UiElementBus.Event.SetIsEnabled(self.Properties.Panels.Active, true)
			UiElementBus.Event.SetIsEnabled(self.CurrentOpenPanel, false)
			self.CurrentOpenPanel = self.Properties.Panels.Active
		end
	end
	
	if actionName == "Create" then
		if UiElementBus.Event.IsEnabled(self.Properties.Panels.Create) == false then
			UiElementBus.Event.SetIsEnabled(self.Properties.Panels.Create, true)
			UiElementBus.Event.SetIsEnabled(self.CurrentOpenPanel, false)
			self.CurrentOpenPanel = self.Properties.Panels.Create
		end
	end
	
	if actionName == "Join" then
		if UiElementBus.Event.IsEnabled(self.Properties.Panels.Join) == false then
			UiElementBus.Event.SetIsEnabled(self.Properties.Panels.Join, true)
			UiElementBus.Event.SetIsEnabled(self.CurrentOpenPanel, false)
			self.CurrentOpenPanel = self.Properties.Panels.Join
		end
	end
	
	if actionName == "View" then
		if UiElementBus.Event.IsEnabled(self.Properties.Panels.View) == false then
			UiElementBus.Event.SetIsEnabled(self.Properties.Panels.View, true)
			UiElementBus.Event.SetIsEnabled(self.CurrentOpenPanel, false)
			self.CurrentOpenPanel = self.Properties.Panels.View
		end
	end
	
	if actionName == "Exit" then
		UiCanvasManagerBus.Broadcast.UnloadCanvas(self.CanvasEntityId)
	end
end

return Tournement;