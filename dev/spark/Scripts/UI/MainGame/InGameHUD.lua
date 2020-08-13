require "scripts.variableHolder"
require "scripts/library/timers"
require "scripts.library.client_events"

local InGameHUD = 
{
	CanvasEntityId = nil;
	Abilities = 0;
	ShopOpen = false;
	selectedUnit = nil;
	SettingsOpen = false;
	SettingCanvas = nil;
	talentsOpen = false;
	spawnTickets = {};
	Offsets = nil;
	modifierAnimRaised = false;
	Properties =
	{
		MainAreaID = {default = EntityId()},
		AbilitiesAreaID = {default = EntityId()},
		ItemsAreaID = {default = EntityId()},
		NumberOfAbilities = {default = 4},
		ShopCanvas = {default = EntityId()},	
		ToolTip = {default = EntityId()},	
		SettingsButton = {default = EntityId()},
		TalentTree = {default = EntityId()},
		TalentPanelA = {default = EntityId()},
		TalentPanelB = {default = EntityId()},
		Chat = {default = EntityId()},
		GoldText = {default = EntityId()},
		MiniMap = {default = EntityId()},
		Shop_Money = {default = EntityId()},
		ScoreBoard = {default = EntityId()},
		TextNotification = {default = EntityId()},
		Modifiers = 
		{
			GoodModifiers = {default = EntityId()},
			BadModifiers = {default = EntityId()},
		},		
	},
	Colors = 
	{
		white = Color(255, 255, 255),
		black = Color(0, 0, 0),
		red = Color(255, 0, 0),
		blue = Color(0, 0, 255),
		yellow = Color(255, 255, 0),
		green = Color(0, 255, 0),
	},
}

function InGameHUD:OnTick(deltaTime, timePoint)
    --self.tickBusHandler:Disconnect()
	if self.selectedUnit == SelectionRequestBus.Broadcast.GetMainSelectedUnit() then
		if self.Modifiers ~= nil then
			for i=1,#self.Modifiers do
				local modifier = VariableHolder({entityId=self.Modifiers[i]})
				if modifier:HasValue("current_stacked_amount") == true and modifier:GetValue("current_stacked_amount") >= 0 then
					self:UpdateModifiers("Good")
				end
			end
		end
		if self.Modifiers ~= nil and VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.selectedUnit, "ability_points")) > 0 then
			if self.modifierAnimRaised == false then
				UiAnimationBus.Event.StartSequence(self.CanvasEntityId, "ModifierRaise")
				self.modifierAnimRaised = true
			end
		elseif VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.selectedUnit, "ability_points")) == 0 then
			if self.modifierAnimRaised == true then
				UiAnimationBus.Event.StartSequence(self.CanvasEntityId, "ModifierLower")
				self.modifierAnimRaised = false
			end
		end
	end
	
	-- Settings Toggle
	if UiElementBus.Event.IsEnabled(self.Properties.SettingsButton) == true then
		self.SettingsOpen = false
		self.SettingCanvas = nil
	end
end

function InGameHUD:OnActivate()
	self.tickBusHandler = TickBus.Connect(self);
	self.AbilityHandler = nil
	self.slotHandler = SlotNotificationBus.Connect(self)
	self.selectionHandler = SelectionNotificationBus.Connect(self)
	ClientEvents:OnActivate()
	CreateTimer(function() 
       self:InitUnit() 
    end,0.1);
	
	local DraggableRef = require "scripts.ui.maingame.draggableelement"
	DraggableRef.DragParent =  UiElementBus.Event.GetCanvas(self.entityId)
	
	self.notificationHandler = ListenToClientEvent("notificationText", partial(self.OnNotificationText, self))
end

function InGameHUD:OnNotificationText(data)
	UiTextBus.Event.SetText(self.Properties.TextNotification, data.text)
	if data.color and self.Colors[data.color] then
		UiTextBus.Event.SetColor(self.Properties.TextNotification, self.Colors[data.color])
	else
		UiTextBus.Event.SetColor(self.Properties.TextNotification, self.Colors.white)
	end
	UiElementBus.Event.SetIsEnabled(self.Properties.TextNotification ,true)
	
	CreateTimer(function ()
	 	self:RemoveNotificationText()
	 end, data.duration)
end

function InGameHUD:RemoveNotificationText()
	UiTextBus.Event.SetText(self.Properties.TextNotification, "")
	UiElementBus.Event.SetIsEnabled(self.Properties.TextNotification ,false)
