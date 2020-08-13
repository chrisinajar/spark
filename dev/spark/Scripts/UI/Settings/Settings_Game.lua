local Game = 
{
	init = false;
	Properties =
	{
		AutoSelectSummedUnits = {default = EntityId()},
		UnifiedOrders = {default = EntityId()},
		TeleportRequiresHold = {default = EntityId()},
		ChanneledAbilitiesRequireHold = {default = EntityId()},
		RightClickForceAttack = {default = EntityId()},
		QuickAttack = {default = EntityId()},
		QuickMove = {default = EntityId()},
		AutoAttackNever = {default = EntityId()},
		AutoAttackStandard = {default = EntityId()},
		AutoAttackAlways = {default = EntityId()},
		
		CenterCameraOnHeroRespawn = {default = EntityId()},
		DisableCameraZoom = {default = EntityId()},
		CameraSpeed = {default = EntityId()},
		HoldSelectHeroToFollow = {default = EntityId()},
		EdgePan = {default = EntityId()},
		
		Texts = {
			CameraSpeedText = {default = EntityId()},
		},
	},
}
function Game:OnTick(deltaTime, timePoint)
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
	
	if self.init == false then
		self:LoadSettings()
	end
end

function Game:OnActivate()
	self.tickBusHandler = TickBus.Connect(self);
	self.canvasNotificationHandler = nil	
end

function Game:OnDeactivate()
	self.tickBusHandler:Disconnect()
	self.canvasNotificationHandler:Disconnect()
end

