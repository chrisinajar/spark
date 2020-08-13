require "scripts/library/timers"
require "scripts.common";

local PickScreenAbilityScript = 
{
	CurrentAbility = nil;
	OwnButton = nil;
	Info = nil;
	abilityHandler = nil;
	ToolTip = nil;
	AbilityPanel = nil;
	spawnTickets = {};
	init = false;
	Properties = 
	{
		ManaText = {default = EntityId()},	
		Image = {default = EntityId()},	 
		CurrentLevelTextColour = {default = Color(255/255, 255/255, 255/255)},
		NonCurrentLevelTextColour = {default = Color(0/255, 0/255, 0/255)},
		LeveledColour = {default = Color(255/255, 255/255, 255/255)},
		DisabledColour = {default = Color(126/255, 126/255, 126/255)},
	}
}

-- On Activate 
function PickScreenAbilityScript:OnActivate()
	self.OwnButton = UiElementBus.Event.FindChildByName(self.entityId, "Button")
	self.canvasNotificationHandler = nil	
	self.handler = InputMapperNotificationBus.Connect(self);	
	
	CreateTimer(function() 
        self:Init() 
    end,0.1);
end

function PickScreenAbilityScript:Init()
	self.CanvasEntityId = self.CanvasEntityId or UiElementBus.Event.GetCanvas(self.entityId)
	if self.CanvasEntityId ~= nil then
		if self.canvasNotificationHandler then
			self.canvasNotificationHandler:Disconnect()
			self.canvasNotificationHandler = nil
		end		
		self.canvasNotificationHandler = UiCanvasNotificationBus.Connect(self, self.CanvasEntityId)
		self.ToolTip = self.Tooltip or UiCanvasBus.Event.FindElementByName(self.CanvasEntityId, "AbilityDescriptor")
		self.AbilityPanel = self.AbilityPanel or UiCanvasBus.Event.FindElementByName(self.CanvasEntityId, "Abilities")
		self.ChosenHero = self.ChosenHero or UiCanvasBus.Event.FindElementByName(self.CanvasEntityId, "ChosenHero")
	end
	
	if UiElementBus.Event.IsEnabled(self.ChosenHero) == true and self.init == false and self.OwnButton ~= nil then
		local parentSlot = UiElementBus.Event.GetParent(self.OwnButton)
		if parentSlot ~= nil then
			self.CurrentAbility = UiTextBus.Event.GetText(UiElementBus.Event.FindDescendantByName(parentSlot, "AbilityName"))
		end
	end
	
	--Setting Ability Button Images
	if self.CurrentAbility ~= nil then
		if GetAbilityInfo(self.CurrentAbility) ~= nil then
			local Info = GetAbilityInfo(self.CurrentAbility);
			if Info~=nil and Info["icon-file"] ~= nil then
				local AbilityImage = Info["icon-file"]
				local ImageString = tostring(tostring(AbilityImage) .. ".dds")
				UiImageBus.Event.SetSpritePathname(self.Properties.Image, ImageString)
			else
				UiImageBus.Event.SetSpritePathname(self.Properties.Image, "textures/core.dds")
			end
			self.init = true
		end
	end	
end

function PickScreenAbilityScript:OnEntitySpawned(ticket, id)

end

function PickScreenAbilityScript:OnSpawnEnd(ticket)

end

function PickScreenAbilityScript:OnSpawnFailed(ticket)

end

-- Deactivate 
function PickScreenAbilityScript:OnDeactivate()
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

-- Actions In Ability Script
function PickScreenAbilityScript:OnAction(entityId, actionName) 
	if self.OwnButton == entityId then
		-- Ability Tool Tip
		if actionName == "AbilityHovering" then
			local AbilityButton = UiElementBus.Event.GetParent(entityId)
			local text = UiTextBus.Event.GetText(UiElementBus.Event.FindDescendantByName(AbilityButton, "AbilityName"))
			local CurrentHoverAbility = text
				
			self:UpdateToolTip(CurrentHoverAbility)
		end
	end
end

