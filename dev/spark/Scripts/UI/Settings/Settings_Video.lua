require "scripts.common";

local Video = 
{
	init = false;
	Properties =
	{
		SizeDropDownText = {default = EntityId()},
		DisplayDropDownText = {default = EntityId()},
		
		SizeDropDown = {default = EntityId()},
		DisplayDropDown = {default = EntityId()},
		
		AspectRatio4_3 = {default = EntityId()},
		AspectRatio16_9 = {default = EntityId()},
		AspectRatio16_10 = {default = EntityId()},
		CustomRes = {default = EntityId()},
		UseCurrentRes = {default = EntityId()},
		UseBasicRenderingSettings = {default = EntityId()},
		UseAdvancedRenderingSettings = {default = EntityId()},
		TextureLow = {default = EntityId()},
		TextureMed = {default = EntityId()},
		TextureHigh = {default = EntityId()},
		EffectLow = {default = EntityId()},
		EffectMed = {default = EntityId()},
		EffectHigh = {default = EntityId()},
		ShadowLow = {default = EntityId()},
		ShadowMed = {default = EntityId()},
		ShadowHigh = {default = EntityId()},
		
		AnimatePortrait = {default = EntityId()},
		AntiAliasing = {default = EntityId()},
		AdditiveLightPass = {default = EntityId()},
		Specular = {default = EntityId()},
		WorldLighting = {default = EntityId()},
		SpecularAndLightBlooms = {default = EntityId()},
		AmbientOcclusions = {default = EntityId()},
		HighQualityDashboard = {default = EntityId()},
		NormalMaps = {default = EntityId()},
		AtmosphericFog = {default = EntityId()},
		TreeWind = {default = EntityId()},
		Grass = {default = EntityId()},
		
		GameScreenRenderQuality = {default = EntityId()},
		MaxFramePerSecond = {default = EntityId()},
	},
}
function Video:OnTick(deltaTime, timePoint)
    self.tickBusHandler:Disconnect()

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
function Video:OnActivate()
	self.tickBusHandler = TickBus.Connect(self);
	self.canvasNotificationHandler = nil
end

