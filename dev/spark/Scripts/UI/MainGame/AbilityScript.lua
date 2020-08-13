require "scripts/library/timers"
require "scripts.common"
require "gamemode.Spark.modules.hero_level_xp"

local AbilityScript = 
{
	selectedUnit = nil;
	CurrentAbility = nil;
	OnCoolDown = false;
	OwnButton = nil;
	AbilityToLevel = nil;
	Info = nil;
	abilityHandler = nil;
	ToolTip = nil;
	AbilityPanel = nil;
	spawnTickets = {};
	Properties = 
	{
		ManaText = {default = EntityId()},
		CooldownTextRef = {default = EntityId()},
		CoolddownTimer = {default = EntityId()},
		CooldownPanel = {default = EntityId()},
		KeyBindText = {default = EntityId()},
		LevelUpPanel = {default = EntityId()},	
		Image = {default = EntityId()},	 
		DisabledImage = {default = EntityId()},	
		AbilityLevelPanel = {default = EntityId()},
		LevelUpBackground = {default = EntityId()},
		CurrentLevelTextColour = {default = Color(255/255, 255/255, 255/255)},
		NonCurrentLevelTextColour = {default = Color(0/255, 0/255, 0/255)},
		LeveledColour = {default = Color(255/255, 255/255, 255/255)},
		DisabledColour = {default = Color(126/255, 126/255, 126/255)},
		ActiveImage = {default = EntityId()},
		Particle = {default = EntityId()},
	}
}

-- On Activate 
function AbilityScript:OnActivate()	
	self.tickBusHandler = TickBus.Connect(self)
	CreateTimer(function() 
		self:Init() 
	end,0.1);
end

function AbilityScript:Init()
	self.OwnButton = UiElementBus.Event.FindChildByName(self.entityId, "Button")
	self.canvasNotificationHandler = nil	
	self.handler = InputMapperNotificationBus.Connect(self);
	self.selectionHandler = SelectionNotificationBus.Connect(self)
	
	self.CanvasEntityId = self.CanvasEntityId or UiElementBus.Event.GetCanvas(self.entityId)
	if self.CanvasEntityId ~= nil then
		if self.canvasNotificationHandler then
			self.canvasNotificationHandler:Disconnect()
			self.canvasNotificationHandler = nil
		end		
		self.canvasNotificationHandler = UiCanvasNotificationBus.Connect(self, self.CanvasEntityId)
		self.ToolTip = self.Tooltip or UiCanvasBus.Event.FindElementByName(self.CanvasEntityId, "AbilityDescriptor")
		self.AbilityPanel = self.AbilityPanel or UiCanvasBus.Event.FindElementByName(self.CanvasEntityId, "AbilitesPanel")
	end 
	self:InitUnit()
end

function AbilityScript:InitUnit()
	self.selectedUnit = SelectionRequestBus.Broadcast.GetMainSelectedUnit()
	self:OnMainSelectedUnitChanged(self.selectedUnit)
end

function AbilityScript:UpdateAbilitySlot(key)
	-- Setting KeyBinds For Abilties
	local keys=key:GetKeys();
	for i=1, #keys do
		if i == 2 then
			UiTextBus.Event.SetText(self.Properties.KeyBindText, KeyToString(keys[2]) .. "\n" .. KeyToString(keys[1]))
		else 
			UiTextBus.Event.SetText(self.Properties.KeyBindText, KeyToString(keys[i]))
		end		
	end	
	
	--Setting Ability Button Images
	if self.CurrentAbility ~= nil then
		if self.Info ~= GetAbilityInfo(self.CurrentAbility) or self.Info == nil then		
			self.Info = GetAbilityInfo(self.CurrentAbility);
			if self.Info~=nil and self.Info["icon-file"] ~= nil then
				local AbilityImage = self.Info["icon-file"]
				local ImageString = tostring(tostring(AbilityImage) .. ".dds")
				UiImageBus.Event.SetSpritePathname(self.Properties.Image, ImageString)
			else
				UiImageBus.Event.SetSpritePathname(self.Properties.Image, "textures/core.dds")
			end
		end
		--Spawning Level Icons Under Abilities
		local childCount = UiElementBus.Event.GetChildren(self.Properties.AbilityLevelPanel)
		for i = 1,#childCount do
			UiElementBus.Event.DestroyElement(UiElementBus.Event.GetChild(self.Properties.AbilityLevelPanel, i))
		end
		for i=1, Ability.GetMaxLevel({entityId = self.CurrentAbility}) do
			if #UiElementBus.Event.GetChildren(self.Properties.AbilityLevelPanel) == 0 then
				self.spawnTickets[i] = UiSpawnerBus.Event.Spawn(self.Properties.AbilityLevelPanel)
			end
		end
	end	
