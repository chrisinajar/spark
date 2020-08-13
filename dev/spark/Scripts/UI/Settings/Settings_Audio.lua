local Audio = 
{
	currentSlot = nil;
	init = false;
	Properties =
	{
		MasterVolume = {default = EntityId()},
		GameSounds = {default = EntityId()},
		Music = {default = EntityId()},
		Voice = {default = EntityId()},
		UnitSpeech = {default = EntityId()},
		UnitSpeechOff = {default = EntityId()},
		UnitSpeechEvents = {default = EntityId()},
		UnitSpeechAll = {default = EntityId()},
		
		SoundDevice = {default = EntityId()},
		SpeakerConfig = {default = EntityId()},
		
		PlaySoundInDesktop = {default = EntityId()},
		ChatMessageSound = {default = EntityId()},
		
		VoiceChatPartyKey = {default = EntityId()},
		VoiceChatPartyOpenMic = {default = EntityId()},
		
		VoiceChatTeamKey = {default = EntityId()},
		VoiceChatTeamOpenMic = {default = EntityId()},
		
		OpenMicThreshold = {default = EntityId()},	

		Texts = {
			MasterVolumeText = {default = EntityId()},
			GameSoundsText = {default = EntityId()},
			MusicText = {default = EntityId()},
			VoiceText = {default = EntityId()},
			UnitSpeechText = {default = EntityId()},			
			OpenMicThresholdText = {default = EntityId()},

			SoundDeviceText = {default = EntityId()},
			SpeakerConfigText = {default = EntityId()},
		},
	},
}
function Audio:OnTick(deltaTime, timePoint)
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
function Audio:OnActivate()
	self.tickBusHandler = TickBus.Connect(self);
	self.canvasNotificationHandler = nil
end

function Audio:OnDeactivate()
	self.tickBusHandler:Disconnect()
	self.canvasNotificationHandler:Disconnect()
end