end

function InGameHUD:InitUnit()	
	self.CanvasEntityId = UiElementBus.Event.GetCanvas(self.entityId)
	if self.CanvasEntityId then
		if self.canvasNotificationHandler then
			self.canvasNotificationHandler:Disconnect()
			self.canvasNotificationHandler = nil
		end
	    self.canvasNotificationHandler = UiCanvasNotificationBus.Connect(self, self.CanvasEntityId)	
	    self.ToolTip = self.Tooltip or UiCanvasBus.Event.FindElementByName(self.CanvasEntityId, "AbilityDescriptor")
		--self:UpdateFromSettings()
	end
	self.selectedUnit = SelectionRequestBus.Broadcast.GetMainSelectedUnit()
	local gold = VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.selectedUnit, "gold"))
	UiTextBus.Event.SetText(self.Properties.GoldText, tostring(tonumber(math.floor(gold))))
	self:OnMainSelectedUnitChanged(self.selectedUnit)
end

-- Update Selected Unit
function InGameHUD:OnMainSelectedUnitChanged(unit)
	Debug.Log("UNIT SELECTED CHANGED TO : " .. tostring(unit))
	self.selectedUnit = unit
	if self.UnitNotificationHandler then
		self.UnitNotificationHandler:Disconnect()
		self.UnitNotificationHandler = nil
	end
	self.UnitNotificationHandler = UnitNotificationBus.Connect(self, self.selectedUnit)
	self.VariableHandler = VariableHolderNotificationBus.Connect(self, self.selectedUnit)
	self.Modifiers = UnitRequestBus.Event.GetModifiers(self.selectedUnit)
	self:UpdateModifiers("Good")
	self:UpdateAbilityCount()
end

function InGameHUD:OnDeactivate()
	self.tickBusHandler:Disconnect()
	self.canvasNotificationHandler:Disconnect()
	self.UnitNotificationHandler:Disconnect()
	self.selectionHandler:Disconnect()
	ClientEvents:OnDeactivate()
end

function InGameHUD:OnSetValue(id, value)
	if id.variableId == "gold" then
		if tostring(value) ~= UiTextBus.Event.GetText(self.Properties.GoldText) then
			UiTextBus.Event.SetText(self.Properties.GoldText, tostring(value))
		end
	elseif VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.selectedUnit, "gold")) ~= UiTextBus.Event.GetText(self.Properties.GoldText) then
		local gold = VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.selectedUnit, "gold"))
		UiTextBus.Event.SetText(self.Properties.GoldText,  tostring(tonumber(math.floor(gold))))
	end
end

-- On Modifier Attached
function InGameHUD:OnModifierAttached(modifierId)
	if self.selectedUnit == SelectionRequestBus.Broadcast.GetMainSelectedUnit() then
		self.Modifiers = UnitRequestBus.Event.GetModifiers(self.selectedUnit)
		self:UpdateModifiers("Good")
	end	
end

-- On Modifier Detached
function InGameHUD:OnModifierDetached(modifierId)
	self.Modifiers = UnitRequestBus.Event.GetModifiers(self.selectedUnit)
	self:UpdateModifiers("Good")
end

-- Update Modifiers
function InGameHUD:UpdateModifiers(side)
	if self.Modifiers ~= nil then
		local FreeSlot = 0
		if #self.Modifiers == 0 then
			for j=0,9 do 
				local ModifierChild = UiElementBus.Event.GetChild(self.Properties.Modifiers.GoodModifiers, j)
				
				if UiElementBus.Event.IsEnabled(ModifierChild) == true then	
					UiElementBus.Event.SetIsEnabled(ModifierChild,false)
				end
				
				local ModifierBadChild = UiElementBus.Event.GetChild(self.Properties.Modifiers.BadModifiers, j)
				if UiElementBus.Event.IsEnabled(ModifierBadChild) == true then	
					UiElementBus.Event.SetIsEnabled(ModifierBadChild,false)
				end
			end
		end
			
		for i=1,#self.Modifiers do
			local ModifierChild = UiElementBus.Event.GetChild(self.Properties.Modifiers.GoodModifiers, FreeSlot)
			local Image = UiElementBus.Event.FindChildByName(ModifierChild, "Image")
			local Text = UiElementBus.Event.FindChildByName(ModifierChild, "Text")
			if self.Modifiers[i] ~= nil then	
				if ModifierRequestBus.Event.IsVisible(self.Modifiers[i]) then
					FreeSlot = FreeSlot + 1
					UiElementBus.Event.SetIsEnabled(ModifierChild,true)			
					local Ref = ModifierRequestBus.Event.GetIconTexture(self.Modifiers[i])
					local TextureRef = tostring(tostring(Ref) .. ".dds")
					UiImageBus.Event.SetSpritePathname(Image, TextureRef)
					
					local modifier = VariableHolder({entityId=self.Modifiers[i]})
					local value =  modifier:GetValue("current_stacked_amount")
					if modifier:HasValue("current_stacked_amount") == true and value > 0 then
						UiElementBus.Event.SetIsEnabled(Text,true)
						UiTextBus.Event.SetText(Text, tostring(value))
					else
						UiElementBus.Event.SetIsEnabled(Text,false)
					end
				else
					UiElementBus.Event.SetIsEnabled(ModifierChild,false)
				end
			else
				UiElementBus.Event.SetIsEnabled(ModifierChild,false)
			end			
		end	
		
		for i=#self.Modifiers, 10 do
			local ModifierChild = UiElementBus.Event.GetChild(self.Properties.Modifiers.GoodModifiers, FreeSlot)
			if UiElementBus.Event.IsEnabled(ModifierChild) == true then		
				UiElementBus.Event.SetIsEnabled(ModifierChild,false)
			end	
		end	
	end
