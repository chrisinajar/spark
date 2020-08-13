require "scripts.library.ui_events"

local PickScreen = 
{
	SettingCanvas = nil;
	selectedHero = nil;
	spawnTickets = {};
	initComplete = false;
	CurrentAbilities = {};
	Properties =
	{
		SettingsButton = {default = EntityId()},
		BackButton = {default = EntityId()},
		SelectedHero = {default = EntityId()},
		LockIn = {default = EntityId()}, 
		
		Strength = {default = EntityId()},
		Agility = {default = EntityId()},
		Intelligence = {default = EntityId()},
		Will = {default = EntityId()},
		
		AbilityPanel = {default = EntityId()},
	},	
}

function PickScreen:OnTick(deltaTime, timePoint)
    --self.tickBusHandler:Disconnect()

	-- Settings Toggle
	if UiElementBus.Event.IsEnabled(self.Properties.SettingsButton) == true then
		self.SettingsOpen = false
		self.SettingCanvas = nil
	end
end

function PickScreen:OnActivate()
	self.tickBusHandler = TickBus.Connect(self);
	CreateTimer(function() 
        self:Init() 
    end,0.1);
end

function PickScreen:Init()
	self.CanvasEntityId = UiElementBus.Event.GetCanvas(self.entityId)
	if self.CanvasEntityId and not self.canvasNotificationHandler then
	   if self.canvasNotificationHandler then
			self.canvasNotificationHandler:Disconnect()
			self.canvasNotificationHandler = nil
		end		
		self.canvasNotificationHandler = UiCanvasNotificationBus.Connect(self, self.CanvasEntityId)
	    self.ToolTip = self.Tooltip or UiCanvasBus.Event.FindElementByName(self.CanvasEntityId, "AbilityDescriptor")
	end

	self.unitList = InfoRequestBus.Broadcast.GetHeroList()
	
	for i=1, #self.unitList do
		if self.unitList[i] then
			Debug.Log("Getting info for " .. self.unitList[i])
			local info = GetUnitInfo(self.unitList[i])
			local PrimeAttribute = tostring(info["attribute_primary"] or info["primary_attribute"])
			if string.upper(PrimeAttribute) == "STRENGTH" then
				self.spawnerHandler = UiSpawnerNotificationBus.Connect(self, self.Properties.Strength)
				self.spawnTickets[i] = UiSpawnerBus.Event.Spawn(self.Properties.Strength)
			elseif string.upper(PrimeAttribute) == "WILL" then
				self.spawnerHandler = UiSpawnerNotificationBus.Connect(self, self.Properties.Will)
				self.spawnTickets[i] = UiSpawnerBus.Event.Spawn(self.Properties.Will)
			elseif string.upper(PrimeAttribute) == "AGILITY" then
				self.spawnerHandler = UiSpawnerNotificationBus.Connect(self, self.Properties.Agility)
				self.spawnTickets[i] = UiSpawnerBus.Event.Spawn(self.Properties.Agility)
			elseif string.upper(PrimeAttribute) == "INTELLIGENCE" then
				self.spawnerHandler = UiSpawnerNotificationBus.Connect(self, self.Properties.Intelligence)
				self.spawnTickets[i] = UiSpawnerBus.Event.Spawn(self.Properties.Intelligence)
			end
		end
	end	
	self.initComplete = true
end

function PickScreen:OnDeactivate()
	self.tickBusHandler:Disconnect()
	self.tickBusHandler = nil
	self.canvasNotificationHandler:Disconnect()
	self.canvasNotificationHandler = nil	
end

function PickScreen:OnEntitySpawned(ticket, id)
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

function PickScreen:OnSpawnEnd(ticket)

end

function PickScreen:OnSpawnFailed(ticket)

end

-- In Game HUD Actions
function PickScreen:OnAction(entityId, actionName) 
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
	
	if actionName == "RandomHero" then
		local result = math.random(#self.unitList)
		SendUIEvent("SelectHero", tostring(self.unitList[result]));
		UiCanvasManagerBus.Broadcast.UnloadCanvas(self.CanvasEntityId)
	end
	
	if actionName == "LockInHero" then
		SendUIEvent("SelectHero", tostring(self.selectedHero));
		--SendUIEvent("SelectHero", "bard");
		--UiCanvasManagerBus.Broadcast.UnloadCanvas(self.CanvasEntityId)
	end
	
	if actionName == "OpenSettings" then
		if self.SettingsOpen == false then
			self.SettingsOpen = true
			self:OpenSettings()
			UiElementBus.Event.SetIsEnabled(self.Properties.SettingsButton, false)
			UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.SettingsButton, false)
		end
	end
	
	if actionName == "Exit" then
		Debug.Log("FOUND EXIT FOR SETTINGS MENU")
	end	
	
end

function PickScreen:UpdateSelectedHero(hero)
	self.selectedHero = hero
	self:UpdateAbilities(self.selectedHero)
end

function PickScreen:UpdateAbilities(hero)
	local info = GetUnitInfo(hero)
	
	local childCount = UiElementBus.Event.GetChildren(self.Properties.AbilityPanel)
	for i = 1,#childCount do 
		self.CurrentAbilities[i] = nil
		UiElementBus.Event.DestroyElement(childCount[i])
	end
	
	for i = 1, 20 do
		local ability = tostring("ability_" .. tostring(tonumber(i-1)))
		local abilityInfo = info[tostring(ability)]
		if not abilityInfo or abilityInfo == nil or abilityInfo == "" then
			break
		end
		if abilityInfo then
			self.spawnerAbilityHandler = UiSpawnerNotificationBus.Connect(self, self.Properties.AbilityPanel)
			self.spawnTickets[i] = UiSpawnerBus.Event.Spawn(self.Properties.AbilityPanel)
			self.CurrentAbilities[i] = abilityInfo
		end
	end
end

function PickScreen:OpenSettings()
	self.SettingCanvas = UiCanvasManagerBus.Broadcast.LoadCanvas("UI/Canvases/settings.uicanvas")
end

function PickScreen:CloseSettings()
	self:SettingsClosed()
	self.SettingsOpen = false
end

function PickScreen:SettingsClosed()
	UiElementBus.Event.SetIsEnabled(self.Properties.SettingsButton, true)
	UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.SettingsButton, true)
end

function PickScreen:UpdateFromSettings()
	-- Update any changes from settings made here...
end

return PickScreen;