end

function AbilityScript:OnEntitySpawned(ticket, id)

end

function AbilityScript:OnSpawnEnd(ticket)

end

function AbilityScript:OnSpawnFailed(ticket)

end

function AbilityScript:OnTick(deltaTime, timePoint)
	-- Check If On Cooldown Or Not
	if(self.OnCoolDown) then
		if self.selectedUnit == SelectionRequestBus.Broadcast.GetMainSelectedUnit() then
			if Ability.GetLevel({entityId = self.CurrentAbility}) >= 1 then
				UiElementBus.Event.SetIsEnabled(self.Properties.CooldownPanel, true)
				self:UpdateCooldown()			
			else
				UiElementBus.Event.SetIsEnabled(self.Properties.CooldownPanel, false)
			end
		else
			UiElementBus.Event.SetIsEnabled(self.Properties.CooldownPanel, false)
		end
	else
		UiElementBus.Event.SetIsEnabled(self.Properties.CooldownPanel, false)	
	end
	
	-- Checks Ability Level And Cooldown
	if self.CurrentAbility ~= nil then
		if self.selectedUnit == SelectionRequestBus.Broadcast.GetMainSelectedUnit() then
			self:AbilityLevelledUp(self.CurrentAbility)
			if self.abilityHandler then
				self.abilityHandler:Disconnect()
				self.abilityHandler = nil
			end
			self.abilityHandler = AbilityNotificationBus.Connect(self, self.CurrentAbility)
		end
		if VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.CurrentAbility,"cooldown_timer")) > 0 then
			self.OnCoolDown = true
		end
	end	
end

function AbilityScript:OnMainSelectedUnitChanged(unit)
	local ButtonSlot = UiElementBus.Event.GetParent(self.entityId)
	if ButtonSlot ~= nil then
		local index = UiElementBus.Event.GetIndexOfChildByEntityId(self.AbilityPanel, ButtonSlot)
		if index ~= nil then
			self.CurrentAbility = UnitAbilityRequestBus.Event.GetAbilityInSlot(self.selectedUnit,Slot(Slot.Ability,index))					
			self:UpdateAbilityStats(self.CurrentAbility)
			local SelfOwner = UiElementBus.Event.FindChildByName(self.entityId, "AbilityKey")
			if self.Properties.KeyBindText == UiElementBus.Event.FindChildByName(SelfOwner, "AbilityKeyText") then
				key = InputMapperRequestBus.Broadcast.GetSlotHotkey(Slot(Slot.Ability,index))
				self:UpdateAbilitySlot(key)
			end
		end
	end
end

-- Deactivate 
function AbilityScript:OnDeactivate()
	self.tickBusHandler:Disconnect()
	if self.abilityHandler then
		self.abilityHandler:Disconnect()
	end
	if self.canvasNotificationHandler then
		self.canvasNotificationHandler:Disconnect()
	end
	if self.selectionHandler then
		self.selectionHandler:Disconnect()
	end
end

-- On Spell Start
function AbilityScript:OnSpellStart()
	UiElementBus.Event.SetIsEnabled(self.Properties.ActiveImage, false)
	self.OnCoolDown = true
end