end

-- Updates Ability Count
function InGameHUD:UpdateAbilityCount()
local childCount = UiElementBus.Event.GetChildren(self.Properties.AbilitiesAreaID)
	for i = 1,#childCount do 
		UiElementBus.Event.DestroyElement(childCount[i])
	end
	
	local AbilityCount = 0
	if self.selectedUnit ~= nil then
		local abilities = UnitAbilityRequestBus.Event.GetAbilities(self.selectedUnit)
		if abilities then
			for i = 1,#abilities do
				local abilityId = abilities[i]
				if StaticDataRequestBus.Event.GetValue(abilityId,"IsItem") ~= "true" then
					--todo check also if is not hidden
					AbilityCount = AbilityCount + 1
					self.spawnTickets[i] = UiSpawnerBus.Event.Spawn(self.Properties.AbilitiesAreaID)
				end
			end
			
			if AbilityCount ~= self.Abilities then
				self.Abilities = AbilityCount
				self:FindMainArea()
			end
		end
	end
end

-- Dynamic Set For Ability Area
function InGameHUD:FindMainArea()
self.Offsets = UiTransform2dBus.Event.GetOffsets(self.Properties.MainAreaID)
	if self.Abilities > 6  and self.Abilities <= 10 then
		self.Offsets.left = -340 - (25 * (self.Abilities - 6))
		self.Offsets.right = 340 + (25 * (self.Abilities - 6))
	elseif self.Abilities < 6 and self.Abilities >= 3 then
		self.Offsets.left = -340 + (25 * (6 - self.Abilities))
		self.Offsets.right = 340 - (25 * (6 - self.Abilities))
	else
		self.Offsets.left = -340 + (25 * (6 - 2))
		self.Offsets.right = 340 - (25 * (6 - 2))
	end

	if self.Offsets ~= nil then	
		UiTransform2dBus.Event.SetOffsets(self.Properties.MainAreaID, self.Offsets)
	end
end

function InGameHUD:OnEntitySpawned(ticket, id)

end

function InGameHUD:OnSpawnEnd(ticket)

end

function InGameHUD:OnSpawnFailed(ticket)

end

-- On Slot Pressed
function InGameHUD:OnSlotPressed(slot)	
	if(slot:GetType() == Slot.Chat) then
		self.focus = not self.focus		
		local Child = UiElementBus.Event.GetChild(self.Properties.Chat, 0)
		local Text = UiTextInputBus.Event.GetText(Child)
		if Text == "" or Text == "Tap here to chat..." then
			UiElementBus.Event.SetIsEnabled(self.Properties.Chat, self.focus)
			-- add in auto focus here
		end		
		SparkCameraRequestBus.Broadcast.SetFocus(not self.focus);
	end

	if slot == Slot(Slot.Shop,0) then
		self:OnAction(nil, "Shop")
	end 
end