function Video:OnAction(entityId, actionName) 
  	if actionName == "DefaultSettings" then
		self:DefaultSettings()
	end
	
	if actionName == "ScreenSizeSelected" then
		local text = UiTextBus.Event.GetText(self.Properties.SizeDropDownText)
		SettingsRequestBus.Broadcast.SetSettingValue("ScreenSize",tostring(text), "Video")
		
		self:SetSize()
	end
	
	if actionName == "DisplayModeSelected" then
		local text = UiTextBus.Event.GetText(self.Properties.DisplayDropDownText)
		SettingsRequestBus.Broadcast.SetSettingValue("DisplayMode",tostring(text), "Video")
		
		self:SetSize()
	end
	
	if actionName == "AspectRatio4:3" then
		SettingsRequestBus.Broadcast.SetSettingValue("AspectRatio4:3",tostring(UiCheckboxBus.Event.GetState(self.Properties.AspectRatio4_3)), "Video")
		UiCheckboxBus.Event.SetState(self.Properties.AspectRatio16_9, false)
		UiCheckboxBus.Event.SetState(self.Properties.AspectRatio16_10, false)
		SettingsRequestBus.Broadcast.SetSettingValue("AspectRatio16:9",tostring(UiCheckboxBus.Event.GetState(self.Properties.AspectRatio16_9)), "Video")
		SettingsRequestBus.Broadcast.SetSettingValue("AspectRatio16:10",tostring(UiCheckboxBus.Event.GetState(self.Properties.AspectRatio16_10)), "Video")
	end
	
	if actionName == "AspectRatio16:9" then
		SettingsRequestBus.Broadcast.SetSettingValue("AspectRatio16:9",tostring(UiCheckboxBus.Event.GetState(self.Properties.AspectRatio16_9)), "Video")
		UiCheckboxBus.Event.SetState(self.Properties.AspectRatio4_3, false)
		UiCheckboxBus.Event.SetState(self.Properties.AspectRatio16_10, false)
		SettingsRequestBus.Broadcast.SetSettingValue("AspectRatio4:3",tostring(UiCheckboxBus.Event.GetState(self.Properties.AspectRatio4_3)), "Video")
		SettingsRequestBus.Broadcast.SetSettingValue("AspectRatio16:10",tostring(UiCheckboxBus.Event.GetState(self.Properties.AspectRatio16_10)), "Video")
	end
	
	if actionName == "AspectRatio16:10" then
		SettingsRequestBus.Broadcast.SetSettingValue("AspectRatio16:10",tostring(UiCheckboxBus.Event.GetState(self.Properties.AspectRatio16_10)), "Video")
		UiCheckboxBus.Event.SetState(self.Properties.AspectRatio16_9, false)
		UiCheckboxBus.Event.SetState(self.Properties.AspectRatio4_3, false)
		SettingsRequestBus.Broadcast.SetSettingValue("AspectRatio16:9",tostring(UiCheckboxBus.Event.GetState(self.Properties.AspectRatio16_9)), "Video")
		SettingsRequestBus.Broadcast.SetSettingValue("AspectRatio4:3",tostring(UiCheckboxBus.Event.GetState(self.Properties.AspectRatio4_3)), "Video")
	end	
	
	if actionName == "CustomRes" then
		SettingsRequestBus.Broadcast.SetSettingValue("CustomRes",tostring(UiCheckboxBus.Event.GetState(self.Properties.CustomRes)), "Video")
	end
	
	if actionName == "UseCurrentRes" then
		SettingsRequestBus.Broadcast.SetSettingValue("UseCurrentRes",tostring(UiCheckboxBus.Event.GetState(self.Properties.UseCurrentRes)), "Video")
	end
	
	if actionName == "UseBasicRenderingSettings" then
		SettingsRequestBus.Broadcast.SetSettingValue("UseBasicRenderingSettings",tostring(UiCheckboxBus.Event.GetState(self.Properties.UseBasicRenderingSettings)), "Video")
	end
	
	if actionName == "UseAdvancedRenderingSettings" then
		SettingsRequestBus.Broadcast.SetSettingValue("UseAdvancedRenderingSettings",tostring(UiCheckboxBus.Event.GetState(self.Properties.UseAdvancedRenderingSettings)), "Video")
	end
	
	if actionName == "TextureLow" then
		SettingsRequestBus.Broadcast.SetSettingValue("TextureLow",tostring(UiCheckboxBus.Event.GetState(self.Properties.TextureLow)), "Video")
		UiCheckboxBus.Event.SetState(self.Properties.TextureMed, false)
		UiCheckboxBus.Event.SetState(self.Properties.TextureHigh, false)
		SettingsRequestBus.Broadcast.SetSettingValue("TextureMed",tostring(UiCheckboxBus.Event.GetState(self.Properties.TextureMed)), "Video")
		SettingsRequestBus.Broadcast.SetSettingValue("TextureHigh",tostring(UiCheckboxBus.Event.GetState(self.Properties.TextureHigh)), "Video")
	end
	
	if actionName == "TextureMed" then
		SettingsRequestBus.Broadcast.SetSettingValue("TextureMed",tostring(UiCheckboxBus.Event.GetState(self.Properties.TextureMed)), "Video")
		UiCheckboxBus.Event.SetState(self.Properties.TextureLow, false)
		UiCheckboxBus.Event.SetState(self.Properties.TextureHigh, false)
		SettingsRequestBus.Broadcast.SetSettingValue("TextureLow",tostring(UiCheckboxBus.Event.GetState(self.Properties.TextureLow)), "Video")
		SettingsRequestBus.Broadcast.SetSettingValue("TextureHigh",tostring(UiCheckboxBus.Event.GetState(self.Properties.TextureHigh)), "Video")
	end
	
	if actionName == "TextureHigh" then
		SettingsRequestBus.Broadcast.SetSettingValue("TextureHigh",tostring(UiCheckboxBus.Event.GetState(self.Properties.TextureHigh)), "Video")
		UiCheckboxBus.Event.SetState(self.Properties.TextureLow, false)
		UiCheckboxBus.Event.SetState(self.Properties.TextureMed, false)
		SettingsRequestBus.Broadcast.SetSettingValue("TextureLow",tostring(UiCheckboxBus.Event.GetState(self.Properties.TextureLow)), "Video")
		SettingsRequestBus.Broadcast.SetSettingValue("TextureMed",tostring(UiCheckboxBus.Event.GetState(self.Properties.TextureMed)), "Video")
	end
	
	if actionName == "EffectLow" then
		SettingsRequestBus.Broadcast.SetSettingValue("EffectLow",tostring(UiCheckboxBus.Event.GetState(self.Properties.EffectLow)), "Video")
		UiCheckboxBus.Event.SetState(self.Properties.EffectHigh, false)
		UiCheckboxBus.Event.SetState(self.Properties.EffectMed, false)
		SettingsRequestBus.Broadcast.SetSettingValue("EffectMed",tostring(UiCheckboxBus.Event.GetState(self.Properties.EffectMed)), "Video")
		SettingsRequestBus.Broadcast.SetSettingValue("EffectHigh",tostring(UiCheckboxBus.Event.GetState(self.Properties.EffectHigh)), "Video")
	end
	
	if actionName == "EffectMed" then
		SettingsRequestBus.Broadcast.SetSettingValue("EffectMed",tostring(UiCheckboxBus.Event.GetState(self.Properties.EffectMed)), "Video")
		UiCheckboxBus.Event.SetState(self.Properties.EffectLow, false)
		UiCheckboxBus.Event.SetState(self.Properties.EffectHigh, false)
		SettingsRequestBus.Broadcast.SetSettingValue("EffectLow",tostring(UiCheckboxBus.Event.GetState(self.Properties.EffectLow)), "Video")
		SettingsRequestBus.Broadcast.SetSettingValue("EffectHigh",tostring(UiCheckboxBus.Event.GetState(self.Properties.EffectHigh)), "Video")
	end
	
	if actionName == "EffectHigh" then
		SettingsRequestBus.Broadcast.SetSettingValue("EffectHigh",tostring(UiCheckboxBus.Event.GetState(self.Properties.EffectHigh)), "Video")
		UiCheckboxBus.Event.SetState(self.Properties.EffectLow, false)
		UiCheckboxBus.Event.SetState(self.Properties.EffectMed, false)
		SettingsRequestBus.Broadcast.SetSettingValue("EffectLow",tostring(UiCheckboxBus.Event.GetState(self.Properties.EffectLow)), "Video")
		SettingsRequestBus.Broadcast.SetSettingValue("EffectMed",tostring(UiCheckboxBus.Event.GetState(self.Properties.EffectMed)), "Video")
	end
	
	if actionName == "ShadowLow" then
		SettingsRequestBus.Broadcast.SetSettingValue("ShadowLow",tostring(UiCheckboxBus.Event.GetState(self.Properties.ShadowLow)), "Video")
		UiCheckboxBus.Event.SetState(self.Properties.ShadowMed, false)
		UiCheckboxBus.Event.SetState(self.Properties.ShadowHigh, false)
		SettingsRequestBus.Broadcast.SetSettingValue("ShadowMed",tostring(UiCheckboxBus.Event.GetState(self.Properties.ShadowMed)), "Video")
		SettingsRequestBus.Broadcast.SetSettingValue("ShadowHigh",tostring(UiCheckboxBus.Event.GetState(self.Properties.ShadowHigh)), "Video")
	end
	
	if actionName == "ShadowMed" then
		SettingsRequestBus.Broadcast.SetSettingValue("ShadowMed",tostring(UiCheckboxBus.Event.GetState(self.Properties.ShadowMed)), "Video")
		UiCheckboxBus.Event.SetState(self.Properties.ShadowLow, false)
		UiCheckboxBus.Event.SetState(self.Properties.ShadowHigh, false)
		SettingsRequestBus.Broadcast.SetSettingValue("ShadowLow",tostring(UiCheckboxBus.Event.GetState(self.Properties.ShadowLow)), "Video")
		SettingsRequestBus.Broadcast.SetSettingValue("ShadowHigh",tostring(UiCheckboxBus.Event.GetState(self.Properties.ShadowHigh)), "Video")
	end
	
	if actionName == "ShadowHigh" then
		SettingsRequestBus.Broadcast.SetSettingValue("ShadowHigh",tostring(UiCheckboxBus.Event.GetState(self.Properties.ShadowHigh)), "Video")
		UiCheckboxBus.Event.SetState(self.Properties.ShadowLow, false)
		UiCheckboxBus.Event.SetState(self.Properties.ShadowMed, false)
		SettingsRequestBus.Broadcast.SetSettingValue("ShadowLow",tostring(UiCheckboxBus.Event.GetState(self.Properties.ShadowLow)), "Video")
		SettingsRequestBus.Broadcast.SetSettingValue("ShadowMed",tostring(UiCheckboxBus.Event.GetState(self.Properties.ShadowMed)), "Video")
	end
	
	if actionName == "GameScreenRenderQualityChanged" then
		local Parent = UiElementBus.Event.GetParent(entityId)
		local Text = UiElementBus.Event.FindDescendantByName(Parent,"Text")
		SettingsRequestBus.Broadcast.SetSettingValue("GameScreenRenderQuality",tostring(UiSliderBus.Event.GetValue(self.Properties.GameScreenRenderQuality)), "Video")
	end
	
	if actionName == "MaxFramePerSecondChanged" then
		local Parent = UiElementBus.Event.GetParent(entityId)
		local Text = UiElementBus.Event.FindDescendantByName(Parent,"Text")
		SettingsRequestBus.Broadcast.SetSettingValue("MaxFramePerSecond",tostring(UiSliderBus.Event.GetValue(self.Properties.MaxFramePerSecond)), "Video")
	end
	
	if actionName == "AnimatePortrait" then
		SettingsRequestBus.Broadcast.SetSettingValue("AnimatePortrait",tostring(UiCheckboxBus.Event.GetState(self.Properties.AnimatePortrait)), "Video")
	end
	
	if actionName == "AntiAliasing" then
		SettingsRequestBus.Broadcast.SetSettingValue("AntiAliasing",tostring(UiCheckboxBus.Event.GetState(self.Properties.AntiAliasing)), "Video")
	end
	
	if actionName == "AdditiveLightPass" then
		SettingsRequestBus.Broadcast.SetSettingValue("AdditiveLightPass",tostring(UiCheckboxBus.Event.GetState(self.Properties.AdditiveLightPass)), "Video")
	end
	
	if actionName == "Specular" then
		SettingsRequestBus.Broadcast.SetSettingValue("Specular",tostring(UiCheckboxBus.Event.GetState(self.Properties.Specular)), "Video")
	end
	
	if actionName == "WorldLighting" then
		SettingsRequestBus.Broadcast.SetSettingValue("WorldLighting",tostring(UiCheckboxBus.Event.GetState(self.Properties.WorldLighting)), "Video")
	end
	
	if actionName == "SpecularAndLightBlooms" then
		SettingsRequestBus.Broadcast.SetSettingValue("SpecularAndLightBlooms",tostring(UiCheckboxBus.Event.GetState(self.Properties.SpecularAndLightBlooms)), "Video")
	end
	
	if actionName == "AmbientOcclusions" then
		SettingsRequestBus.Broadcast.SetSettingValue("AmbientOcclusions",tostring(UiCheckboxBus.Event.GetState(self.Properties.AmbientOcclusions)), "Video")
	end
	
	if actionName == "HighQualityDashboard" then
		SettingsRequestBus.Broadcast.SetSettingValue("HighQualityDashboard",tostring(UiCheckboxBus.Event.GetState(self.Properties.HighQualityDashboard)), "Video")
	end
	
	if actionName == "NormalMaps" then
		SettingsRequestBus.Broadcast.SetSettingValue("NormalMaps",tostring(UiCheckboxBus.Event.GetState(self.Properties.NormalMaps)), "Video")
	end
	
	if actionName == "AtmosphericFog" then
		SettingsRequestBus.Broadcast.SetSettingValue("AtmosphericFog",tostring(UiCheckboxBus.Event.GetState(self.Properties.AtmosphericFog)), "Video")
	end
	
	if actionName == "TreeWind" then
		SettingsRequestBus.Broadcast.SetSettingValue("TreeWind",tostring(UiCheckboxBus.Event.GetState(self.Properties.TreeWind)), "Video")
	end
	
	if actionName == "Grass" then
		SettingsRequestBus.Broadcast.SetSettingValue("Grass",tostring(UiCheckboxBus.Event.GetState(self.Properties.Grass)), "Video")
	end