-- Actions In Ability Script
function AbilityScript:OnAction(entityId, actionName) 
local pressed = false
	if self.OwnButton == entityId then
		-- Ability Pressed 
		if actionName == "AbilityPressed" then
			if pressed == false then
				pressed = true
				if self.OwnButton == entityId then
					if Ability.GetLevel({entityId = self.CurrentAbility}) > 0 then
						UiElementBus.Event.SetIsEnabled(self.Properties.ActiveImage, true)						
					end				
				end
			end
		end
		
		-- Ability Tool Tip
		if actionName == "AbilityHovering" then
			local AbilityButton = UiElementBus.Event.GetParent(entityId)
			local ButtonSlot = UiElementBus.Event.GetParent(AbilityButton)
			local index = UiElementBus.Event.GetIndexOfChildByEntityId(self.AbilityPanel, ButtonSlot)
			local CurrentHoverAbility = UnitAbilityRequestBus.Event.GetAbilityInSlot(self.selectedUnit,Slot(Slot.Ability,index))
				
			self:UpdateToolTip(CurrentHoverAbility)			
		end
		
		-- Level Up Abilities
		if actionName == "LevelUpAbility" then
			local LevelUpButton = UiElementBus.Event.GetParent(entityId)
			local ButtonSlot = UiElementBus.Event.GetParent(LevelUpButton)
			local index = UiElementBus.Event.GetIndexOfChildByEntityId(self.AbilityPanel, ButtonSlot)
			self.AbilityToLevel = UnitAbilityRequestBus.Event.GetAbilityInSlot(self.selectedUnit,Slot(Slot.Ability,index))
		end
		pressed = false
	end
end

-- Update Ability Costs 
function AbilityScript:UpdateAbilityStats(ability)	
	if ability ~= nil then
		local vector = AbilityRequestBus.Event.GetCosts(ability);
		UiTextBus.Event.SetText(self.Properties.ManaText,"");
		if( not vector ) then 
			return;
		end
		for i=1, #vector do
			if vector[i].variable == "mana" then
				UiTextBus.Event.SetText(self.Properties.ManaText , tostring(tonumber(math.floor(vector[i].amount))))
			end
		end	
	end	
end

