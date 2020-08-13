local Interface = 
{
	init = false;
	Properties =
	{
		MiniMapLocation = {default = EntityId()},
		SimpleColors = {default = EntityId()},
		AltToShowHero = {default = EntityId()},
		InvertAlt = {default = EntityId()},
		MiniMapHeroSize = {default = EntityId()},
		
		DisplaySteamAsWisper = {default = EntityId()},
		MuteAllChat = {default = EntityId()},
		MuteAllEnemyChat = {default = EntityId()},
		
		DisableStatusText = {default = EntityId()},
		HideDamage = {default = EntityId()},
		ColorBlind = {default = EntityId()},
		DifferentiateAllyHealthBars = {default = EntityId()},
		HideEnemyManaBar = {default = EntityId()},
		HUDScale = {default = EntityId()},
		
		AdvancedHUDLayout = {default = EntityId()},
		
		HeroName = {default = EntityId()},
		SteamName = {default = EntityId()},
		Nothing = {default = EntityId()},
		
		Texts = {
			HUDScaleText = {default = EntityId()},
			MiniMapHeroSizeText = {default = EntityId()},
		},
	},
}
function Interface:OnTick(deltaTime, timePoint)
   -- self.tickBusHandler:Disconnect()

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
	if self.init == false then
		self:LoadSettings()
	end
	
end
function Interface:OnActivate()
	self.tickBusHandler = TickBus.Connect(self);
	self.canvasNotificationHandler = nil	
end

function Interface:OnDeactivate()
	self.tickBusHandler:Disconnect()
	self.canvasNotificationHandler:Disconnect()
end