function Audio:OnAction(entityId, actionName) 
  	if actionName == "DefaultSettings" then
		self:DefaultSettings()
	end
	
	if actionName == "MasterVolumeChanged" then
		local Parent = UiElementBus.Event.GetParent(entityId)
		local Text = UiElementBus.Event.FindDescendantByName(Parent,"Text")
		SettingsRequestBus.Broadcast.SetSettingValue("MasterVolume",tostring(UiSliderBus.Event.GetValue(self.Properties.MasterVolume)), "Audio")
		UiTextBus.Event.SetText(Text,tostring(UiSliderBus.Event.GetValue(self.Properties.MasterVolume)))
	end
	
	if actionName == "GameSoundsChanged" then
		local Parent = UiElementBus.Event.GetParent(entityId)
		local Text = UiElementBus.Event.FindDescendantByName(Parent,"Text")
		SettingsRequestBus.Broadcast.SetSettingValue("GameSounds",tostring(UiSliderBus.Event.GetValue(self.Properties.GameSounds)), "Audio")
		UiTextBus.Event.SetText(Text,tostring(UiSliderBus.Event.GetValue(self.Properties.GameSounds)))
	end
    
	if actionName == "MusicChanged" then
		local Parent = UiElementBus.Event.GetParent(entityId)
		local Text = UiElementBus.Event.FindDescendantByName(Parent,"Text")
		SettingsRequestBus.Broadcast.SetSettingValue("Music",tostring(UiSliderBus.Event.GetValue(self.Properties.Music)), "Audio")
		UiTextBus.Event.SetText(Text,tostring(UiSliderBus.Event.GetValue(self.Properties.Music)))
	end
    
	if actionName == "VoiceChanged" then
		local Parent = UiElementBus.Event.GetParent(entityId)
		local Text = UiElementBus.Event.FindDescendantByName(Parent,"Text")
		SettingsRequestBus.Broadcast.SetSettingValue("Voice",tostring(UiSliderBus.Event.GetValue(self.Properties.Voice)), "Audio")
		UiTextBus.Event.SetText(Text,tostring(UiSliderBus.Event.GetValue(self.Properties.Voice)))
	end
    
	if actionName == "UnitSpeechChanged" then
		local Parent = UiElementBus.Event.GetParent(entityId)
		local Text = UiElementBus.Event.FindDescendantByName(Parent,"Text")
		SettingsRequestBus.Broadcast.SetSettingValue("UnitSpeech",tostring(UiSliderBus.Event.GetValue(self.Properties.UnitSpeech)), "Audio")
		UiTextBus.Event.SetText(Text,tostring(UiSliderBus.Event.GetValue(self.Properties.UnitSpeech)))
	end
    
	if actionName == "UnitSpeechOff" then
		SettingsRequestBus.Broadcast.SetSettingValue("UnitSpeechOff",tostring(UiCheckboxBus.Event.GetState(self.Properties.UnitSpeechOff)), "Audio")
		UiCheckboxBus.Event.SetState(self.Properties.UnitSpeechEvents, false)
		UiCheckboxBus.Event.SetState(self.Properties.UnitSpeechAll, false)
		SettingsRequestBus.Broadcast.SetSettingValue("UnitSpeechAll",tostring(UiCheckboxBus.Event.GetState(self.Properties.UnitSpeechAll)), "Audio")
		SettingsRequestBus.Broadcast.SetSettingValue("UnitSpeechEvents",tostring(UiCheckboxBus.Event.GetState(self.Properties.UnitSpeechEvents)), "Audio")
	end
    
	if actionName == "UnitSpeechEvents" then
		SettingsRequestBus.Broadcast.SetSettingValue("UnitSpeechEvents",tostring(UiCheckboxBus.Event.GetState(self.Properties.UnitSpeechEvents)), "Audio")
		UiCheckboxBus.Event.SetState(self.Properties.UnitSpeechOff, false)
		UiCheckboxBus.Event.SetState(self.Properties.UnitSpeechAll, false)
		SettingsRequestBus.Broadcast.SetSettingValue("UnitSpeechOff",tostring(UiCheckboxBus.Event.GetState(self.Properties.UnitSpeechOff)), "Audio")
		SettingsRequestBus.Broadcast.SetSettingValue("UnitSpeechAll",tostring(UiCheckboxBus.Event.GetState(self.Properties.UnitSpeechAll)), "Audio")
	end
    
	if actionName == "UnitSpeechAll" then
		SettingsRequestBus.Broadcast.SetSettingValue("UnitSpeechAll",tostring(UiCheckboxBus.Event.GetState(self.Properties.UnitSpeechAll)), "Audio")
		UiCheckboxBus.Event.SetState(self.Properties.UnitSpeechOff, false)
		UiCheckboxBus.Event.SetState(self.Properties.UnitSpeechEvents, false)
		SettingsRequestBus.Broadcast.SetSettingValue("UnitSpeechOff",tostring(UiCheckboxBus.Event.GetState(self.Properties.UnitSpeechOff)), "Audio")
		SettingsRequestBus.Broadcast.SetSettingValue("UnitSpeechEvents",tostring(UiCheckboxBus.Event.GetState(self.Properties.UnitSpeechEvents)), "Audio")
	end
    
	if actionName == "SoundDeviceSelected" then
		SettingsRequestBus.Broadcast.SetSettingValue("SoundDevice",tostring(UiDropdownBus.Event.GetValue(self.Properties.SoundDevice)), "Audio")
	end
    
	if actionName == "SpeakerConfigSelected" then
		SettingsRequestBus.Broadcast.SetSettingValue("SpeakerConfig",tostring(UiDropdownBus.Event.GetValue(self.Properties.SpeakerConfig)), "Audio")
	end
     
	if actionName == "PlaySoundInDesktop" then
		SettingsRequestBus.Broadcast.SetSettingValue("PlaySoundInDesktop",tostring(UiCheckboxBus.Event.GetState(self.Properties.PlaySoundInDesktop)), "Audio")
	end
     
	if actionName == "ChatMessageSound" then
		SettingsRequestBus.Broadcast.SetSettingValue("ChatMessageSound",tostring(UiCheckboxBus.Event.GetState(self.Properties.ChatMessageSound)), "Audio")
	end
     
	if actionName == "VoiceChatPartyKeyPressed" then
		self.currentSlot = "Slot.VoiceChatParty"
		--InputMapperRequestBus.Broadcast.StartHotkeyRegistration();
	end
     
	if actionName == "VoiceChatPartyOpenMic" then
		SettingsRequestBus.Broadcast.SetSettingValue("VoiceChatPartyOpenMic",tostring(UiCheckboxBus.Event.GetState(self.Properties.VoiceChatPartyOpenMic)), "Audio")
	end
     
	if actionName == "VoiceChatTeamKeyPressed" then
		self.currentSlot = "Slot.VoiceChatTeam"
		--InputMapperRequestBus.Broadcast.StartHotkeyRegistration();
	end
     
	if actionName == "VoiceChatTeamOpenMic" then
		SettingsRequestBus.Broadcast.SetSettingValue("VoiceChatTeamOpenMic",tostring(UiCheckboxBus.Event.GetState(self.Properties.VoiceChatTeamOpenMic)), "Audio")
	end
	
	if actionName == "OpenMicThreshold" then
		local Parent = UiElementBus.Event.GetParent(entityId)
		local Text = UiElementBus.Event.FindDescendantByName(Parent,"Text")
		SettingsRequestBus.Broadcast.SetSettingValue("OpenMicThreshold",tostring(UiSliderBus.Event.GetValue(self.Properties.OpenMicThreshold)), "Audio")
		UiTextBus.Event.SetText(Text,tostring(UiSliderBus.Event.GetValue(self.Properties.OpenMicThreshold)))
	end
    
    
