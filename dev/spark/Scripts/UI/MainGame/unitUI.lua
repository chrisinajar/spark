local UnitUI = 
{
	spawnTickets = {};
	damageListener = nil;
	spawnDamageTickets = {};
	spawnDamageDealtTickets = {};
	UnitsOnScreen = {};
	selectColor = nil;
	Unit = nil;
	Properties = 
	{
		HUD = {default = EntityId()},
		selfDamageColor = {default = Color(255/255, 0/255, 0/255)},
		dealtDamageColor = {default = Color(255/255, 255/255, 255/255)},
		differentiateColor = {default = Color(170/255, 255/255, 127/255)},
		normalColor = {default = Color(85/255, 255/255, 0/255)},
	},
}

function UnitUI:OnActivate()
	
	local setUnitKey = GameplayNotificationId(self.entityId, "SetUnit", "EntityId")
	local destroyKey = GameplayNotificationId(self.entityId, "Destroy", "EntityId")

	self.setUnitHandler = ConnectToEvents(GameplayNotificationBus, setUnitKey , { "OnEventBegin" })
	self.destroyHandler = ConnectToEvents(GameplayNotificationBus, destroyKey , { "OnEventBegin" })

	self.setUnitHandler.OnEventBegin(partial(self.SetUnit, self))
	self.destroyHandler.OnEventBegin(partial(self.Destroy, self))

	self.HUD = self.entityId
	self.health = UiElementBus.Event.FindChildByName(self.HUD, "Health")
	self.mana = UiElementBus.Event.FindChildByName(self.HUD, "Mana")
	self.nameRef = UiElementBus.Event.FindChildByName(self.HUD, "Name")
	self.level = UiElementBus.Event.FindChildByName(self.HUD, "Level")
	self.fill = nil
	if self.health then
		self.fill = UiElementBus.Event.FindChildByName(self.health, "Fill")
	end
	self.Name = "";
	self.tickBusHandler = TickBus.Connect(self)
end

function UnitUI:OnDeactivate()
	self.tickBusHandler:Disconnect()
	if self.spawnerHandler then
		self.spawnerHandler:Disconnect()
		self.spawnerHandler = nil
	end
	if self.damageListener then
		self.damageListener:Disconnect()
		self.damageListener = nil
	end	
	if self.damageHandler ~= nil then
		self.damageHandler:Disconnect()
		self.damageHandler = nil
	end
	if self.tah ~= nil then
		self.tah:Disconnect()
		self.tah = nil
	end
	self.setUnitHandler()
	self.destroyHandler()
	if self.unitDestroyHandler then
		self.unitDestroyHandler()
	end
end

function UnitUI:SetUnit (data)
	Debug.Log('Got unit set to ' .. tostring(data))
	self.Unit = data
	local unitDestroyKey = GameplayNotificationId(self.Unit, "Destroy", "EntityId")
	self.unitDestroyHandler = ConnectToEvents(GameplayNotificationBus, unitDestroyKey , { "OnEventBegin" })
	self.unitDestroyHandler.OnEventBegin(partial(self.Destroy, self))
	self.damageListener = OnDamageTakenNotificationBus.Connect(self,GetId(self.Unit))
end

function UnitUI:Destroy (data)
	UiElementBus.Event.DestroyElement(self.entityId)
end

function UnitUI:OnTick(deltaTime, timePoint)
	if self.tah == nil then
		self.TriggerArea = GameManagerRequestBus.Broadcast.GetEntitiesHavingTag(Crc32("camera_frustum"))
		if self.TriggerArea[1] then
			self.tah = TriggerAreaNotificationBus.Connect(self, self.TriggerArea[1])
		end
	end

	if self:CheckEnableDisable() then
		self:UpdateStats(self.Unit)
	end
end

function UnitUI:OnDamageTaken(damage)
	Debug.Log("DAMAGE BEEN TAKEN")
	if SettingsRequestBus.Broadcast.GetSettingValue("HideDamageNumbers", "interface") == "false" then
		self:UpdateDamageTaken(damage)
	end
end

function UnitUI:UpdateDamageTaken(damage)
	if damage.source == self.Unit or damage.target == self.Unit then
		if damage.target == self.Unit and damage.target == SelectionRequestBus.Broadcast.GetMainSelectedUnit() then
			local HUD = self.entityId
			local damageUi = UiElementBus.Event.FindChildByName(HUD, "Damage")
			self.damageHandler = UiSpawnerNotificationBus.Connect(self, damageUi)
			self.spawnDamageTickets[1] = UiSpawnerBus.Event.Spawn(damageUi)
			self.recentDamage = damage.damage
			self.selectColor = self.Properties.selfDamageColor
		else 
			local HUD = self.entityId
			local damageUi = UiElementBus.Event.FindChildByName(HUD, "Damage")
			self.damageHandler = UiSpawnerNotificationBus.Connect(self, damageUi)
			self.spawnDamageDealtTickets[1] = UiSpawnerBus.Event.Spawn(damageUi)
			self.recentDamage = damage.damage
			self.selectColor = self.Properties.dealtDamageColor
		end
	end