-- Tool Tip Function
function AbilityScript:UpdateToolTip(ability)
	-- Check If Ability Is Valid
	if( not ability or not ability:IsValid()) then
		return;
	end	
	
	--Debug.Log(tostring(AbilityRequestBus.Event.GetAbilityName(ability)))
	
	local cooldown_reduction = VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.selectedUnit, "cooldown_reduction"))
	local ItemPanel = UiElementBus.Event.FindDescendantByName(self.ToolTip, "Item")
	local AbilityPanel = UiElementBus.Event.FindDescendantByName(self.ToolTip, "Ability")
	local ModifierPanel = UiElementBus.Event.FindDescendantByName(self.ToolTip, "Modifier")
	UiElementBus.Event.SetIsEnabled(ItemPanel, false)
	UiElementBus.Event.SetIsEnabled(AbilityPanel, true)
	UiElementBus.Event.SetIsEnabled(ModifierPanel, false)
	
	local StatArea = UiElementBus.Event.FindDescendantByName(AbilityPanel, "AbilityStats")
	
	local NamePanel = UiElementBus.Event.FindDescendantByName(AbilityPanel, "AbilityName")
	local LevelPanel = UiElementBus.Event.FindDescendantByName(AbilityPanel, "AbilityLevel")
	local DescriptionPanel = UiElementBus.Event.FindDescendantByName(AbilityPanel, "AbilityDescription")
	
	UiTextBus.Event.SetText(LevelPanel, tostring("Level:"..Ability.GetLevel({entityId = ability})))
	
	local CoolDownArea = UiElementBus.Event.FindDescendantByName(AbilityPanel, "CoolDownPanel")
	local ManaPanel = UiElementBus.Event.FindDescendantByName(AbilityPanel, "ManaPanel")
	
	local CoolDownTextPanel = UiElementBus.Event.FindDescendantByName(AbilityPanel, "CoolDownTextPanel")
	local ManaTextPanel = UiElementBus.Event.FindDescendantByName(AbilityPanel, "ManaTextPanel")	
	
	-- Assign Ability Info
	if self.Info ~= GetAbilityInfo(ability) or self.Info == nil then
		self.Info = GetAbilityInfo(ability);
	end
	
	-- Set Tool Tip Ability Name
	if Ability.GetType({entityId = ability}) ~= nil then
		Debug.Log("LOOK HERE FOR ABILITY CASTING BEHAVIOR !" .. tostring(Ability.GetCastingBehavior({entityId = ability})))
		Debug.Log(tostring(self.selectedUnit) .. ' has tag unit ' .. tostring(Crc32("unit")) .. '? ' .. tostring(HasTag(self.selectedUnit, "unit")))

		local values=splitstring(Ability.GetType({entityId = ability}), "/")
		file_name = tostring(values[2])
		localizeText =  LocalizationRequestBus.Broadcast.LocalizeString("@ui_ability_" .. tostring(file_name))
		UiTextBus.Event.SetText(NamePanel, localizeText)
		descriptionLocalizeText = LocalizationRequestBus.Broadcast.LocalizeString("@ui_ability_" .. tostring(file_name) .. "_description")
		UiTextBus.Event.SetText(DescriptionPanel, descriptionLocalizeText)
	else
		UiTextBus.Event.SetText(NamePanel, "Ability Name")
	end
	
	local MaxLevel = Ability.GetMaxLevel({entityId = ability})
    for i=1,MaxLevel do 
		local CoolDownText = UiElementBus.Event.GetChild(CoolDownTextPanel,i-1)
		local ManaText = UiElementBus.Event.GetChild(ManaTextPanel,i-1)		
		
		-- Update Tool Tip Cooldown
		if self.Info.cooldown ~= nil then
			UiElementBus.Event.SetIsEnabled(CoolDownArea, true)
			if GetKvValue(self.Info.cooldown,i) ~= nil then	
				if i <= 1 then			
					UiElementBus.Event.SetIsEnabled(CoolDownText, true)
					if cooldown_reduction > 0 then
						UiTextBus.Event.SetText(CoolDownText, GetKvValue(self.Info.cooldown,i) * (1-cooldown_reduction))
					else
						UiTextBus.Event.SetText(CoolDownText, GetKvValue(self.Info.cooldown,i))
					end
					if i == Ability.GetLevel({entityId = ability}) then
						UiTextBus.Event.SetColor(CoolDownText, self.Properties.CurrentLevelTextColour)
					else
						UiTextBus.Event.SetColor(CoolDownText, self.Properties.NonCurrentLevelTextColour)
					end
				elseif i >= 2 and GetKvValue(self.Info.cooldown,i) ~= GetKvValue(self.Info.cooldown,1) then
					UiElementBus.Event.SetIsEnabled(CoolDownText, true)
					UiTextBus.Event.SetText(CoolDownText, GetKvValue(self.Info.cooldown,i))
					if i == Ability.GetLevel({entityId = ability}) then
						UiTextBus.Event.SetColor(CoolDownText, self.Properties.CurrentLevelTextColour)
					else
						UiTextBus.Event.SetColor(CoolDownText, self.Properties.NonCurrentLevelTextColour)
					end
				elseif i >= 2 and GetKvValue(self.Info.cooldown,i) == GetKvValue(self.Info.cooldown,1) then
					UiElementBus.Event.SetIsEnabled(CoolDownText, false)
				end
			else
				UiElementBus.Event.SetIsEnabled(CoolDownText, false)
			end
		else
			UiElementBus.Event.SetIsEnabled(CoolDownArea, false)	
		end
		
		-- Update Tool Tip Mana Cost
		if self.Info.costs ~= nil then
			UiElementBus.Event.SetIsEnabled(ManaPanel, true)
			if self.Info.costs.mana ~= nil then
				if GetKvValue(self.Info.costs.mana,i) ~= nil then
					if i <= 1 then
						UiElementBus.Event.SetIsEnabled(ManaText, true)
						UiTextBus.Event.SetText(ManaText, GetKvValue(self.Info.costs.mana,i))
						if i == Ability.GetLevel({entityId = ability}) then
							UiTextBus.Event.SetColor(ManaText, self.Properties.CurrentLevelTextColour)
						else
							UiTextBus.Event.SetColor(ManaText, self.Properties.NonCurrentLevelTextColour)
						end
					elseif i >= 2 and GetKvValue(self.Info.costs.mana,i) ~= GetKvValue(self.Info.costs.mana,1) then
						UiElementBus.Event.SetIsEnabled(ManaText, true)
						UiTextBus.Event.SetText(ManaText, GetKvValue(self.Info.costs.mana,i))
						if i == Ability.GetLevel({entityId = ability}) then
							UiTextBus.Event.SetColor(ManaText, self.Properties.CurrentLevelTextColour)
						else
							UiTextBus.Event.SetColor(ManaText, self.Properties.NonCurrentLevelTextColour)
						end
					elseif i >= 2 and GetKvValue(self.Info.costs.mana,i) == GetKvValue(self.Info.costs.mana,1) then
						UiElementBus.Event.SetIsEnabled(ManaText, false)
					end
				else
					UiElementBus.Event.SetIsEnabled(ManaText, false)
				end	
			else
				UiElementBus.Event.SetIsEnabled(ManaPanel, false)	
			end	
		else
			UiElementBus.Event.SetIsEnabled(ManaPanel, false)	
		end
		if self.Info["special-values"] ~= nil then
			local count = 0
			for i=1,9 do 
				UiElementBus.Event.SetIsEnabled(UiElementBus.Event.GetChild(StatArea,i),false)
			end
			for k,v in pairs(self.Info["special-values"]) do			
				if type(k) == "string" then
					count = count + 1
					self.statName = k
					local child = UiElementBus.Event.GetChild(StatArea,count)
					UiElementBus.Event.SetIsEnabled(child,true)
					local name = UiElementBus.Event.GetChild(child,0)
					local stat = UiElementBus.Event.GetChild(child,1)
					local StatText = UiElementBus.Event.GetChild(stat,i-1)
					statLocalizeText =  LocalizationRequestBus.Broadcast.LocalizeString("@ui_ability_" .. tostring(file_name) .. "_" .. tostring(k))
					UiTextBus.Event.SetText(name,statLocalizeText)
					
					local json = self.Info["special-values"][k]
					if GetKvValue(tostring(json),i) ~= nil then
						if i <= 1 then
							UiElementBus.Event.SetIsEnabled(StatText, true)
							UiTextBus.Event.SetText(StatText, GetKvValue(tostring(json),i))
							if i == Ability.GetLevel({entityId = ability}) then
								UiTextBus.Event.SetColor(StatText, self.Properties.CurrentLevelTextColour)
							else
								UiTextBus.Event.SetColor(StatText, self.Properties.NonCurrentLevelTextColour)
							end
						elseif i >= 2 and GetKvValue(tostring(json),i) ~= GetKvValue(tostring(json),1) then
							UiElementBus.Event.SetIsEnabled(StatText, true)
							UiTextBus.Event.SetText(StatText, GetKvValue(tostring(json),i))
							if i == Ability.GetLevel({entityId = ability}) then
								UiTextBus.Event.SetColor(StatText, self.Properties.CurrentLevelTextColour)
							else
								UiTextBus.Event.SetColor(StatText, self.Properties.NonCurrentLevelTextColour)
							end
						elseif i >= 2 and GetKvValue(tostring(json),i) == GetKvValue(tostring(json),1) then
							UiElementBus.Event.SetIsEnabled(StatText, false)
						end
					else
						UiElementBus.Event.SetIsEnabled(StatText, false)
					end
				end			
			end
		end
    end	