-- In Game HUD Actions
function InGameHUD:OnAction(entityId, actionName) 
local button=CanvasInputRequestBus.Broadcast.GetMouseButton()
	if button == MouseButton.Right then	
		if actionName == "ConsumeEvent" then
			--Debug.Log("CONSUMING MOUSE EVENT HERE !!")
			CanvasInputRequestBus.Broadcast.ConsumeEvent()
		end
	end
	
	if actionName == "OnHoverModifier" then
		local index = UiElementBus.Event.GetIndexOfChildByEntityId(self.Properties.Modifiers.GoodModifiers, entityId)
		if ModifierRequestBus.Event.IsVisible(self.Modifiers[index+1]) then
			self:UpdateToolTip(self.Modifiers[index+1])
		end
	end
	
	if actionName == "AbilityPressed" then
		-- functionality for detecting slot button pressed, needs to be hooked up correctly 
		local AbilityButton = UiElementBus.Event.GetParent(entityId)
		local ButtonSlot = UiElementBus.Event.GetParent(AbilityButton)
		local index = UiElementBus.Event.GetIndexOfChildByEntityId(self.Properties.AbilitiesAreaID, ButtonSlot)
		
		-- On slot pressed, with dynamic index added
		if Ability.GetLevel({entityId =  UnitAbilityRequestBus.Event.GetAbilityInSlot(self.selectedUnit,Slot(Slot.Ability,index))}) > 0 then
			SlotNotificationBus.Broadcast.OnSlotPressed(Slot(Slot.Ability,index))		
		end

	end
	
	if actionName == "OptionSelected" then
		Debug.Log("OPTION SELECTED WOKRING HERE !")
		local TextComponent = UiDropdownBus.Event.GetTextElement(entityId)
		LocalizationRequestBus.Broadcast.SetLanguage(tostring(UiTextBus.Event.GetText(TextComponent)))
	end
	
	if actionName == "ItemPressed" then
		-- functionality for detecting slot button pressed, needs to be hooked up correctly 
		local ItemButton = UiElementBus.Event.GetParent(entityId)
		local ItemButtonSlot = UiElementBus.Event.GetParent(ItemButton)
		local Itemindex = UiElementBus.Event.GetIndexOfChildByEntityId(self.Properties.ItemsAreaID, ItemButtonSlot)
				
		-- On slot pressed, with dynamic index added		
		SlotNotificationBus.Broadcast.OnSlotPressed(Slot(Slot.Inventory,Itemindex))		
	end
	
	if actionName == "LevelUpAbility" then
		local LevelUpButton = UiElementBus.Event.GetParent(entityId)
		local ButtonSlot = UiElementBus.Event.GetParent(LevelUpButton)
		local index = UiElementBus.Event.GetIndexOfChildByEntityId(self.Properties.AbilitiesAreaID, ButtonSlot)
		self.AbilityToLevel = UnitAbilityRequestBus.Event.GetAbilityInSlot(self.selectedUnit,Slot(Slot.Ability,index))
		self:LevelUpAbility(self.AbilityToLevel, ButtonSlot)
	end
	
	if actionName == "OpenSettings" then
		if self.SettingsOpen == false then
			self.SettingsOpen = true
			self:OpenSettings()
			UiElementBus.Event.SetIsEnabled(self.Properties.SettingsButton, false)
			UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.SettingsButton, false)
			if UiElementBus.Event.IsEnabled(self.Properties.ScoreBoard) then
				UiElementBus.Event.SetIsEnabled(self.Properties.ScoreBoard, false)
				UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.ScoreBoard, false)
			end			
		end
	end
	
	if actionName == "OpenScoreBoard" then
		if UiElementBus.Event.IsEnabled(self.Properties.ScoreBoard) then
			UiElementBus.Event.SetIsEnabled(self.Properties.ScoreBoard, false)
			UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.ScoreBoard, false)
		else
			UiElementBus.Event.SetIsEnabled(self.Properties.ScoreBoard, true)
			UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.ScoreBoard, true)
		end
	end
	
	if actionName == "Exit" then
		Debug.Log("FOUND EXIT FOR SETTINGS MENU")
	end	
	
	-- Open Shop
	if actionName == "Shop" then		
		self:OpenShop(self.ShopOpen)
		if UiElementBus.Event.IsEnabled(self.Properties.ScoreBoard) then
			UiElementBus.Event.SetIsEnabled(self.Properties.ScoreBoard, false)
			UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.ScoreBoard, false)
		end
	end
	
	-- Talents Pressed
	if actionName == "TalentPressed" then
		if self.talentsOpen == true then
			self.talentsOpen = false
			UiElementBus.Event.SetIsEnabled(self.Properties.TalentTree, false)
			local Button = UiElementBus.Event.GetParent(entityId)
			local Slot = UiElementBus.Event.GetParent(Button)
			local TalentNumber = UiElementBus.Event.GetIndexOfChildByEntityId(self.Properties.TalentPanelA, Slot)
			local OppositeIndex = TalentNumber
			if TalentNumber == nil then 
				local TalentNumber = UiElementBus.Event.GetIndexOfChildByEntityId(self.Properties.TalentPanelB, Slot)
				UiInteractableBus.Event.SetIsHandlingEvents(entityId, false)
				local oppositeTalent = UiElementBus.Event.GetChild(self.Properties.TalentPanelA,OppositeIndex)
				UiInteractableBus.Event.SetIsHandlingEvents(oppositeTalent, false)
				
				self:LevelTalent(TalentNumber)			
			else
				UiInteractableBus.Event.SetIsHandlingEvents(entityId, false)
				local oppositeTalent = UiElementBus.Event.GetChild(self.Properties.TalentPanelB,OppositeIndex)
				UiInteractableBus.Event.SetIsHandlingEvents(oppositeTalent, false)
				self:LevelTalent(TalentNumber)
			end			
		end
	end
	
	if actionName == "LevelTalents" then
		self.talentsOpen = true
		UiElementBus.Event.SetIsEnabled(self.Properties.TalentTree, true)		
	end
	
	if actionName == "TalentHover" then
		UiElementBus.Event.SetIsEnabled(self.Properties.TalentTree, true)
	end
	
	if actionName == "TalentUnHover" then
		if self.talentsOpen == false then
			UiElementBus.Event.SetIsEnabled(self.Properties.TalentTree, false)
		end
	end	