end

function UnitUI:UpdateStats(Unit)
	local HUD = self.HUD
	local health = self.health
	local mana = self.mana
	local nameRef = self.nameRef
	local level = self.level
	local fill = UiElementBus.Event.FindChildByName(self.health, "Fill")
	local NumberReference = 0;
	local HealthPercentageAmount = 0;
	local ManaPercentageAmount = 0;
	local Name = self.Name 
	
	if health and health:IsValid() then
		local newHP = (VariableManagerRequestBus.Broadcast.GetValue(VariableId(Unit, "hp")) / VariableManagerRequestBus.Broadcast.GetValue(VariableId(Unit, "hp_max"))) * 100
		if newHP ~= HealthPercentageAmount then
			HealthPercentageAmount = newHP
			UiSliderBus.Event.SetValue(health, HealthPercentageAmount)
		end
	end

	if mana and mana:IsValid() and UnitRequestBus.Event.GetName(Unit) ~= "creep" and ManaPercentageAmount ~= (VariableManagerRequestBus.Broadcast.GetValue(VariableId(Unit, "mana")) / VariableManagerRequestBus.Broadcast.GetValue(VariableId(Unit, "mana_max"))) * 100 then
		ManaPercentageAmount = (VariableManagerRequestBus.Broadcast.GetValue(VariableId(Unit, "mana")) / VariableManagerRequestBus.Broadcast.GetValue(VariableId(Unit, "mana_max"))) * 100
		UiSliderBus.Event.SetValue(mana, ManaPercentageAmount)
		if UiElementBus.Event.IsEnabled(mana) == false then
			UiElementBus.Event.SetIsEnabled(mana, true)
		end
	elseif mana and mana:IsValid() and UnitRequestBus.Event.GetName(Unit) == "creep" then
		if VariableManagerRequestBus.Broadcast.GetValue(VariableId(Unit, "mana")) == 0 then
			UiElementBus.Event.SetIsEnabled(mana, false)
		else
			if ManaPercentageAmount ~= (VariableManagerRequestBus.Broadcast.GetValue(VariableId(Unit, "mana")) / VariableManagerRequestBus.Broadcast.GetValue(VariableId(Unit, "mana_max"))) * 100 then
				ManaPercentageAmount = (VariableManagerRequestBus.Broadcast.GetValue(VariableId(Unit, "mana")) / VariableManagerRequestBus.Broadcast.GetValue(VariableId(Unit, "mana_max"))) * 100
				UiSliderBus.Event.SetValue(mana, ManaPercentageAmount)
				UiElementBus.Event.SetIsEnabled(mana, true)
			end
		end
	end

	if level and level:IsValid() and NumberReference ~= VariableManagerRequestBus.Broadcast.GetValue(VariableId(Unit, "level")) then
		NumberReference = VariableManagerRequestBus.Broadcast.GetValue(VariableId(Unit, "level"))
		UiTextBus.Event.SetText(UiElementBus.Event.FindChildByName(level, "Text"), tostring(tonumber(NumberReference)))
	end

	if nameRef ~= nil and UiTextBus.Event.GetText(UiElementBus.Event.FindChildByName(nameRef, "Text")) ~= nil then
		if string.lower(UiTextBus.Event.GetText(UiElementBus.Event.FindChildByName(nameRef, "Text"))) ~= tostring(UnitRequestBus.Event.GetName(Unit)) then
			local TextRef = UiElementBus.Event.FindChildByName(nameRef, "Text")
			if Name ~= UnitRequestBus.Event.GetName(Unit) and TextRef ~= UiTextBus.Event.GetText(TextRef) then
				if SettingsRequestBus.Broadcast.GetSettingValue("HeroName","Interface") == "true" then
					Name = UnitRequestBus.Event.GetName(Unit)			
					if TextRef ~= nil then
						if HasTag(Unit,"hero") then
							local heroName = LocalizationRequestBus.Broadcast.LocalizeString("@npc_spark_hero_" .. tostring(Name) .. "_name")
							UiTextBus.Event.SetText(TextRef, tostring(heroName))
							UiElementBus.Event.SetIsEnabled(TextRef, true)
						elseif HasTag(Unit,"creep") then
							local creepName = LocalizationRequestBus.Broadcast.LocalizeString("@npc_spark_".. string.lower(tostring(Name)) .."_name")
							UiTextBus.Event.SetText(TextRef, tostring(creepName))
							UiElementBus.Event.SetIsEnabled(TextRef, true)
						end
					end		
			    elseif SettingsRequestBus.Broadcast.GetSettingValue("SteamName","Interface") == "true" then
					--[[Name = UnitRequestBus.Event.GetName(Unit)
					if TextRef ~= nil then
						UiTextBus.Event.SetText(TextRef, tostring(Name))
						UiElementBus.Event.SetIsEnabled(TextRef, true)
					end	]]
			    elseif SettingsRequestBus.Broadcast.GetSettingValue("NothingName","Interface") == "true" then
					UiElementBus.Event.SetIsEnabled(TextRef, false)
				end
			end	
		end
	end
	
	
	if fill and UiImageBus.Event.GetColor(fill) ~= Color(255,0,0) and UnitRequestBus.Event.GetTeamId(Unit) ~= UnitRequestBus.Event.GetTeamId(SelectionRequestBus.Broadcast.GetMainSelectedUnit()) then
		UiImageBus.Event.SetColor(fill,Color(255,0,0))
	elseif fill and UnitRequestBus.Event.GetTeamId(Unit) == UnitRequestBus.Event.GetTeamId(SelectionRequestBus.Broadcast.GetMainSelectedUnit()) then
		if SettingsRequestBus.Broadcast.GetSettingValue("DifferentiateAllyHealthBars","Interface") == "true" and UiImageBus.Event.GetColor(fill) ~= self.Properties.differentiateColor then
			UiImageBus.Event.SetColor(fill,self.Properties.differentiateColor)
		else
			if UiImageBus.Event.GetColor(fill) ~= Color(0, 255, 0) and SettingsRequestBus.Broadcast.GetSettingValue("DifferentiateAllyHealthBars","Interface") == "false" then
				UiImageBus.Event.SetColor(fill,Color(0, 255, 0))
			end
		end
	end

	local position = TransformBus.Event.GetWorldTranslation(Unit)
	if position ~= nil then
		-- Debug.Log('Setting position ' .. tostring(position))
		position.z = position.z + 7
		local screenPos = SparkCameraRequestBus.Broadcast.ConvertWorldToPixelCoordinates(position)
		local pos = UiTransformBus.Event.GetViewportPosition(self.HUD)
		if pos ~= nil then
			pos.x = screenPos.x
			pos.y = screenPos.y
			UiTransformBus.Event.SetViewportPosition(self.HUD, pos)
		end
	else
		Debug.Log('This unit has no location? ' .. tostring(Unit))
	end