function Interface:OnAction(entityId, actionName) 
  	if actionName == "DefaultSettings" then
		self:DefaultSettings()
	end
	
	if actionName == "MiniMapRight" then
		--Set Mini Map Location to be on the right
		SettingsRequestBus.Broadcast.SetSettingValue("minimapLocation","right","Interface")
	end
	
	if actionName == "MiniMapLeft" then
		--Set Mini Map Location to be on the left
		SettingsRequestBus.Broadcast.SetSettingValue("minimapLocation","left","Interface")	
	end
	
	if actionName == "SimpleColorsChanged" then		
		SettingsRequestBus.Broadcast.SetSettingValue("UseSimplifiedColorsInMiniMap",tostring(UiCheckboxBus.Event.GetState(self.Properties.SimpleColors)),"Interface")	
	end
	
	if actionName == "AltToShowHeroIcon" then
		SettingsRequestBus.Broadcast.SetSettingValue("AltToShowHeroIcon",tostring(UiCheckboxBus.Event.GetState(self.Properties.AltToShowHero)),"Interface")	
	end
	
	if actionName == "InvertAltToggled" then
		SettingsRequestBus.Broadcast.SetSettingValue("InvertAltToggled",tostring(UiCheckboxBus.Event.GetState(self.Properties.InvertAlt)),"Interface")	
	end
	
	if actionName == "MiniMapHeroSizeChanged" then
		local Parent = UiElementBus.Event.GetParent(entityId)
		local Text = UiElementBus.Event.FindDescendantByName(Parent,"Text")
		SettingsRequestBus.Broadcast.SetSettingValue("MiniMapHeroSize",tostring(UiSliderBus.Event.GetValue(self.Properties.MiniMapHeroSize)),"Interface")
		UiTextBus.Event.SetText(Text,tostring(UiSliderBus.Event.GetValue(self.Properties.MiniMapHeroSize)))
	end
	
	if actionName == "DisplaySteamMessageAsWisper" then
		SettingsRequestBus.Broadcast.SetSettingValue("DisplaySteamMessageAsWisper",tostring(UiCheckboxBus.Event.GetState(self.Properties.DisplaySteamAsWisper)),"Interface")	
	end
	
	if actionName == "MuteAllChat" then
		SettingsRequestBus.Broadcast.SetSettingValue("MuteAllChat",tostring(UiCheckboxBus.Event.GetState(self.Properties.MuteAllChat)),"Interface")
	end
	
	if actionName == "MuteAllEnemyChat" then
		SettingsRequestBus.Broadcast.SetSettingValue("MuteAllEnemyChat",tostring(UiCheckboxBus.Event.GetState(self.Properties.MuteAllEnemyChat)),"Interface")	
	end
	
	if actionName == "StatusText" then
		SettingsRequestBus.Broadcast.SetSettingValue("DisableStatusText",tostring(UiCheckboxBus.Event.GetState(self.Properties.DisableStatusText)),"Interface")	
	end
	
	if actionName == "DamageNumbers" then
		SettingsRequestBus.Broadcast.SetSettingValue("HideDamageNumbers",tostring(UiCheckboxBus.Event.GetState(self.Properties.HideDamage)),"Interface")
	end
	
	if actionName == "HUDScaleChanged" then
		local Parent = UiElementBus.Event.GetParent(entityId)
		local Text = UiElementBus.Event.FindDescendantByName(Parent,"Text")
		SettingsRequestBus.Broadcast.SetSettingValue("HUDScale",tostring(UiSliderBus.Event.GetValue(self.Properties.HUDScale)),"Interface")
		UiTextBus.Event.SetText(Text,tostring(UiSliderBus.Event.GetValue(self.Properties.HUDScale)))
	end
	
	if actionName == "ColorBlindMode" then
		SettingsRequestBus.Broadcast.SetSettingValue("ColorBlindMode",tostring(UiCheckboxBus.Event.GetState(self.Properties.ColorBlind)),"Interface")	
	end
	
	if actionName == "DifferentiateAllyHealthBars" then
		SettingsRequestBus.Broadcast.SetSettingValue("DifferentiateAllyHealthBars" ,tostring(UiCheckboxBus.Event.GetState(self.Properties.DifferentiateAllyHealthBars)),"Interface")	
	end
	
	if actionName == "HideEnemyManaBars" then
		SettingsRequestBus.Broadcast.SetSettingValue("HideEnemyManaBars",tostring(UiCheckboxBus.Event.GetState(self.Properties.HideEnemyManaBar)),"Interface")	
	end	
	
	if actionName == "AdvancedHUD" then
		SettingsRequestBus.Broadcast.SetSettingValue("AdvancedHUDLayout",tostring(UiCheckboxBus.Event.GetState(self.Properties.AdvancedHUDLayout)),"Interface")	
	end
	
	if actionName == "HeroName" then
		SettingsRequestBus.Broadcast.SetSettingValue("HeroName",tostring(UiCheckboxBus.Event.GetState(self.Properties.HeroName)), "Interface")
		UiCheckboxBus.Event.SetState(self.Properties.SteamName, false)
		UiCheckboxBus.Event.SetState(self.Properties.Nothing, false)
		SettingsRequestBus.Broadcast.SetSettingValue("SteamName",tostring(UiCheckboxBus.Event.GetState(self.Properties.SteamName)), "Interface")
		SettingsRequestBus.Broadcast.SetSettingValue("NothingName",tostring(UiCheckboxBus.Event.GetState(self.Properties.Nothing)), "Interface")
	end
	
	if actionName == "SteamName" then
		SettingsRequestBus.Broadcast.SetSettingValue("SteamName",tostring(UiCheckboxBus.Event.GetState(self.Properties.SteamName)), "Interface")
		UiCheckboxBus.Event.SetState(self.Properties.HeroName, false)
		UiCheckboxBus.Event.SetState(self.Properties.Nothing, false)
		SettingsRequestBus.Broadcast.SetSettingValue("HeroName",tostring(UiCheckboxBus.Event.GetState(self.Properties.HeroName)), "Interface")
		SettingsRequestBus.Broadcast.SetSettingValue("NothingName",tostring(UiCheckboxBus.Event.GetState(self.Properties.Nothing)), "Interface")
	end
	
	if actionName == "NothingName" then
		SettingsRequestBus.Broadcast.SetSettingValue("NothingName",tostring(UiCheckboxBus.Event.GetState(self.Properties.Nothing)), "Interface")
		UiCheckboxBus.Event.SetState(self.Properties.SteamName, false)
		UiCheckboxBus.Event.SetState(self.Properties.HeroName, false)
		SettingsRequestBus.Broadcast.SetSettingValue("HeroName",tostring(UiCheckboxBus.Event.GetState(self.Properties.HeroName)), "Interface")
		SettingsRequestBus.Broadcast.SetSettingValue("SteamName",tostring(UiCheckboxBus.Event.GetState(self.Properties.SteamName)), "Interface")
	end
end