end

-- Level Up Ability
function AbilityScript:AbilityLevelledUp(ability)
	-- Set Ability Level Icon
	for i=0, Ability.GetMaxLevel({entityId = ability})-1 do
		if i >= Ability.GetLevel({entityId = ability}) then
			local levelChild = UiElementBus.Event.GetChild(self.Properties.AbilityLevelPanel,i)
			UiImageBus.Event.SetColor(levelChild, self.Properties.DisabledColour)
		else
			local levelChild = UiElementBus.Event.GetChild(self.Properties.AbilityLevelPanel,i)
			UiImageBus.Event.SetColor(levelChild, self.Properties.LeveledColour)
		end
	end
	-- Update Ability Stats
	self:UpdateAbilityStats(ability)
	
	-- Update Level For Ability Costs And Level
	if Ability.GetLevel({entityId = ability}) >= 1 then
		if UiElementBus.Event.IsEnabled(self.Properties.DisabledImage) == true then
			local castingBehavior = Ability.GetCastingBehavior({entityId = ability})
			if castingBehavior:Contains(CastingBehavior.PASSIVE) then
				UiElementBus.Event.SetIsEnabled(self.Properties.KeyBindText, false)
				UiElementBus.Event.SetIsEnabled(self.Properties.ManaText, false)
			else
				UiElementBus.Event.SetIsEnabled(self.Properties.KeyBindText, true)
				UiElementBus.Event.SetIsEnabled(self.Properties.ManaText, true)
			end
			UiElementBus.Event.SetIsEnabled(self.Properties.DisabledImage, false)
			UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.DisabledImage, false)
		end
	else
		UiElementBus.Event.SetIsEnabled(self.Properties.KeyBindText, false)
		UiElementBus.Event.SetIsEnabled(self.Properties.ManaText, false)
		UiElementBus.Event.SetIsEnabled(self.Properties.DisabledImage, true)
		UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.DisabledImage, true)
	end
	
	--Checks Ability Points
	if VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.selectedUnit, "ability_points")) > 0 and Ability.GetLevel({entityId = ability}) ~= Ability.GetMaxLevel({entityId = ability}) then
		local playerLevel = tonumber(VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.selectedUnit, "level")))
		--Debug.Log("ABILITY ULT BOOL IS : " .. tostring(Ability.GetIsUltimate({entityId = ability})))
		if self.Info ~= GetAbilityInfo(ability) or self.Info == nil then
			self.Info = GetAbilityInfo(ability);
		end
		if self.Info.is_ultimate == nil or self.Info.is_ultimate == false then
			if LevelingXP[playerLevel][4] > Ability.GetLevel({entityId = ability}) then
				UiElementBus.Event.SetIsEnabled(self.Properties.LevelUpPanel, true)
				UiElementBus.Event.SetIsEnabled(self.Properties.LevelUpBackground, true)
				UiElementBus.Event.SetIsEnabled(self.Properties.Particle, true)
			else
				UiElementBus.Event.SetIsEnabled(self.Properties.LevelUpPanel, false)
				UiElementBus.Event.SetIsEnabled(self.Properties.LevelUpBackground, false)
				UiElementBus.Event.SetIsEnabled(self.Properties.Particle, false)
			end
		elseif self.Info.is_ultimate == true then
			if LevelingXP[playerLevel][5] > Ability.GetLevel({entityId = ability}) then
				UiElementBus.Event.SetIsEnabled(self.Properties.LevelUpPanel, true)
				UiElementBus.Event.SetIsEnabled(self.Properties.LevelUpBackground, true)
				UiElementBus.Event.SetIsEnabled(self.Properties.Particle, true)
			
			else
				UiElementBus.Event.SetIsEnabled(self.Properties.LevelUpPanel, false)
				UiElementBus.Event.SetIsEnabled(self.Properties.LevelUpBackground, false)
				UiElementBus.Event.SetIsEnabled(self.Properties.Particle, false)
			end
		else
			UiElementBus.Event.SetIsEnabled(self.Properties.LevelUpPanel, false)
			UiElementBus.Event.SetIsEnabled(self.Properties.LevelUpBackground, false)
			UiElementBus.Event.SetIsEnabled(self.Properties.Particle, false)
		end
	else
		UiElementBus.Event.SetIsEnabled(self.Properties.LevelUpPanel, false)
		UiElementBus.Event.SetIsEnabled(self.Properties.LevelUpBackground, false)
		UiElementBus.Event.SetIsEnabled(self.Properties.Particle, false)
	end