end

function InGameHUD:UpdateToolTip(modifier)
	local ItemPanel = UiElementBus.Event.FindDescendantByName(self.ToolTip, "Item")
	local AbilityPanel = UiElementBus.Event.FindDescendantByName(self.ToolTip, "Ability")
	local ModifierPanel = UiElementBus.Event.FindDescendantByName(self.ToolTip, "Modifier")
	UiElementBus.Event.SetIsEnabled(ItemPanel, false)
	UiElementBus.Event.SetIsEnabled(AbilityPanel, false)
	UiElementBus.Event.SetIsEnabled(ModifierPanel, true)
	
	local name = ModifierRequestBus.Event.GetModifierTypeId(modifier);
	
	localizeNameText =  LocalizationRequestBus.Broadcast.LocalizeString("@ui_" .. tostring(name))
	localizeDescriptionText =  LocalizationRequestBus.Broadcast.LocalizeString("@ui_" .. tostring(name) .. "_description")
	
	local NamePanel = UiElementBus.Event.FindDescendantByName(ModifierPanel, "ModifierName")
	local DescriptionPanel = UiElementBus.Event.FindDescendantByName(ModifierPanel, "ModifierDescription")
	UiTextBus.Event.SetText(NamePanel, localizeNameText)
	UiTextBus.Event.SetText(DescriptionPanel, localizeDescriptionText)
end

--Give Player Talent They Selected
function InGameHUD:LevelTalent(index)
	Debug.Log("GIVE THE TALENT SELECTED TO THE PLAYER !")
end

function InGameHUD:OpenSettings()
	Debug.Log("LOADING AND OPENING SETTINGS MENU")
	self.SettingCanvas = UiCanvasManagerBus.Broadcast.LoadCanvas("UI/Canvases/settings.uicanvas")
end

function InGameHUD:CloseSettings()
	InGameHUD:SettingsClosed()
	self.SettingsOpen = false
end

function InGameHUD:SettingsClosed()
	UiElementBus.Event.SetIsEnabled(self.Properties.SettingsButton, true)
	UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.SettingsButton, true)
end

-- Open / Close Shop
function InGameHUD:OpenShop(IsShopOpen)
	if IsShopOpen == false then
		UiElementBus.Event.SetIsEnabled(self.Properties.ShopCanvas, true)
		UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.ShopCanvas, true)
		self.ShopOpen = true
	elseif IsShopOpen == true then
		UiElementBus.Event.SetIsEnabled(self.Properties.ShopCanvas, false)
		UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.ShopCanvas, false)
		self.ShopOpen = false
	end	
end

function InGameHUD:LevelUpAbility (ability, ButtonSlot)
	local ability = Ability({entityId = ability})
	if ability:GetLevel() + 1 <= ability:GetMaxLevel() then
		ability:Upgrade()
		return
	end	
end

return InGameHUD;