function Game:OnAction(entityId, actionName) 
  	if actionName == "DefaultSettings" then
		self:DefaultSettings()
	end
	
	if actionName == "AutoSelectSummedUnits" then
		SettingsRequestBus.Broadcast.SetSettingValue("AutoSelectSummedUnits",tostring(UiCheckboxBus.Event.GetState(self.Properties.AutoSelectSummedUnits)), "Game")
	end
	
	if actionName == "UnifiedOrdersWithCtrl" then
		SettingsRequestBus.Broadcast.SetSettingValue("UnifiedOrdersWithCtrl",tostring(UiCheckboxBus.Event.GetState(self.Properties.UnifiedOrders)), "Game")
	end
	
	if actionName == "TeleportRequiresHold" then
		SettingsRequestBus.Broadcast.SetSettingValue("TeleportRequiresHold",tostring(UiCheckboxBus.Event.GetState(self.Properties.TeleportRequiresHold)), "Game")
	end
	
	if actionName == "AutoAttackNever" then
		SettingsRequestBus.Broadcast.SetSettingValue("AutoAttackNever",tostring(UiCheckboxBus.Event.GetState(self.Properties.AutoAttackNever)), "Game")
		UiCheckboxBus.Event.SetState(self.Properties.AutoAttackStandard, false)
		UiCheckboxBus.Event.SetState(self.Properties.AutoAttackAlways, false)
		SettingsRequestBus.Broadcast.SetSettingValue("AutoAttackStandard",tostring(UiCheckboxBus.Event.GetState(self.Properties.AutoAttackStandard)), "Game")
		SettingsRequestBus.Broadcast.SetSettingValue("AutoAttackAlways",tostring(UiCheckboxBus.Event.GetState(self.Properties.AutoAttackAlways)), "Game")
	end
	
	if actionName == "AutoAttackStandard" then
		SettingsRequestBus.Broadcast.SetSettingValue("AutoAttackStandard",tostring(UiCheckboxBus.Event.GetState(self.Properties.AutoAttackStandard)), "Game")
		UiCheckboxBus.Event.SetState(self.Properties.AutoAttackNever, false)
		UiCheckboxBus.Event.SetState(self.Properties.AutoAttackAlways, false)
		SettingsRequestBus.Broadcast.SetSettingValue("AutoAttackNever",tostring(UiCheckboxBus.Event.GetState(self.Properties.AutoAttackNever)), "Game")
		SettingsRequestBus.Broadcast.SetSettingValue("AutoAttackAlways",tostring(UiCheckboxBus.Event.GetState(self.Properties.AutoAttackAlways)), "Game")
	end
	
	if actionName == "AutoAttackAlways" then
		SettingsRequestBus.Broadcast.SetSettingValue("AutoAttackAlways",tostring(UiCheckboxBus.Event.GetState(self.Properties.AutoAttackAlways)), "Game")
		UiCheckboxBus.Event.SetState(self.Properties.AutoAttackNever, false)
		UiCheckboxBus.Event.SetState(self.Properties.AutoAttackStandard, false)
		SettingsRequestBus.Broadcast.SetSettingValue("AutoAttackStandard",tostring(UiCheckboxBus.Event.GetState(self.Properties.AutoAttackStandard)), "Game")
		SettingsRequestBus.Broadcast.SetSettingValue("AutoAttackNever",tostring(UiCheckboxBus.Event.GetState(self.Properties.AutoAttackNever)), "Game")
	end
	
	if actionName == "ChanneledAbilitiesRequireHold" then
		SettingsRequestBus.Broadcast.SetSettingValue("ChanneledAbilitiesRequireHold",tostring(UiCheckboxBus.Event.GetState(self.Properties.ChanneledAbilitiesRequireHold)), "Game")
	end
	
	if actionName == "RightClickForceAttack" then
		SettingsRequestBus.Broadcast.SetSettingValue("RightClickForceAttack",tostring(UiCheckboxBus.Event.GetState(self.Properties.RightClickForceAttack)), "Game")
	end
	
	if actionName == "QuickAttack" then
		SettingsRequestBus.Broadcast.SetSettingValue("QuickAttack",tostring(UiCheckboxBus.Event.GetState(self.Properties.QuickAttack)), "Game")
	end
	
	if actionName == "QuickMove" then
		SettingsRequestBus.Broadcast.SetSettingValue("QuickMove",tostring(UiCheckboxBus.Event.GetState(self.Properties.QuickMove)), "Game")
	end
	
	if actionName == "CenterCameraOnHeroRespawn" then
		SettingsRequestBus.Broadcast.SetSettingValue("CenterCameraOnHeroRespawn",tostring(UiCheckboxBus.Event.GetState(self.Properties.CenterCameraOnHeroRespawn)), "Game")
	end
	
	if actionName == "DisableCameraZoom" then
		SettingsRequestBus.Broadcast.SetSettingValue("DisableCameraZoom",tostring(UiCheckboxBus.Event.GetState(self.Properties.DisableCameraZoom)), "Game")
	end
	
	if actionName == "CameraSpeedChanged" then
		local Parent = UiElementBus.Event.GetParent(entityId)
		local Text = UiElementBus.Event.FindDescendantByName(Parent,"Text")
		SettingsRequestBus.Broadcast.SetSettingValue("CameraSpeed",tostring(UiSliderBus.Event.GetValue(self.Properties.CameraSpeed)), "Game")
		UiTextBus.Event.SetText(Text,tostring(UiSliderBus.Event.GetValue(self.Properties.CameraSpeed)))
	end
	
	if actionName == "HoldSelectHeroToFollow" then
		SettingsRequestBus.Broadcast.SetSettingValue("HoldSelectHeroToFollow",tostring(UiCheckboxBus.Event.GetState(self.Properties.HoldSelectHeroToFollow)), "Game")
	end
	
	if actionName == "EdgePan" then
		SettingsRequestBus.Broadcast.SetSettingValue("EdgePan",tostring(UiCheckboxBus.Event.GetState(self.Properties.EdgePan)), "Game")
	end
end