-- Tool Tip Function
function PickScreenAbilityScript:UpdateToolTip(ability)
	-- Check If Ability Is Valid
	if not ability then
		return;
	end	
	
	local ItemPanel = UiElementBus.Event.FindDescendantByName(self.ToolTip, "Item")
	local AbilityPanel = UiElementBus.Event.FindDescendantByName(self.ToolTip, "Ability")
	UiElementBus.Event.SetIsEnabled(ItemPanel, false)
	UiElementBus.Event.SetIsEnabled(AbilityPanel, true)
	
	local StatArea = UiElementBus.Event.FindDescendantByName(AbilityPanel, "AbilityStats")
	
	local NamePanel = UiElementBus.Event.FindDescendantByName(AbilityPanel, "AbilityName")
	local LevelPanel = UiElementBus.Event.FindDescendantByName(AbilityPanel, "AbilityLevel")
	local DescriptionPanel = UiElementBus.Event.FindDescendantByName(AbilityPanel, "AbilityDescription")
	
	UiElementBus.Event.SetIsEnabled(LevelPanel, false)
	
	local CoolDownArea = UiElementBus.Event.FindDescendantByName(AbilityPanel, "CoolDownPanel")
	local ManaPanel = UiElementBus.Event.FindDescendantByName(AbilityPanel, "ManaPanel")
	
	local CoolDownTextPanel = UiElementBus.Event.FindDescendantByName(AbilityPanel, "CoolDownTextPanel")
	local ManaTextPanel = UiElementBus.Event.FindDescendantByName(AbilityPanel, "ManaTextPanel")	
	
	-- Assign Ability Info
	if self.Info ~= ability or self.Info == nil then
		self.Info =  GetAbilityInfo(ability)
	end
	
	-- Set Tool Tip Ability Name
	if ability ~= nil then
		--Debug.Log("LOOK HERE FOR ABILITY CASTING BEHAVIOR !" .. tostring(Ability.GetCastingBehavior({entityId = ability})))
		local values=splitstring(ability, "/")
		file_name = tostring(values[2])
		localizeText =  LocalizationRequestBus.Broadcast.LocalizeString("@ui_ability_" .. tostring(file_name))
		UiTextBus.Event.SetText(NamePanel, localizeText)
		descriptionLocalizeText = LocalizationRequestBus.Broadcast.LocalizeString("@ui_ability_" .. tostring(file_name) .. "_description")
		UiTextBus.Event.SetText(DescriptionPanel, descriptionLocalizeText)
	else
		UiTextBus.Event.SetText(NamePanel, "Ability Name")
	end
	
	--needs to be updated once max level can be found, currently setting to 4 for default
	local MaxLevel = self.Info.maxlevel or 4
	Debug.Log(tostring(MaxLevel))
    for i=1,MaxLevel do 
		local CoolDownText = UiElementBus.Event.GetChild(CoolDownTextPanel,i-1)
		local ManaText = UiElementBus.Event.GetChild(ManaTextPanel,i-1)		
		
		-- Update Tool Tip Cooldown
		if self.Info.cooldown ~= nil then
			UiElementBus.Event.SetIsEnabled(CoolDownArea, true)
			if GetKvValue(self.Info.cooldown,i) ~= nil then	
				if i <= 1 then			
					UiElementBus.Event.SetIsEnabled(CoolDownText, true)
					UiTextBus.Event.SetText(CoolDownText, GetKvValue(self.Info.cooldown,i))
					UiTextBus.Event.SetColor(CoolDownText, self.Properties.CurrentLevelTextColour)
				elseif i >= 2 and GetKvValue(self.Info.cooldown,i) ~= GetKvValue(self.Info.cooldown,1) then
					UiElementBus.Event.SetIsEnabled(CoolDownText, true)
					UiTextBus.Event.SetText(CoolDownText, GetKvValue(self.Info.cooldown,i))
					UiTextBus.Event.SetColor(CoolDownText, self.Properties.CurrentLevelTextColour)
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
						UiTextBus.Event.SetColor(ManaText, self.Properties.CurrentLevelTextColour)
					elseif i >= 2 and GetKvValue(self.Info.costs.mana,i) ~= GetKvValue(self.Info.costs.mana,1) then
						UiElementBus.Event.SetIsEnabled(ManaText, true)
						UiTextBus.Event.SetText(ManaText, GetKvValue(self.Info.costs.mana,i))
						UiTextBus.Event.SetColor(ManaText, self.Properties.CurrentLevelTextColour)
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
							UiTextBus.Event.SetColor(StatText, self.Properties.CurrentLevelTextColour)
						elseif i >= 2 and GetKvValue(tostring(json),i) ~= GetKvValue(tostring(json),1) then
							UiElementBus.Event.SetIsEnabled(StatText, true)
							UiTextBus.Event.SetText(StatText, GetKvValue(tostring(json),i))
							UiTextBus.Event.SetColor(StatText, self.Properties.CurrentLevelTextColour)
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

return PickScreenAbilityScript;