end

-- Update Cooldown
function AbilityScript:UpdateCooldown()
	if self.OwnButton == UiElementBus.Event.FindChildByName(self.entityId, "Button") and self.selectedUnit == SelectionRequestBus.Broadcast.GetMainSelectedUnit() and Ability.GetLevel({entityId = self.CurrentAbility}) >= 1 then 
		if self.OnCoolDown == true then
			local coolDownText = VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.CurrentAbility,"cooldown_timer"))
			local coolDownMaxText = VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.CurrentAbility,"cooldown"))
			if coolDownText >= tonumber("2.0") then
				UiTextBus.Event.SetText(self.Properties.CooldownTextRef, tostring(tonumber(math.floor(coolDownText))))
			elseif coolDownText < tonumber("2.0") and coolDownText >= tonumber("0.0") then
				UiTextBus.Event.SetText(self.Properties.CooldownTextRef, tostring(tonumber(string.format("%" .. 0.1 .. "f",coolDownText))))
			end
			UiImageBus.Event.SetFillAmount(self.Properties.CoolddownTimer,coolDownText / coolDownMaxText)
			if  UiElementBus.Event.IsEnabled(self.Properties.CooldownPanel) and VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.CurrentAbility,"cooldown_timer")) <= 0.01 then
				self.OnCoolDown = false
				UiElementBus.Event.SetIsEnabled(self.Properties.CooldownPanel, false)
			end
		end
	end
end

return AbilityScript;