function Interface:DefaultSettings()
	--Set Default Settings
	
	if not UiElementBus.Event.IsEnabled(self.entityId) then
		return
	end
	
	SettingsRequestBus.Broadcast.SetSettingValue("HeroName","true", "Interface")
	
	SettingsRequestBus.Broadcast.SetSettingValue("SteamName","false", "Interface")
	
	SettingsRequestBus.Broadcast.SetSettingValue("NothingName","false", "Interface")
	
	SettingsRequestBus.Broadcast.SetSettingValue("minimapLocation","left","Interface")

	SettingsRequestBus.Broadcast.SetSettingValue("UseSimplifiedColorsInMiniMap","false","Interface")	

	SettingsRequestBus.Broadcast.SetSettingValue("AltToShowHeroIcon","false","Interface")	

	SettingsRequestBus.Broadcast.SetSettingValue("InvertAltToggled","false","Interface")	

	SettingsRequestBus.Broadcast.SetSettingValue("MiniMapHeroSize",50,"Interface")

	SettingsRequestBus.Broadcast.SetSettingValue("DisplaySteamMessageAsWisper","true","Interface")	

	SettingsRequestBus.Broadcast.SetSettingValue("MuteAllChat","false","Interface")

	SettingsRequestBus.Broadcast.SetSettingValue("MuteAllEnemyChat","false","Interface")	

	SettingsRequestBus.Broadcast.SetSettingValue("DisableStatusText","false","Interface")	

	SettingsRequestBus.Broadcast.SetSettingValue("HideDamageNumbers","false","Interface")

	SettingsRequestBus.Broadcast.SetSettingValue("HUDScale",100,"Interface")

	SettingsRequestBus.Broadcast.SetSettingValue("ColorBlindMode","false","Interface")	

	SettingsRequestBus.Broadcast.SetSettingValue("DifferentiateAllyHealthBars","true","Interface")	

	SettingsRequestBus.Broadcast.SetSettingValue("HideEnemyManaBars","false","Interface")	
	
	SettingsRequestBus.Broadcast.SetSettingValue("AdvancedHUDLayout","false","Interface")	

	self:LoadSettings()
end

function Interface:GetBool(result)
	if result == "true" then
		return true
	elseif result == "false" then
		return false
	end
end

function Interface:LoadSettings()
	--Load Current Settings On Load
	if SettingsRequestBus.Broadcast.GetSettingValue("minimapLocation","Interface") == "right" then
		UiCheckboxBus.Event.SetState(self.Properties.MiniMapLocation,true)
	else
		UiCheckboxBus.Event.SetState(self.Properties.MiniMapLocation,false)
	end
	UiCheckboxBus.Event.SetState(self.Properties.SimpleColors,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("UseSimplifiedColorsInMiniMap","Interface")))
	UiCheckboxBus.Event.SetState(self.Properties.AltToShowHero,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("AltToShowHeroIcon","Interface")))
	UiCheckboxBus.Event.SetState(self.Properties.InvertAlt,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("InvertAltToggled","Interface")))
	UiCheckboxBus.Event.SetState(self.Properties.DisplaySteamAsWisper,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("DisplaySteamMessageAsWisper","Interface")))
	UiCheckboxBus.Event.SetState(self.Properties.MuteAllChat,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("MuteAllChat","Interface")))
	UiCheckboxBus.Event.SetState(self.Properties.MuteAllEnemyChat,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("MuteAllEnemyChat","Interface")))
	UiCheckboxBus.Event.SetState(self.Properties.DisableStatusText,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("DisableStatusText","Interface")))
	UiCheckboxBus.Event.SetState(self.Properties.HideDamage,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("HideDamageNumbers","Interface")))
	UiCheckboxBus.Event.SetState(self.Properties.ColorBlind,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("ColorBlindMode","Interface")))
	UiCheckboxBus.Event.SetState(self.Properties.DifferentiateAllyHealthBars,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("DifferentiateAllyHealthBars","Interface")))
	UiCheckboxBus.Event.SetState(self.Properties.HideEnemyManaBar,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("HideEnemyManaBars","Interface")))
	UiCheckboxBus.Event.SetState(self.Properties.AdvancedHUDLayout,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("AdvancedHUDLayout","Interface")))
	UiCheckboxBus.Event.SetState(self.Properties.HeroName,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("HeroName","Interface")))
	UiCheckboxBus.Event.SetState(self.Properties.SteamName,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("SteamName","Interface")))
	UiCheckboxBus.Event.SetState(self.Properties.Nothing,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("NothingName","Interface")))
	
	UiSliderBus.Event.SetValue(self.Properties.MiniMapHeroSize,SettingsRequestBus.Broadcast.GetSettingValue("MiniMapHeroSize","Interface"))
	UiTextBus.Event.SetText(self.Properties.Texts.MiniMapHeroSizeText,SettingsRequestBus.Broadcast.GetSettingValue("MiniMapHeroSize","Interface"))
	
	UiSliderBus.Event.SetValue(self.Properties.HUDScale,SettingsRequestBus.Broadcast.GetSettingValue("HUDScale","Interface"))
	UiTextBus.Event.SetText(self.Properties.Texts.HUDScaleText,SettingsRequestBus.Broadcast.GetSettingValue("HUDScale","Interface"))
	
	if self.init == false then
		self.init = true
	end
end

return Interface;