function Game:DefaultSettings()
	--Set Default Settings
	if UiElementBus.Event.IsEnabled(self.entityId) then
	
	SettingsRequestBus.Broadcast.SetSettingValue("AutoSelectSummedUnits","true", "Game")

	SettingsRequestBus.Broadcast.SetSettingValue("UnifiedOrdersWithCtrl","true", "Game")

	SettingsRequestBus.Broadcast.SetSettingValue("TeleportRequiresHold","false", "Game")

	SettingsRequestBus.Broadcast.SetSettingValue("AutoAttackNever","false", "Game")

	SettingsRequestBus.Broadcast.SetSettingValue("AutoAttackStandard","true", "Game")

	SettingsRequestBus.Broadcast.SetSettingValue("AutoAttackAlways","false", "Game")

	SettingsRequestBus.Broadcast.SetSettingValue("ChanneledAbilitiesRequireHold","false", "Game")

	SettingsRequestBus.Broadcast.SetSettingValue("RightClickForceAttack","false", "Game")

	SettingsRequestBus.Broadcast.SetSettingValue("QuickAttack","false", "Game")

	SettingsRequestBus.Broadcast.SetSettingValue("QuickMove","false", "Game")

	SettingsRequestBus.Broadcast.SetSettingValue("CenterCameraOnHeroRespawn","true", "Game")

	SettingsRequestBus.Broadcast.SetSettingValue("DisableCameraZoom","false", "Game")


	SettingsRequestBus.Broadcast.SetSettingValue("CameraSpeed",50, "Game")


	SettingsRequestBus.Broadcast.SetSettingValue("HoldSelectHeroToFollow","false", "Game")

	SettingsRequestBus.Broadcast.SetSettingValue("EdgePan","true", "Game")
	
	self:LoadSettings()
	
	end

end

function Game:GetBool(result)
	if result == "true" then
		return true
	elseif result == "false" then
		return false
	end
end

function Game:LoadSettings()
	--Load Current Settings On Load
	
	UiCheckboxBus.Event.SetState(self.Properties.AutoSelectSummedUnits,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("AutoSelectSummedUnits","Game")))
	UiCheckboxBus.Event.SetState(self.Properties.UnifiedOrders,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("UnifiedOrdersWithCtrl","Game")))
	UiCheckboxBus.Event.SetState(self.Properties.TeleportRequiresHold,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("TeleportRequiresHold","Game")))
	UiCheckboxBus.Event.SetState(self.Properties.AutoAttackNever,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("AutoAttackNever","Game")))
	UiCheckboxBus.Event.SetState(self.Properties.AutoAttackStandard,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("AutoAttackStandard","Game")))
	UiCheckboxBus.Event.SetState(self.Properties.AutoAttackAlways,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("AutoAttackAlways","Game")))
	UiCheckboxBus.Event.SetState(self.Properties.ChanneledAbilitiesRequireHold,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("ChanneledAbilitiesRequireHold","Game")))
	UiCheckboxBus.Event.SetState(self.Properties.RightClickForceAttack,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("RightClickForceAttack","Game")))
	UiCheckboxBus.Event.SetState(self.Properties.QuickAttack,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("QuickAttack","Game")))
	UiCheckboxBus.Event.SetState(self.Properties.QuickMove,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("QuickMove","Game")))
	UiCheckboxBus.Event.SetState(self.Properties.CenterCameraOnHeroRespawn,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("CenterCameraOnHeroRespawn","Game")))
	UiCheckboxBus.Event.SetState(self.Properties.DisableCameraZoom,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("DisableCameraZoom","Game")))
	UiCheckboxBus.Event.SetState(self.Properties.HoldSelectHeroToFollow,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("HoldSelectHeroToFollow","Game")))
	UiCheckboxBus.Event.SetState(self.Properties.EdgePan,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("EdgePan","Game")))
	
	UiSliderBus.Event.SetValue(self.Properties.CameraSpeed,SettingsRequestBus.Broadcast.GetSettingValue("CameraSpeed","Game"))
	UiTextBus.Event.SetText(self.Properties.Texts.CameraSpeedText,tostring(UiSliderBus.Event.GetValue(self.Properties.CameraSpeed)))
	
	if self.init == false then
		self.init = true
	end
end

return Game;