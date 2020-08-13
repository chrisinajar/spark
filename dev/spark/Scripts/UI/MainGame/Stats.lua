require "scripts/library/timers"
require "scripts.variableholder"
require "gamemode.Spark.modules.hero_level_xp"

local Stats = 
{
	selectedUnit = nil;	
	TextElement = nil;
	ValueText = nil;
	BonusValueText = nil;
	NumberReference = nil;
	BonusNumberReference = nil;
	PercentageAmount = nil;
	Properties =
	{
		CurrentHP = {default = EntityId()},
		MaxHP = {default = EntityId()},
		CurrentMana = {default = EntityId()},
		MaxMana = {default = EntityId()},
		StrengthStat = {default = EntityId()},
		IntellaganceStat = {default = EntityId()},
		AgilityStat = {default = EntityId()},
		WillStat = {default = EntityId()},
		DamageStat = {default = EntityId()},
		ArmourStat = {default = EntityId()},
		MoveSpeedStat = {default = EntityId()},
		HealthBar = {default = EntityId()},
		ManaBar = {default = EntityId()},
		HPRegenBar = {default = EntityId()},
		ManaRegenBar = {default = EntityId()},
		XpBar = {default = EntityId()},
		CurrentXp = {default = EntityId()},
		Level = {default = EntityId()},
	},
}

function Stats:OnActivate()
	self.canvasNotificationHandler = nil
	self.selectionHandler = SelectionNotificationBus.Connect(self)
	
	CreateTimer(function() 
        self:InitUnit() 
    end,0.1);
end

function Stats:InitUnit()
	self.canvasEntityId = UiElementBus.Event.GetCanvas(self.entityId)
	
	if self.canvasEntityId then
		if self.canvasNotificationHandler then
			self.canvasNotificationHandler:Disconnect()
			self.canvasNotificationHandler = nil
		end
		self.canvasNotificationHandler = UiCanvasNotificationBus.Connect(self, self.canvasEntityId)
	end
	
	self.selectedUnit = SelectionRequestBus.Broadcast.GetMainSelectedUnit()
	self:SetStats(self.selectedUnit)
	self:OnMainSelectedUnitChanged(self.selectedUnit)
end

function Stats:OnSetValue(id, value)
	self:SetStats(tostring(id.variableId))
end

function Stats:OnMainSelectedUnitChanged(unit)
	self.selectedUnit = unit
	if self.VariableHandler then
		self.VariableHandler:Disconnect()
		self.VariableHandler = nil
	end
	self.VariableHandler = VariableHolderNotificationBus.Connect(self, unit)
end

function Stats:OnDeactivate()
	self.selectionHandler:Disconnect()
	if self.VariableHandler then
		self.VariableHandler:Disconnect()
		self.VariableHandler = nil
	end
end

function Stats:OnAction(entityId, actionName) 

end