end

function Video:SetSize()
	local screenSize = SettingsRequestBus.Broadcast.GetSettingValue("ScreenSize","Video")
	local text = SettingsRequestBus.Broadcast.GetSettingValue("DisplayMode","Video")
	local values=splitstring(screenSize, "x")
	if #values >= 2 then
		x = tonumber(values[1])
		y = tonumber(values[2])
		if text == "Borderless Window" or text == "Exclusive Fullscreen" then
			SettingsRequestBus.Broadcast.ChangeVideoMode(Vector2(x,y),true);
		else
			SettingsRequestBus.Broadcast.ChangeVideoMode(Vector2(x,y),false);
		end
	end
	
end

function Video:DefaultSettings()
	--Set Default Settings
	if not UiElementBus.Event.IsEnabled(self.entityId) then
		return
	end
	
	SettingsRequestBus.Broadcast.SetSettingValue("AspectRatio4:3","false", "Video")
	SettingsRequestBus.Broadcast.SetSettingValue("AspectRatio16:9","true", "Video")

	SettingsRequestBus.Broadcast.SetSettingValue("AspectRatio16:10","false", "Video")

	SettingsRequestBus.Broadcast.SetSettingValue("CustomRes","false", "Video")

	SettingsRequestBus.Broadcast.SetSettingValue("UseCurrentRes","true", "Video")

	SettingsRequestBus.Broadcast.SetSettingValue("UseBasicRenderingSettings","true", "Video")

	SettingsRequestBus.Broadcast.SetSettingValue("UseAdvancedRenderingSettings","false", "Video")

	SettingsRequestBus.Broadcast.SetSettingValue("TextureLow","false", "Video")

	SettingsRequestBus.Broadcast.SetSettingValue("TextureMed","false", "Video")

	SettingsRequestBus.Broadcast.SetSettingValue("TextureHigh","true", "Video")

	SettingsRequestBus.Broadcast.SetSettingValue("EffectLow","false", "Video")

	SettingsRequestBus.Broadcast.SetSettingValue("EffectMed","false", "Video")

	SettingsRequestBus.Broadcast.SetSettingValue("EffectHigh","true", "Video")

	SettingsRequestBus.Broadcast.SetSettingValue("ShadowLow","false", "Video")

	SettingsRequestBus.Broadcast.SetSettingValue("ShadowMed","false", "Video")

	SettingsRequestBus.Broadcast.SetSettingValue("ShadowHigh","true", "Video")
	
	SettingsRequestBus.Broadcast.SetSettingValue("GameScreenRenderQuality","0", "Video")
	
	SettingsRequestBus.Broadcast.SetSettingValue("MaxFramePerSecond","0", "Video")

	SettingsRequestBus.Broadcast.SetSettingValue("AnimatePortrait","false", "Video")

	SettingsRequestBus.Broadcast.SetSettingValue("AntiAliasing","false", "Video")

	SettingsRequestBus.Broadcast.SetSettingValue("AdditiveLightPass","false", "Video")

	SettingsRequestBus.Broadcast.SetSettingValue("Specular","false", "Video")

	SettingsRequestBus.Broadcast.SetSettingValue("WorldLighting","false", "Video")

	SettingsRequestBus.Broadcast.SetSettingValue("SpecularAndLightBlooms","false", "Video")

	SettingsRequestBus.Broadcast.SetSettingValue("AmbientOcclusions","false", "Video")

	SettingsRequestBus.Broadcast.SetSettingValue("HighQualityDashboard","false", "Video")

	SettingsRequestBus.Broadcast.SetSettingValue("NormalMaps","false", "Video")

	SettingsRequestBus.Broadcast.SetSettingValue("AtmosphericFog","false", "Video")

	SettingsRequestBus.Broadcast.SetSettingValue("TreeWind","false", "Video")

	SettingsRequestBus.Broadcast.SetSettingValue("Grass","false", "Video")

	self:LoadSettings()