end

function Audio:OnHotkeyRegistrationStart()

end

function Audio:OnHotkeyRegistrationChanged(hotkey)

end

function Audio:OnHotkeyRegistrationDone(hotkey)
	if self.currentlSlot == "Slot.VoiceChatParty" then
		--InputMapperRequestBus.Broadcast.BindHotKeyToSlot(hotkey,Slot(Slot.Ability,self.ButtonHoveredIndex));
	elseif self.currentlSlot == "Slot.VoiceChatTeam" then
		--InputMapperRequestBus.Broadcast.BindHotKeyToSlot(hotkey,Slot(Slot.Inventory,self.ButtonHoveredIndex));
	end
	
	--self:UpdateHotKeys(self.currentlSlot)
end

function Audio:UpdateHotKeys(CurrentSlot)	
	if CurrentSlot == "Slot.VoiceChatParty" then
		Newkey = InputMapperRequestBus.Broadcast.GetSlotHotkey(Slot(Slot.Ability,self.ButtonHoveredIndex))
		self.Button = UiElementBus.Event.GetChild(self.Properties.AbilityKeyBinds.AbilityKeyBindPanel, self.ButtonHoveredIndex)
	elseif CurrentSlot == "Slot.VoiceChatTeam" then
		Newkey = InputMapperRequestBus.Broadcast.GetSlotHotkey(Slot(Slot.Inventory,self.ButtonHoveredIndex))
		self.Button = UiElementBus.Event.GetChild(self.Properties.ItemKeyBinds.ItemKeyBindPanel, self.ButtonHoveredIndex)
	end
	self.Text = UiElementBus.Event.FindChildByName(self.Button, "Text")
	local keys=Newkey:GetKeys();
	for h=1, #keys do
		Debug.Log(KeyToString(keys[h]));
		if h == 2 then
			UiTextBus.Event.SetText(self.Text, KeyToString(keys[2]) .. "\n" .. KeyToString(keys[1]))
		else 
			UiTextBus.Event.SetText(self.Text, KeyToString(keys[h]))
		end
				
	end	
end

function Audio:DefaultSettings()
	--Set Default Settings
	
	if not UiElementBus.Event.IsEnabled(self.entityId) then
		return
	end

	SettingsRequestBus.Broadcast.SetSettingValue("MasterVolume",100, "Audio")

	SettingsRequestBus.Broadcast.SetSettingValue("GameSounds",100, "Audio")

	SettingsRequestBus.Broadcast.SetSettingValue("Music",100, "Audio")

	SettingsRequestBus.Broadcast.SetSettingValue("Voice",100, "Audio")

	SettingsRequestBus.Broadcast.SetSettingValue("UnitSpeech",100, "Audio")


	SettingsRequestBus.Broadcast.SetSettingValue("UnitSpeechOff","false", "Audio")

	SettingsRequestBus.Broadcast.SetSettingValue("UnitSpeechEvents","false", "Audio")

	SettingsRequestBus.Broadcast.SetSettingValue("UnitSpeechAll","true", "Audio")

	--SettingsRequestBus.Broadcast.SetSettingValue("SoundDevice",tostring(UiDropdownBus.Event.GetValue(self.Properties.SoundDevice)), "Audio")

	--SettingsRequestBus.Broadcast.SetSettingValue("SpeakerConfig",tostring(UiDropdownBus.Event.GetValue(self.Properties.SpeakerConfig)), "Audio")

	SettingsRequestBus.Broadcast.SetSettingValue("PlaySoundInDesktop","true", "Audio")

	SettingsRequestBus.Broadcast.SetSettingValue("ChatMessageSound","true", "Audio")


	SettingsRequestBus.Broadcast.SetSettingValue("VoiceChatPartyOpenMic","false", "Audio")



	SettingsRequestBus.Broadcast.SetSettingValue("VoiceChatTeamOpenMic","false", "Audio")

	SettingsRequestBus.Broadcast.SetSettingValue("OpenMicThreshold",25, "Audio")
	
	self:LoadSettings()

