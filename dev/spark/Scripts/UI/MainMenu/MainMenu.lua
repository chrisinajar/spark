local MainMenu = 
{
	GameMenuState = nil;
	Properties =
	{
		MainTab = {default = EntityId()},
		WatchTab = {default = EntityId()},
		HeroTab = {default = EntityId()},
		StoreTab = {default = EntityId()},
		LearnTab = {default = EntityId()},
		TempNetworkTab = {default = EntityId()},
	
		textEntered = nil,
		textField = nil,
		SelectedTab = nil; 
		PreviousTab = nil;
		PreviousWindow = nil;
	},	
}

function MainMenu:OnTick(deltaTime, timePoint)
    self.tickBusHandler:Disconnect()

    -- Get the canvas entityId
    -- This is done after the OnActivate when the canvas is fully initialized
    self.canvasEntityId = UiElementBus.Event.GetCanvas(self.entityId)
	
	self.canvasNotificationHandler = UiCanvasNotificationBus.Connect(self, self.canvasEntityId)
	
	UiElementBus.Event.SetIsEnabled(self.Properties.TempNetworkTab, false)
	UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.TempNetworkTab, false)
end

function MainMenu:OnActivate()

	--default properties
	self.SelectedTab = "MainMenuSelected";
	self.PreviousTab = "HideMainMenu";
	self.PreviousWindow = self.Properties.MainTab;
	
	--init states of the tabs
	UiElementBus.Event.SetIsEnabled(self.Properties.WatchTab, false)
	UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.WatchTab, false)
	UiElementBus.Event.SetIsEnabled(self.Properties.HeroTab, false)
	UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.HeroTab, false)
	UiElementBus.Event.SetIsEnabled(self.Properties.StoreTab, false)
	UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.StoreTab, false)
	UiElementBus.Event.SetIsEnabled(self.Properties.LearnTab, false)
	UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.LearnTab, false)
	
	self.tickBusHandler = TickBus.Connect(self);
	
	--set menu state
	self.GameMenuState = "Closed"
	
	self.canvasNotificationHandler = nil
end

function MainMenu:OnDeactivate()
	self.tickBusHandler:Disconnect()
	self.canvasNotificationHandler:Disconnect()
end

function MainMenu:OnAction(entityId, actionName) 
	if actionName == "Start" then
		--[[if self.GameMenuState == "Closed" then
			UiAnimationBus.Event.StartSequence(self.canvasEntityId, "ShowGameMenu")
			self.GameMenuState = "Open"
		end]]
		UiElementBus.Event.SetIsEnabled(self.Properties.TempNetworkTab, true)
		UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.TempNetworkTab, true)
	end
	
	if actionName == "StartSelectedGame" then
		UiElementBus.Event.SetIsEnabled(self.Properties.TempNetworkTab, true)
		UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.TempNetworkTab, true)
	end
	
	if actionName == "GameSelectionBack" or actionName == "OverlayBackButtonPressed" then
		if self.GameMenuState == "Open" then
			UiAnimationBus.Event.StartSequence(self.canvasEntityId, "HideGameMenu")
			self.GameMenuState = "Closed"
		end
	end
	
	if actionName == "OpenSettings" then
		 local optionsCanvasEntityId = UiCanvasManagerBus.Broadcast.LoadCanvas("UI/Canvases/settings.uicanvas")
	end
	
	if actionName == "Heros" then
		if self.SelectedTab ~= "HerosSelected" then
			self:SortHandlingEvents(self.Properties.HeroTab, self.PreviousWindow)
			UiAnimationBus.Event.StartSequence(self.canvasEntityId, self.PreviousTab)
			UiAnimationBus.Event.StartSequence(self.canvasEntityId, "ShowHeroTab")
			self.PreviousTab = "HideHeroTab"
			self.PreviousWindow = self.Properties.HeroTab			 
		end
	self.SelectedTab = "HerosSelected";
	end

	if actionName == "MainMenu" then
		if self.SelectedTab ~= "MainMenuSelected" then
			self:SortHandlingEvents(self.Properties.MainTab, self.PreviousWindow)
			UiAnimationBus.Event.StartSequence(self.canvasEntityId, self.PreviousTab)
			UiAnimationBus.Event.StartSequence(self.canvasEntityId, "ShowMainMenu")
			self.PreviousTab = "HideMainMenu"
			self.PreviousWindow = self.Properties.MainTab
		end
		self.SelectedTab = "MainMenuSelected"
	end	
	
	if actionName == "Watch" then
		if self.SelectedTab ~= "WatchSelected" then
			self:SortHandlingEvents(self.Properties.WatchTab, self.PreviousWindow)
			UiAnimationBus.Event.StartSequence(self.canvasEntityId, self.PreviousTab)
			UiAnimationBus.Event.StartSequence(self.canvasEntityId, "ShowWatchTab")
			self.PreviousTab = "HideWatchTab"
			self.PreviousTab = self.Properties.WatchTab
		end
		self.SelectedTab = "WatchSelected"
	end
	
	if actionName == "Store" then
		if self.SelectedTab ~= "StoreSelected" then
			self:SortHandlingEvents(self.Properties.StoreTab, self.PreviousWindow)
			UiAnimationBus.Event.StartSequence(self.canvasEntityId, self.PreviousTab)
			UiAnimationBus.Event.StartSequence(self.canvasEntityId, "ShowStoreTab")
			self.PreviousTab = "HideStoreTab"
			self.PreviousWindow = self.Properties.StoreTab
		end
		self.SelectedTab = "StoreSelected"
	end
	
	if actionName == "Learn" then
		if self.SelectedTab ~= "LearnSelected" then
			self:SortHandlingEvents(self.Properties.LearnTab, self.PreviousWindow)
			UiAnimationBus.Event.StartSequence(self.canvasEntityId, self.PreviousTab)
			UiAnimationBus.Event.StartSequence(self.canvasEntityId, "ShowLearnTab")
			self.PreviousTab = "HideLearnTab"
			self.PreviousWindow = self.Properties.LearnTab
		end
		self.SelectedTab = "LearnSelected"
	end
	
	if actionName == "Tournement" then
		self.TournementCanvas = UiCanvasManagerBus.Broadcast.LoadCanvas("UI/Canvases/Tournement.uicanvas")
	end
	
	if actionName == "Exit" then
		UiCanvasManagerBus.Broadcast.UnloadCanvas(self.canvasEntityId);
		GameManagerSystemRequestBus.Broadcast.ExecuteConsoleCommand("quit")
	end
	
	if actionName == "NetworkJoinGame" then
		GameManagerSystemRequestBus.Broadcast.ExecuteConsoleCommand("gamelift_join")
	end
	
	if actionName == "NetworkHostGame" then
		GameManagerSystemRequestBus.Broadcast.ExecuteConsoleCommand("gamelift_host test 512map 10")
	end
end


function MainMenu:SortHandlingEvents(TabOpenend, TabClosed)
	UiElementBus.Event.SetIsEnabled(TabClosed, false)
	UiInteractableBus.Event.SetIsHandlingEvents(TabClosed, false)
	UiElementBus.Event.SetIsEnabled(TabOpenend, true)
	UiInteractableBus.Event.SetIsHandlingEvents(TabOpenend, true)
end

return MainMenu;