end

function Video:GetBool(result)
	if result == "true" then
		return true
	elseif result == "false" then
		return false
	else
		--self:GetDefaultSetting(result)
	end
end

function Video:LoadSettings()
	--Load Current Settings On Load
	UiCheckboxBus.Event.SetState(self.Properties.AspectRatio4_3,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("AspectRatio4:3","Video")))
	UiCheckboxBus.Event.SetState(self.Properties.AspectRatio16_9,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("AspectRatio16:9","Video")))
	UiCheckboxBus.Event.SetState(self.Properties.AspectRatio16_10,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("AspectRatio16:10","Video")))
	UiCheckboxBus.Event.SetState(self.Properties.CustomRes,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("CustomRes","Video")))
	UiCheckboxBus.Event.SetState(self.Properties.UseCurrentRes,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("UseCurrentRes","Video")))
	UiCheckboxBus.Event.SetState(self.Properties.UseBasicRenderingSettings,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("UseBasicRenderingSettings","Video")))
	UiCheckboxBus.Event.SetState(self.Properties.UseAdvancedRenderingSettings,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("UseAdvancedRenderingSettings","Video")))
	UiCheckboxBus.Event.SetState(self.Properties.TextureLow,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("TextureLow","Video")))
	UiCheckboxBus.Event.SetState(self.Properties.TextureMed,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("TextureMed","Video")))
	UiCheckboxBus.Event.SetState(self.Properties.TextureHigh,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("TextureHigh","Video")))
	UiCheckboxBus.Event.SetState(self.Properties.EffectLow,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("EffectLow","Video")))
	UiCheckboxBus.Event.SetState(self.Properties.EffectMed,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("EffectMed","Video")))
	UiCheckboxBus.Event.SetState(self.Properties.EffectHigh,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("EffectHigh","Video")))
	UiCheckboxBus.Event.SetState(self.Properties.ShadowLow,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("ShadowLow","Video")))
	UiCheckboxBus.Event.SetState(self.Properties.ShadowMed,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("ShadowMed","Video")))
	UiCheckboxBus.Event.SetState(self.Properties.ShadowHigh,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("ShadowHigh","Video")))
	UiCheckboxBus.Event.SetState(self.Properties.AnimatePortrait,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("AnimatePortrait","Video")))
	UiCheckboxBus.Event.SetState(self.Properties.AntiAliasing,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("AntiAliasing","Video")))
	UiCheckboxBus.Event.SetState(self.Properties.AdditiveLightPass,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("AdditiveLightPass","Video")))
	UiCheckboxBus.Event.SetState(self.Properties.Specular,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("Specular","Video")))
	UiCheckboxBus.Event.SetState(self.Properties.WorldLighting,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("WorldLighting","Video")))
	UiCheckboxBus.Event.SetState(self.Properties.SpecularAndLightBlooms,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("SpecularAndLightBlooms","Video")))
	UiCheckboxBus.Event.SetState(self.Properties.AmbientOcclusions,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("AmbientOcclusions","Video")))
	UiCheckboxBus.Event.SetState(self.Properties.HighQualityDashboard,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("HighQualityDashboard","Video")))
	UiCheckboxBus.Event.SetState(self.Properties.NormalMaps,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("NormalMaps","Video")))
	UiCheckboxBus.Event.SetState(self.Properties.AtmosphericFog,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("AtmosphericFog","Video")))
	UiCheckboxBus.Event.SetState(self.Properties.TreeWind,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("TreeWind","Video")))
	--UiCheckboxBus.Event.SetState(self.Properties.Grass,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("Grass ","Video")))
	
	UiSliderBus.Event.SetValue(self.Properties.GameScreenRenderQuality,SettingsRequestBus.Broadcast.GetSettingValue("GameScreenRenderQuality","Video"))
	
	UiSliderBus.Event.SetValue(self.Properties.MaxFramePerSecond,SettingsRequestBus.Broadcast.GetSettingValue("MaxFramePerSecond","Video"))
	
	
	if self.init == false then
		self.init = true
	end
end

return Video;