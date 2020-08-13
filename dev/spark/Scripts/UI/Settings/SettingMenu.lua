local SettingsMenu = 
{
	Properties =
	{
		HotKeysTab = {default = EntityId()},
		InterfaceTab = {default = EntityId()},
		GameTab = {default = EntityId()},
		VideoTab = {default = EntityId()},
		AudioTab = {default = EntityId()},
	
		SelectedTab = nil; 
		PreviousTab = nil;
		PreviousWindow = nil;
		
		TabMenu = {default = EntityId()},
	},
}
function SettingsMenu:OnTick(deltaTime, timePoint)
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

function SettingsMenu:OnActivate()
	
	--default properties
	self.SelectedTab = "HotKeysSelected";
	self.PreviousTab = "HideHotKeyTab";	
	self.PreviousWindow = self.Properties.HotKeysTab;
	
	self.tickBusHandler = TickBus.Connect(self);
	self.canvasNotificationHandler = nil
	
end

function SettingsMenu:OnDeactivate()
	self.tickBusHandler:Disconnect()
	if self.canvasNotificationHandler then
		self.canvasNotificationHandler:Disconnect()
	end
end

function SettingsMenu:OnAction(entityId, actionName)
   if actionName == "HotKeys" then
		if self.SelectedTab ~= "HotKeysSelected" then
			self:SortHandlingEvents(self.Properties.HotKeysTab, self.PreviousWindow)
			UiAnimationBus.Event.StartSequence(self.CanvasEntityId, self.PreviousTab)
			UiAnimationBus.Event.StartSequence(self.CanvasEntityId, "ShowHotKeyTab")
			self.PreviousTab = "HideHotKeyTab";
			self.PreviousWindow = self.Properties.HotKeysTab
			
			local Text = UiElementBus.Event.GetChild(entityId,0)
			local Title = LocalizationRequestBus.Broadcast.LocalizeString(tostring(UiTextBus.Event.GetText(Text)))
			UiTextBus.Event.SetText(self.Properties.TabMenu, Title)
		end
	self.SelectedTab = "HotKeysSelected";
   end
   if actionName == "Interface" then
		if self.SelectedTab ~= "InterfaceSelected" then
			self:SortHandlingEvents(self.Properties.InterfaceTab, self.PreviousWindow)
			UiAnimationBus.Event.StartSequence(self.CanvasEntityId, self.PreviousTab)
			UiAnimationBus.Event.StartSequence(self.CanvasEntityId, "ShowInterfaceTab")
			self.PreviousTab = "HideInterfaceTab";
			self.PreviousWindow = self.Properties.InterfaceTab
			
			local Text = UiElementBus.Event.GetChild(entityId,0)
			local Title = LocalizationRequestBus.Broadcast.LocalizeString(tostring(UiTextBus.Event.GetText(Text)))
			UiTextBus.Event.SetText(self.Properties.TabMenu, Title)
		end	
	self.SelectedTab = "InterfaceSelected";
   end
   if actionName == "Game" then
		if self.SelectedTab ~= "GameSelected" then
			self:SortHandlingEvents(self.Properties.GameTab, self.PreviousWindow)
			UiAnimationBus.Event.StartSequence(self.CanvasEntityId, self.PreviousTab)
			UiAnimationBus.Event.StartSequence(self.CanvasEntityId, "ShowGameTab")
			self.PreviousTab = "HideGameTab";
			self.PreviousWindow = self.Properties.GameTab
			
			local Text = UiElementBus.Event.GetChild(entityId,0)
			local Title = LocalizationRequestBus.Broadcast.LocalizeString(tostring(UiTextBus.Event.GetText(Text)))
			UiTextBus.Event.SetText(self.Properties.TabMenu, Title)
		end	
	self.SelectedTab = "GameSelected";
   end
   if actionName == "Video" then
		if self.SelectedTab ~= "VideoSelected" then
			self:SortHandlingEvents(self.Properties.VideoTab, self.PreviousWindow)
			UiAnimationBus.Event.StartSequence(self.CanvasEntityId, self.PreviousTab)
			UiAnimationBus.Event.StartSequence(self.CanvasEntityId, "ShowVideoTab")
			self.PreviousTab = "HideVideoTab";
			self.PreviousWindow = self.Properties.VideoTab
			
			local Text = UiElementBus.Event.GetChild(entityId,0)
			local Title = LocalizationRequestBus.Broadcast.LocalizeString(tostring(UiTextBus.Event.GetText(Text)))
			UiTextBus.Event.SetText(self.Properties.TabMenu, Title)
		end
	self.SelectedTab = "VideoSelected";
   end
   if actionName == "Audio" then
		if self.SelectedTab ~= "AudioSelected" then
			self:SortHandlingEvents(self.Properties.AudioTab, self.PreviousWindow)
			UiAnimationBus.Event.StartSequence(self.CanvasEntityId, self.PreviousTab)
			UiAnimationBus.Event.StartSequence(self.CanvasEntityId, "ShowAudioTab")
			self.PreviousTab = "HideAudioTab";
			self.PreviousWindow = self.Properties.AudioTab
			
			local Text = UiElementBus.Event.GetChild(entityId,0)
			local Title = LocalizationRequestBus.Broadcast.LocalizeString(tostring(UiTextBus.Event.GetText(Text)))
			UiTextBus.Event.SetText(self.Properties.TabMenu, Title)
		end
	self.SelectedTab = "AudioSelected";
   end
   
   if actionName == "Exit" then
		local InGameHUD = UiCanvasManagerBus.Broadcast.FindLoadedCanvasByPathName("UI/Canvases/InGameHUD.uicanvas")
		local PickingScreenHUD = UiCanvasManagerBus.Broadcast.FindLoadedCanvasByPathName("UI/Canvases/HeroPickingScreen.uicanvas")
		
		if InGameHUD ~= nil then
			local Settings = UiCanvasBus.Event.FindElementByName(InGameHUD, "Settings")
			UiElementBus.Event.SetIsEnabled(Settings, true)
			UiInteractableBus.Event.SetIsHandlingEvents(Settings, true)
			
			UiCanvasManagerBus.Broadcast.UnloadCanvas(self.CanvasEntityId)
		elseif PickingScreenHUD ~= nil then
			local Settings = UiCanvasBus.Event.FindElementByName(PickingScreenHUD, "Settings")
			UiElementBus.Event.SetIsEnabled(Settings, true)
			UiInteractableBus.Event.SetIsHandlingEvents(Settings, true)
			
			UiCanvasManagerBus.Broadcast.UnloadCanvas(self.CanvasEntityId)	
		end		
   end
    
end

function SettingsMenu:SortHandlingEvents(TabOpenend, TabClosed)
	UiElementBus.Event.SetIsEnabled(TabClosed, false)
	UiInteractableBus.Event.SetIsHandlingEvents(TabClosed, false)
	UiElementBus.Event.SetIsEnabled(TabOpenend, true)
	UiInteractableBus.Event.SetIsHandlingEvents(TabOpenend, true)
end

return SettingsMenu;