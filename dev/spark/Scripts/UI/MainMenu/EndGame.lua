local EndGame = 
{
	SelectedTab = nil; 
	Properties =
	{
		Overview = 
		{
			ToolTip = {default = EntityId()},
			TimeAndScorePanel = {default = EntityId()},
			HeroSelectionLeft = {default = EntityId()},
			HeroSelectionRight = {default = EntityId()},
			TeamLeft = {default = EntityId()},
			TeamRight = {default = EntityId()},
			
			Time = {default = EntityId()},
			Game = {default = EntityId()},
			GameMode = {default = EntityId()},
			ScoreLeft = {default = EntityId()},
			ScoreRight = {default = EntityId()},
			TeamNameLeft = {default = EntityId()},
			TeamNameRight = {default = EntityId()},
		},
		
		OverviewPanel = {default = EntityId()},
		ScoreBoardPanel = {default = EntityId()},
		GraphsPanel = {default = EntityId()},
	},	
}

function EndGame:OnTick(deltaTime, timePoint)
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
end

function EndGame:OnActivate()	
	self.tickBusHandler = TickBus.Connect(self);
	
	self.canvasNotificationHandler = nil
	
	self.SelectedTab = self.Properties.OverviewPanel
end

function EndGame:OnDeactivate()
	self.tickBusHandler:Disconnect()
	self.canvasNotificationHandler:Disconnect()
end

function EndGame:OnAction(entityId, actionName) 
	if actionName == "Overview" then
		if UiElementBus.Event.IsEnabled(self.Properties.OverviewPanel) == false then
			UiElementBus.Event.SetIsEnabled(self.Properties.OverviewPanel,true)
			UiElementBus.Event.SetIsEnabled(self.SelectedTab,false)
			self.SelectedTab = self.Properties.OverviewPanel
		end
	end
	
	if actionName == "Scoreboard" then
		if UiElementBus.Event.IsEnabled(self.Properties.ScoreBoardPanel) == false then
			UiElementBus.Event.SetIsEnabled(self.Properties.ScoreBoardPanel,true)
			UiElementBus.Event.SetIsEnabled(self.SelectedTab,false)
			self.SelectedTab = self.Properties.ScoreBoardPanel
		end
	end
	
	if actionName == "Graphs" then
		if UiElementBus.Event.IsEnabled(self.Properties.GraphsPanel) == false then
			UiElementBus.Event.SetIsEnabled(self.Properties.GraphsPanel,true)
			UiElementBus.Event.SetIsEnabled(self.SelectedTab,false)
			self.SelectedTab = self.Properties.GraphsPanel
		end
	end
	
	if actionName == "HeroEndGameScreenPressed" then
		local Parent = UiElementBus.Event.GetParent(entityId)
		if UiElementBus.Event.IsEnabled(self.Properties.Overview.TimeAndScorePanel) == true then
			UiElementBus.Event.SetIsEnabled(self.Properties.Overview.TimeAndScorePanel,false)
		end
		if UiElementBus.Event.GetName(Parent) == "TeamLeft" then
			UiElementBus.Event.SetIsEnabled(self.Properties.Overview.TeamLeft,false)
			UiElementBus.Event.SetIsEnabled(self.Properties.Overview.HeroSelectionLeft,true)
		elseif UiElementBus.Event.GetName(Parent) == "TeamRight" then
			UiElementBus.Event.SetIsEnabled(self.Properties.Overview.TeamRight,false)
			UiElementBus.Event.SetIsEnabled(self.Properties.Overview.HeroSelectionRight,true)
		end
	end
	
	if actionName == "BackToTeamView" then
		local Parent = UiElementBus.Event.GetParent(entityId)
		if UiElementBus.Event.GetName(Parent) == "TeamLeft" then
			UiElementBus.Event.SetIsEnabled(self.Properties.Overview.TeamLeft,true)
			UiElementBus.Event.SetIsEnabled(self.Properties.Overview.HeroSelectionLeft,false)
		elseif UiElementBus.Event.GetName(Parent) == "TeamRight" then
			UiElementBus.Event.SetIsEnabled(self.Properties.Overview.TeamRight,true)
			UiElementBus.Event.SetIsEnabled(self.Properties.Overview.HeroSelectionRight,false)
		end
		if UiElementBus.Event.IsEnabled(self.Properties.Overview.HeroSelectionLeft) == false and UiElementBus.Event.IsEnabled(self.Properties.Overview.HeroSelectionRight) == false then
			UiElementBus.Event.SetIsEnabled(self.Properties.Overview.TimeAndScorePanel,true)
		end
	end
end

return EndGame;