end

function Audio:GetDefaultSetting(result)
	
end

function Audio:GetBool(result)
	if result == "true" then
		return true
	elseif result == "false" then
		return false
	else
		self:GetDefaultSetting(result)
	end
end

function Audio:LoadSettings()
	--Load Current Settings On Load
	
	UiCheckboxBus.Event.SetState(self.Properties.UnitSpeechOff,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("UnitSpeechOff","Audio")))
	UiCheckboxBus.Event.SetState(self.Properties.UnitSpeechEvents,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("UnitSpeechEvents","Audio")))
	UiCheckboxBus.Event.SetState(self.Properties.UnitSpeechAll,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("UnitSpeechAll","Audio")))
	UiCheckboxBus.Event.SetState(self.Properties.PlaySoundInDesktop,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("PlaySoundInDesktop","Audio")))
	UiCheckboxBus.Event.SetState(self.Properties.ChatMessageSound,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("ChatMessageSound","Audio")))
	UiCheckboxBus.Event.SetState(self.Properties.VoiceChatPartyOpenMic,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("VoiceChatPartyOpenMic","Audio")))
	UiCheckboxBus.Event.SetState(self.Properties.VoiceChatTeamOpenMic,self:GetBool(SettingsRequestBus.Broadcast.GetSettingValue("VoiceChatTeamOpenMic","Audio")))

	UiSliderBus.Event.SetValue(self.Properties.OpenMicThreshold,SettingsRequestBus.Broadcast.GetSettingValue("OpenMicThreshold","Audio"))
	UiTextBus.Event.SetText(self.Properties.Texts.OpenMicThresholdText,tostring(UiSliderBus.Event.GetValue(self.Properties.OpenMicThreshold)))

	UiSliderBus.Event.SetValue(self.Properties.MasterVolume,SettingsRequestBus.Broadcast.GetSettingValue("MasterVolume","Audio"))
	UiTextBus.Event.SetText(self.Properties.Texts.MasterVolumeText,tostring(UiSliderBus.Event.GetValue(self.Properties.MasterVolume)))

	UiSliderBus.Event.SetValue(self.Properties.GameSounds,SettingsRequestBus.Broadcast.GetSettingValue("GameSounds","Audio"))
	UiTextBus.Event.SetText(self.Properties.Texts.GameSoundsText,tostring(UiSliderBus.Event.GetValue(self.Properties.GameSounds)))

	UiSliderBus.Event.SetValue(self.Properties.Music,SettingsRequestBus.Broadcast.GetSettingValue("Music","Audio"))
	UiTextBus.Event.SetText(self.Properties.Texts.MusicText,tostring(UiSliderBus.Event.GetValue(self.Properties.Music)))

	UiSliderBus.Event.SetValue(self.Properties.Voice,SettingsRequestBus.Broadcast.GetSettingValue("Voice","Audio"))
	UiTextBus.Event.SetText(self.Properties.Texts.VoiceText,tostring(UiSliderBus.Event.GetValue(self.Properties.Voice)))
	
	UiSliderBus.Event.SetValue(self.Properties.UnitSpeech,SettingsRequestBus.Broadcast.GetSettingValue("UnitSpeech","Audio"))
	UiTextBus.Event.SetText(self.Properties.Texts.UnitSpeechText,tostring(UiSliderBus.Event.GetValue(self.Properties.UnitSpeech)))

	UiTextBus.Event.SetText(self.Properties.Texts.SoundDeviceText,tostring(SettingsRequestBus.Broadcast.GetSettingValue("SoundDevice","Audio")))

	UiTextBus.Event.SetText(self.Properties.Texts.SpeakerConfigText,tostring(SettingsRequestBus.Broadcast.GetSettingValue("SpeakerConfig","Audio")))
	
	if self.init == false then
		self.init = true
	end
end

return Audio;