end

function UnitUI:CheckEnableDisable ()
	local shouldEnable = false
	if self.Unit and self.Unit:IsValid() then
		shouldEnable = UnitRequestBus.Event.IsAlive(self.Unit)
		if shouldEnable then
			shouldEnable = UnitRequestBus.Event.IsVisible(self.Unit)
			-- Debug.Log('Is visible? ' .. tostring(shouldEnable))
		end
	end
	if shouldEnable ~= UiElementBus.Event.IsEnabled(self.HUD) then
		UiElementBus.Event.SetIsEnabled(self.HUD, shouldEnable)
	end

	return shouldEnable
end

function UnitUI:OnEntitySpawned(ticket, id)
	if ticket == self.spawnDamageTickets[1] then
		local Text = UiElementBus.Event.FindDescendantByName(id, "Text")
		--Debug.Log("DAMAGE IS : " .. tostring(self.recentDamage))
		UiTextBus.Event.SetText(Text, tostring(tonumber(math.floor(self.recentDamage))))
		UiTextBus.Event.SetColor(Text, self.selectColor)
	end
	if ticket == self.spawnDamageDealtTickets[1] then
		local Text = UiElementBus.Event.FindDescendantByName(id, "Text")
		--Debug.Log("DAMAGE IS : " .. tostring(self.recentDamage))
		UiTextBus.Event.SetText(Text, tostring(tonumber(math.floor(self.recentDamage))))
		UiTextBus.Event.SetColor(Text, self.selectColor)
	end
end

function UnitUI:OnSpawnEnd(ticket)
	if ticket == self.spawnDamageTickets[1] then
		self.spawnDamageTickets[1] = nil
	end
	if ticket == self.spawnDamageDealtTickets[1] then
		self.spawnDamageDealtTickets[1] = nil
	end
end

function UnitUI:OnSpawnFailed(ticket)

end

return UnitUI;