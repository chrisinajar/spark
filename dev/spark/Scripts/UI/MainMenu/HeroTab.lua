--require "scripts.library.ui_events"

local HeroTab = 
{
	SettingCanvas = nil;
	selectedHero = nil;
	spawnTickets = {};
	initComplete = false;
	CurrentAbilities = {};
	Properties =
	{
		SelectedHero = {default = EntityId()},
		
		Strength = {default = EntityId()},
		Agility = {default = EntityId()},
		Intelligence = {default = EntityId()},
		Will = {default = EntityId()},
		
		AbilityPanel = {default = EntityId()},
		
		ToolTip = {default = EntityId()},
	},	
}

function HeroTab:OnTick(deltaTime, timePoint)
    self.tickBusHandler:Disconnect()
end

function HeroTab:OnActivate()
	self.tickBusHandler = TickBus.Connect(self);
	--[[CreateTimer(function() 
        self:Init() 
    end,0.1);]]
end

function HeroTab:Init()
	self.CanvasEntityId = UiElementBus.Event.GetCanvas(self.entityId)
	if self.CanvasEntityId and not self.canvasNotificationHandler then
	   if self.canvasNotificationHandler then
			self.canvasNotificationHandler:Disconnect()
			self.canvasNotificationHandler = nil
		end		
		self.canvasNotificationHandler = UiCanvasNotificationBus.Connect(self, self.CanvasEntityId)
		self.ToolTip = self.Tooltip or self.Properties.ToolTip
	end

	self.unitList = InfoRequestBus.Broadcast.GetHeroList()
	
	for i=1, #self.unitList do
		if self.unitList[i] then
			local info = GetUnitInfo(self.unitList[i])
			local PrimeAttribute = tostring(info["primary_attribute"] or info["attribute_primary"])
			if string.lower(PrimeAttribute) == "strength" then
				self.spawnerHandler = UiSpawnerNotificationBus.Connect(self, self.Properties.Strength)
				self.spawnTickets[i] = UiSpawnerBus.Event.Spawn(self.Properties.Strength)
			elseif string.lower(PrimeAttribute) == "will" then
				self.spawnerHandler = UiSpawnerNotificationBus.Connect(self, self.Properties.Will)
				self.spawnTickets[i] = UiSpawnerBus.Event.Spawn(self.Properties.Will)
			elseif string.lower(PrimeAttribute) == "agility" then
				self.spawnerHandler = UiSpawnerNotificationBus.Connect(self, self.Properties.Agility)
				self.spawnTickets[i] = UiSpawnerBus.Event.Spawn(self.Properties.Agility)
			elseif string.lower(PrimeAttribute) == "intelligence" then
				self.spawnerHandler = UiSpawnerNotificationBus.Connect(self, self.Properties.Intelligence)
				self.spawnTickets[i] = UiSpawnerBus.Event.Spawn(self.Properties.Intelligence)
			end
		end
	end	
	self.initComplete = true
end

function HeroTab:OnDeactivate()
	self.tickBusHandler:Disconnect()
	self.tickBusHandler = nil
	if self.canvasNotificationHandler then
		self.canvasNotificationHandler:Disconnect()
		self.canvasNotificationHandler = nil
	end
end

function HeroTab:OnEntitySpawned(ticket, id)
	for i=1, #self.unitList do
		if self.spawnTickets[i] == ticket then
			local childText = UiElementBus.Event.FindDescendantByName(id, "Name")
			UiTextBus.Event.SetText(childText, tostring(self.unitList[i]))
		end
	end
	for i = 1, #self.CurrentAbilities do
		if self.spawnTickets[i] == ticket then
			local text = UiElementBus.Event.FindDescendantByName(id, "AbilityName")
			UiTextBus.Event.SetText(text, tostring(self.CurrentAbilities[i]))
		end
	end
end

function HeroTab:OnSpawnEnd(ticket)

end

function HeroTab:OnSpawnFailed(ticket)

end

-- In Game HUD Actions
function HeroTab:OnAction(entityId, actionName) 
	local button=CanvasInputRequestBus.Broadcast.GetMouseButton()	
	if actionName == "HeroSelected" then
		UiElementBus.Event.SetIsEnabled(self.Properties.SelectedHero, true)
		UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.SelectedHero, true)
		local parent = UiElementBus.Event.GetParent(entityId)
		local text = UiElementBus.Event.FindChildByName(parent, "Name")
		self:UpdateSelectedHero(UiTextBus.Event.GetText(text))
		if UiElementBus.Event.IsEnabled(self.Properties.LockIn) == false then
			UiElementBus.Event.SetIsEnabled(self.Properties.LockIn, true)
		end		
	end
	
	if actionName == "HeroHovered" then
	
	end
	
	if actionName == "HeroUnHovered" then
	
	end
	
	if actionName == "Exit" then
		Debug.Log("FOUND EXIT FOR SETTINGS MENU")
	end	
	
end

function HeroTab:UpdateSelectedHero(hero)
	self.selectedHero = hero
	self:UpdateAbilities(self.selectedHero)
end

function HeroTab:UpdateAbilities(hero)
	local info = GetUnitInfo(hero)
	
	local childCount = UiElementBus.Event.GetChildren(self.Properties.AbilityPanel)
	for i = 1,#childCount do 
		self.CurrentAbilities[i] = nil
		UiElementBus.Event.DestroyElement(childCount[i])
	end
	
	for i = 1, 20 do
		local ability = tostring("ability" .. tostring(tonumber(i-1)))
		local abilityInfo = info[tostring(ability)]
		if not abilityInfo then
			break
		end
		if abilityInfo then
			self.spawnerAbilityHandler = UiSpawnerNotificationBus.Connect(self, self.Properties.AbilityPanel)
			self.spawnTickets[i] = UiSpawnerBus.Event.Spawn(self.Properties.AbilityPanel)
			self.CurrentAbilities[i] = abilityInfo
		end
	end
end

return HeroTab;