function Stats:SetStats(id)	
	-- health
	if UnitRequestBus.Event.GetName(self.selectedUnit) == "creep" then
		UiElementBus.Event.SetIsEnabled(self.Properties.Level, false)
		UiElementBus.Event.SetIsEnabled(self.Properties.CurrentXp, false)
	else
		UiElementBus.Event.SetIsEnabled(self.Properties.Level, true)
		UiElementBus.Event.SetIsEnabled(self.Properties.CurrentXp, true)
	end
	
	--if id == "hp" or UiTextBus.Event.GetText(UiElementBus.Event.FindChildByName(self.Properties.CurrentHP, "Text")) == "#" then
		self.NumberReference = VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.selectedUnit, "hp"))
	    self.TextElement = UiElementBus.Event.FindChildByName(self.Properties.CurrentHP, "Text")
		UiTextBus.Event.SetText(self.TextElement, tostring(tonumber(math.floor(self.NumberReference))))
		self.PercentageAmount = (VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.selectedUnit, "hp")) / VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.selectedUnit, "hp_max"))) * 100
		UiSliderBus.Event.SetValue(self.Properties.HealthBar, self.PercentageAmount)
	--end
	--if id == "hp_max" or UiTextBus.Event.GetText(UiElementBus.Event.FindChildByName(self.Properties.MaxHP, "Text")) == "#" then
		self.TextElement = UiElementBus.Event.FindChildByName(self.Properties.MaxHP, "Text")
		self.NumberReference = VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.selectedUnit, "hp_max"))
		UiTextBus.Event.SetText(self.TextElement, tostring(tonumber(math.floor(self.NumberReference))))
	--end
	--if id == "hp_regen" or UiTextBus.Event.GetText(UiElementBus.Event.FindChildByName(self.Properties.HPRegenBar, "Text")) == "#" then
		self.TextElement = UiElementBus.Event.FindChildByName(self.Properties.HPRegenBar, "Text")
		self.NumberReference = VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.selectedUnit, "hp_regen"))
		UiTextBus.Event.SetText(self.TextElement, tostring(tonumber(string.format("%.2f",self.NumberReference))))
	--end
	
	-- mana
	--if id == "mana" or UiTextBus.Event.GetText(UiElementBus.Event.FindChildByName(self.Properties.CurrentMana, "Text")) == "#" then
		self.NumberReference = VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.selectedUnit, "mana"))
	    self.TextElement = UiElementBus.Event.FindChildByName(self.Properties.CurrentMana, "Text")
		UiTextBus.Event.SetText(self.TextElement, tostring(tonumber(math.floor(self.NumberReference))))
		self.PercentageAmount = (VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.selectedUnit, "mana")) / VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.selectedUnit, "mana_max"))) * 100
		UiSliderBus.Event.SetValue(self.Properties.ManaBar, self.PercentageAmount)
	--end
	--if id == "mana_max" or UiTextBus.Event.GetText(UiElementBus.Event.FindChildByName(self.Properties.MaxMana, "Text")) == "#" then
		self.TextElement = UiElementBus.Event.FindChildByName(self.Properties.MaxMana, "Text")
		self.NumberReference = VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.selectedUnit, "mana_max"))
		UiTextBus.Event.SetText(self.TextElement, tostring(tonumber(math.floor(self.NumberReference))))
	--end
	--if id == "mana_regen" or UiTextBus.Event.GetText(UiElementBus.Event.FindChildByName(self.Properties.ManaRegenBar, "Text")) == "#" then
		self.TextElement = UiElementBus.Event.FindChildByName(self.Properties.ManaRegenBar, "Text")
		self.NumberReference = VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.selectedUnit, "mana_regen"))
		UiTextBus.Event.SetText(self.TextElement, tostring(tonumber(string.format("%.2f",self.NumberReference))))
	--end
	
	-- Level
	--if id == "level" or UiTextBus.Event.GetText(UiElementBus.Event.FindChildByName(self.Properties.Level, "Text")) == "#" then
		self.TextElement = UiElementBus.Event.FindChildByName(self.Properties.Level, "Text")
		self.NumberReference = VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.selectedUnit, "level"))
		UiTextBus.Event.SetText(self.TextElement, tostring(tonumber(self.NumberReference)))
	--end
	
	-- XP
	--if id == "experience_progress" or UiTextBus.Event.GetText(UiElementBus.Event.FindChildByName(self.Properties.CurrentXp, "Current")) == "#" then
		self.NumberReference = VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.selectedUnit, "experience"))
	    self.TextElement = UiElementBus.Event.FindChildByName(self.Properties.CurrentXp, "Current")
		UiTextBus.Event.SetText(self.TextElement, tostring(tonumber(math.floor(self.NumberReference))))
		local level = tonumber(VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.selectedUnit, "level")))
		local levelxp = tonumber(LevelingXP[level][2])
		self.PercentageAmount = ((VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.selectedUnit, "experience")) - levelxp) / VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.selectedUnit, "experience_max"))) * 100
		UiSliderBus.Event.SetValue(self.Properties.XpBar, self.PercentageAmount)
	--end
	
	--if id == "experience_max" or UiTextBus.Event.GetText(UiElementBus.Event.FindChildByName(self.Properties.CurrentXp, "Max")) == "#" then
		local level = tonumber(VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.selectedUnit, "level")))
		local levelxp = tonumber(LevelingXP[level+1][2])
		self.NumberReference = levelxp --VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.selectedUnit, "experience_max"))
	    self.TextElement = UiElementBus.Event.FindChildByName(self.Properties.CurrentXp, "Max")
		UiTextBus.Event.SetText(self.TextElement, tostring(tonumber(math.floor(self.NumberReference))))
	--end
	
	-- damage
	--if id == "damage" or UiTextBus.Event.GetText(UiElementBus.Event.FindDescendantByName(self.Properties.DamageStat, "StatNumbers")) == "#" or UiTextBus.Event.GetText(UiElementBus.Event.FindDescendantByName(self.Properties.DamageStat, "StatNumbers2")) == "#" then
		self.ValueText = UiElementBus.Event.FindDescendantByName(self.Properties.DamageStat, "StatNumbers")
		self.BonusValueText = UiElementBus.Event.FindDescendantByName(self.Properties.DamageStat, "StatNumbers2")
		self.NumberReference = VariableManagerRequestBus.Broadcast.GetBaseValue(VariableId(self.selectedUnit, "damage"))
		self.BonusNumberReference = VariableManagerRequestBus.Broadcast.GetBonusValue(VariableId(self.selectedUnit, "damage"))
		if self.BonusNumberReference ~= 0 then
			UiTextBus.Event.SetText(self.BonusValueText, tostring(tonumber(math.floor(self.NumberReference))))
			UiTextBus.Event.SetText(self.ValueText, tostring(tonumber(math.floor(self.BonusNumberReference))))
			UiElementBus.Event.SetIsEnabled(self.BonusValueText,true)
			if self.BonusNumberReference > 0 then
				UiTextBus.Event.SetColor(self.ValueText, Color(85/255, 255/255, 0/255))
			elseif self.BonusNumberReference < 0 then
				UiTextBus.Event.SetColor(self.ValueText, Color(255/255, 0/255, 0/255))
			end
		else
			UiElementBus.Event.SetIsEnabled(self.BonusValueText,false)
			UiTextBus.Event.SetColor(self.ValueText, Color(255/255, 255/255, 255/255))
			UiTextBus.Event.SetText(self.ValueText, tostring(tonumber(math.floor(self.NumberReference))))
		end
	--end
	
	-- movespeed	
	--if id == "movement_speed" or UiTextBus.Event.GetText(UiElementBus.Event.FindDescendantByName(self.Properties.MoveSpeedStat, "StatNumbers")) == "#" then
		self.ValueText = UiElementBus.Event.FindDescendantByName(self.Properties.MoveSpeedStat, "StatNumbers")
		self.NumberReference = VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.selectedUnit, "movement_speed"))
	
		UiTextBus.Event.SetText(self.ValueText, tostring(tonumber(math.floor(self.NumberReference))))
	--end
	
	-- armor
	--if id == "armor" or UiTextBus.Event.GetText(UiElementBus.Event.FindDescendantByName(self.Properties.ArmourStat, "StatNumbers")) == "#"  or UiTextBus.Event.GetText(UiElementBus.Event.FindDescendantByName(self.Properties.ArmourStat, "StatNumbers2")) == "#" then
		self.ValueText = UiElementBus.Event.FindDescendantByName(self.Properties.ArmourStat, "StatNumbers")
		self.BonusValueText = UiElementBus.Event.FindDescendantByName(self.Properties.ArmourStat, "StatNumbers2")
		self.NumberReference = VariableManagerRequestBus.Broadcast.GetBaseValue(VariableId(self.selectedUnit, "armor"))
		self.BonusNumberReference = VariableManagerRequestBus.Broadcast.GetBonusValue(VariableId(self.selectedUnit, "armor"))
		if self.BonusNumberReference ~= 0 then
			UiTextBus.Event.SetText(self.BonusValueText, tostring(tonumber(math.floor(self.NumberReference))))
			UiTextBus.Event.SetText(self.ValueText, tostring(tonumber(math.floor(self.BonusNumberReference))))
			UiElementBus.Event.SetIsEnabled(self.BonusValueText,true)
			if self.BonusNumberReference > 0 then
				UiTextBus.Event.SetColor(self.ValueText, Color(85/255, 255/255, 0/255))
			elseif self.BonusNumberReference < 0 then
				UiTextBus.Event.SetColor(self.ValueText, Color(255/255, 0/255, 0/255))
			end
		else
			UiElementBus.Event.SetIsEnabled(self.BonusValueText,false)
			UiTextBus.Event.SetColor(self.ValueText, Color(255/255, 255/255, 255/255))
			UiTextBus.Event.SetText(self.ValueText, tostring(tonumber(math.floor(self.NumberReference))))
		end
	--end
	
	-- strength
	--if id == "strength" or UiTextBus.Event.GetText(UiElementBus.Event.FindDescendantByName(self.Properties.StrengthStat, "StatNumbers")) == "#" or UiTextBus.Event.GetText(UiElementBus.Event.FindDescendantByName(self.Properties.StrengthStat, "StatNumbers2")) == "#" then
		self.ValueText = UiElementBus.Event.FindDescendantByName(self.Properties.StrengthStat, "StatNumbers")
		self.BonusValueText = UiElementBus.Event.FindDescendantByName(self.Properties.StrengthStat, "StatNumbers2")
		self.NumberReference = VariableManagerRequestBus.Broadcast.GetBaseValue(VariableId(self.selectedUnit, "strength"))
		self.BonusNumberReference = VariableManagerRequestBus.Broadcast.GetBonusValue(VariableId(self.selectedUnit, "strength"))
		if self.BonusNumberReference ~= 0 then
			UiTextBus.Event.SetText(self.BonusValueText, tostring(tonumber(math.floor(self.NumberReference))))
			UiTextBus.Event.SetText(self.ValueText, tostring(tonumber(math.floor(self.BonusNumberReference))))
			UiElementBus.Event.SetIsEnabled(self.BonusValueText,true)
			if self.BonusNumberReference > 0 then
				UiTextBus.Event.SetColor(self.ValueText, Color(85/255, 255/255, 0/255))
			elseif self.BonusNumberReference < 0 then
				UiTextBus.Event.SetColor(self.ValueText, Color(255/255, 0/255, 0/255))
			end
		else
			UiElementBus.Event.SetIsEnabled(self.BonusValueText,false)
			UiTextBus.Event.SetColor(self.ValueText, Color(255/255, 255/255, 255/255))
			UiTextBus.Event.SetText(self.ValueText, tostring(tonumber(math.floor(self.NumberReference))))
		end
	--end
	
	-- agility
	--if id == "agility" or UiTextBus.Event.GetText(UiElementBus.Event.FindDescendantByName(self.Properties.AgilityStat, "StatNumbers")) == "#" or UiTextBus.Event.GetText(UiElementBus.Event.FindDescendantByName(self.Properties.AgilityStat, "StatNumbers2")) == "#" then
		self.ValueText = UiElementBus.Event.FindDescendantByName(self.Properties.AgilityStat, "StatNumbers")
		self.BonusValueText = UiElementBus.Event.FindDescendantByName(self.Properties.AgilityStat, "StatNumbers2")
		self.NumberReference = VariableManagerRequestBus.Broadcast.GetBaseValue(VariableId(self.selectedUnit, "agility"))
		self.BonusNumberReference = VariableManagerRequestBus.Broadcast.GetBonusValue(VariableId(self.selectedUnit, "agility"))
		if self.BonusNumberReference ~= 0 then
			UiTextBus.Event.SetText(self.BonusValueText, tostring(tonumber(math.floor(self.NumberReference))))
			UiTextBus.Event.SetText(self.ValueText, tostring(tonumber(math.floor(self.BonusNumberReference))))
			UiElementBus.Event.SetIsEnabled(self.BonusValueText,true)
			if self.BonusNumberReference > 0 then
				UiTextBus.Event.SetColor(self.ValueText, Color(85/255, 255/255, 0/255))
			elseif self.BonusNumberReference < 0 then
				UiTextBus.Event.SetColor(self.ValueText, Color(255/255, 0/255, 0/255))
			end
		else
			UiElementBus.Event.SetIsEnabled(self.BonusValueText,false)
			UiTextBus.Event.SetColor(self.ValueText, Color(255/255, 255/255, 255/255))
			UiTextBus.Event.SetText(self.ValueText, tostring(tonumber(math.floor(self.NumberReference))))
		end
	--end
	
	-- intelligence
	--if id == "intelligence" or UiTextBus.Event.GetText(UiElementBus.Event.FindDescendantByName(self.Properties.IntellaganceStat, "StatNumbers")) == "#" or UiTextBus.Event.GetText(UiElementBus.Event.FindDescendantByName(self.Properties.IntellaganceStat, "StatNumbers2")) == "#" then	
		self.ValueText = UiElementBus.Event.FindDescendantByName(self.Properties.IntellaganceStat, "StatNumbers")
		self.BonusValueText = UiElementBus.Event.FindDescendantByName(self.Properties.IntellaganceStat, "StatNumbers2")
		self.NumberReference = VariableManagerRequestBus.Broadcast.GetBaseValue(VariableId(self.selectedUnit, "intelligence"))
		self.BonusNumberReference = VariableManagerRequestBus.Broadcast.GetBonusValue(VariableId(self.selectedUnit, "intelligence"))
		if self.BonusNumberReference ~= 0 then
			UiTextBus.Event.SetText(self.BonusValueText, tostring(tonumber(math.floor(self.NumberReference))))
			UiTextBus.Event.SetText(self.ValueText, tostring(tonumber(math.floor(self.BonusNumberReference))))
			UiElementBus.Event.SetIsEnabled(self.BonusValueText,true)
			if self.BonusNumberReference > 0 then
				UiTextBus.Event.SetColor(self.ValueText, Color(85/255, 255/255, 0/255))
			elseif self.BonusNumberReference < 0 then
				UiTextBus.Event.SetColor(self.ValueText, Color(255/255, 0/255, 0/255))
			end
		else
			UiElementBus.Event.SetIsEnabled(self.BonusValueText,false)
			UiTextBus.Event.SetColor(self.ValueText, Color(255/255, 255/255, 255/255))
			UiTextBus.Event.SetText(self.ValueText, tostring(tonumber(math.floor(self.NumberReference))))
		end
	--end
	
	-- will
	--if id == "will" or UiTextBus.Event.GetText(UiElementBus.Event.FindDescendantByName(self.Properties.WillStat, "StatNumbers")) == "#" or UiTextBus.Event.GetText(UiElementBus.Event.FindDescendantByName(self.Properties.WillStat, "StatNumbers2")) == "#" then	
		self.ValueText = UiElementBus.Event.FindDescendantByName(self.Properties.WillStat, "StatNumbers")
		self.BonusValueText = UiElementBus.Event.FindDescendantByName(self.Properties.WillStat, "StatNumbers2")
		self.NumberReference = VariableManagerRequestBus.Broadcast.GetBaseValue(VariableId(self.selectedUnit, "will"))
		self.BonusNumberReference = VariableManagerRequestBus.Broadcast.GetBonusValue(VariableId(self.selectedUnit, "will"))
		if self.BonusNumberReference ~= 0 then
			UiTextBus.Event.SetText(self.BonusValueText, tostring(tonumber(math.floor(self.NumberReference))))
			UiTextBus.Event.SetText(self.ValueText, tostring(tonumber(math.floor(self.BonusNumberReference))))
			UiElementBus.Event.SetIsEnabled(self.BonusValueText,true)
			if self.BonusNumberReference > 0 then
				UiTextBus.Event.SetColor(self.ValueText, Color(85/255, 255/255, 0/255))
			elseif self.BonusNumberReference < 0 then
				UiTextBus.Event.SetColor(self.ValueText, Color(255/255, 0/255, 0/255))
			end
		else
			UiElementBus.Event.SetIsEnabled(self.BonusValueText,false)
			UiTextBus.Event.SetColor(self.ValueText, Color(255/255, 255/255, 255/255))
			UiTextBus.Event.SetText(self.ValueText, tostring(tonumber(math.floor(self.NumberReference))))
		end	
	--end
end

return Stats;