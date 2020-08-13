require "scripts/library/timers"

local itemscript = 
{
	selectedUnit = nil;
	Item = nil;
	OnCoolDown = false;
	OwnButton = nil;
	InventoryPanel = nil;
	Itemindex = nil;
	ToolTip = nil;
	ItemOptions = nil;
	InBackPack = nil;
	ItemTreeArea = nil;
	CurrentInspectionItem = nil;
	Properties = 
	{
		Image = {default = EntityId()},
		ManaText = {default = EntityId()},
		CooldownTextRef = {default = EntityId()},
		CoolddownTimer = {default = EntityId()},
		CooldownPanel = {default = EntityId()},
		KeyBindText = {default = EntityId()},
		CurrentLevelTextColour = {default = Color(255/255, 255/255, 255/255)},	
		StackText = {default = EntityId()},
	}
}

function itemscript:OnActivate()
	UiElementBus.Event.SetIsEnabled(self.Properties.CooldownPanel, false)
	
	self.tickBusHandler = TickBus.Connect(self)
	
	CreateTimer(function() 
		self:Init() 
	end,0.1);
end

function itemscript:Init()
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
		self.ItemOptions = self.ItemOptions or UiCanvasBus.Event.FindElementByName(self.CanvasEntityId, "ItemOptions")
		self.InventoryPanel = self.InventoryPanel or UiCanvasBus.Event.FindElementByName(self.CanvasEntityId, "InventorySlots")
		self.ItemTreeArea = self.ItemTreeArea or UiCanvasBus.Event.FindElementByName(self.CanvasEntityId, "ItemTreeArea")
	end 
	self:InitUnit()
end

function itemscript:InitUnit()
	self.selectedUnit = SelectionRequestBus.Broadcast.GetMainSelectedUnit()
	self:OnMainSelectedUnitChanged(self.selectedUnit)
end

function itemscript:OnMainSelectedUnitChanged(unit)
	self.selectedUnit = unit
end

function itemscript:UpdateKeyInput(key)	
	--Set Action Key For Items
	local keys=key:GetKeys();
	for i=1, #keys do
		
		if i == 2 then
			UiTextBus.Event.SetText(self.Properties.KeyBindText, KeyToString(keys[2]) .. "\n" .. KeyToString(keys[1]))
		else 
			UiTextBus.Event.SetText(self.Properties.KeyBindText, KeyToString(keys[i]))
		end
		
	end	
	-- Setting Item Image
	if self.Item ~= nil then
		if self.Info ~= GetAbilityInfo(self.Item) or self.Info == nil then		
			self.Info = GetAbilityInfo(self.Item);
			if self.Info ~= nil then
				if self.Info["icon-file"] ~= nil then
					local AbilityImage = self.Info["icon-file"]
					local ImageString = tostring(tostring(AbilityImage) .. ".dds")
					UiImageBus.Event.SetSpritePathname(self.Properties.Image, ImageString)
				else
					UiImageBus.Event.SetSpritePathname(self.Properties.Image, "textures/core.dds")
				end
			end
		end
	end	
end

function itemscript:OnTick(deltaTime, timePoint)
	--self.tickBusHandler:Disconnect()
	local Enabled = (UiElementBus.Event.IsEnabled(self.entityId))
	if Enabled == true then

		if self.selectedUnit == SelectionRequestBus.Broadcast.GetMainSelectedUnit() then
			self.ButtonSlot = UiElementBus.Event.GetParent(self.entityId)
			if self.ButtonSlot ~= nil then
				self.Itemindex = UiElementBus.Event.GetIndexOfChildByEntityId(self.InventoryPanel, self.ButtonSlot)
			end
		end
		
		--Check Item Slot For Backpack Slots
		if self.Itemindex ~= nil and self.selectedUnit == SelectionRequestBus.Broadcast.GetMainSelectedUnit()then
			self.Item = UnitAbilityRequestBus.Event.GetAbilityInSlot(self.selectedUnit,Slot(Slot.Inventory,self.Itemindex))
			if self.Itemindex == 6 or self.Itemindex == 7 or self.Itemindex == 8 then
				self.InBackPack = true
			else
				self.InBackPack = false
			end
			self:UpdateItemStats(self.Item)
			local SelfOwner = UiElementBus.Event.FindChildByName(self.entityId, "ItemHotKey")
			if self.Properties.KeyBindText == UiElementBus.Event.FindChildByName(SelfOwner, "ItemKeyText") then
				key = InputMapperRequestBus.Broadcast.GetSlotHotkey(Slot(Slot.Inventory,self.Itemindex))
				if key ~= nil then
					self:UpdateKeyInput(key)
				end
			end
		end		
	end
	
	-- Check and Update Cooldown
	if(self.OnCoolDown) then
		if self.selectedUnit == SelectionRequestBus.Broadcast.GetMainSelectedUnit() then
			UiElementBus.Event.SetIsEnabled(self.Properties.CooldownPanel, true)
			self:UpdateCooldown()			
		else
			UiElementBus.Event.SetIsEnabled(self.Properties.CooldownPanel, false)
		end
	else
		UiElementBus.Event.SetIsEnabled(self.Properties.CooldownPanel, false)	
	end
	
	-- Check item And Update Cooldown and Stats
	if self.Item ~= nil then
		if self.selectedUnit == SelectionRequestBus.Broadcast.GetMainSelectedUnit() then
			self.abilityHandler = AbilityNotificationBus.Connect(self, self.Item)
		end
		if VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.Item,"cooldown_timer")) > 0 then
			self.OnCoolDown = true
		end
		local castingBehavior = Ability.GetCastingBehavior({entityId = self.Item})
		if castingBehavior ~= nil then
			if castingBehavior:Contains(CastingBehavior.PASSIVE) then
				--Debug.Log(tostring("ABILITY IS PASSIVE LOOK HERE !"))
				UiElementBus.Event.SetIsEnabled(self.Properties.KeyBindText, false)
				UiElementBus.Event.SetIsEnabled(self.Properties.ManaText, false)
			else
				if self.InBackPack == false then
					UiElementBus.Event.SetIsEnabled(self.Properties.KeyBindText, true)
					UiElementBus.Event.SetIsEnabled(self.Properties.ManaText, true)
				else
					UiElementBus.Event.SetIsEnabled(self.Properties.KeyBindText, false)
					UiElementBus.Event.SetIsEnabled(self.Properties.ManaText, false)
				end
			end
		end
	end	 
end

function itemscript:OnDeactivate()
	self.tickBusHandler:Disconnect()
	self.selectionHandler:Disconnect()
end

function itemscript:OnSpellStart()
	self.OnCoolDown = true
end

-- Itemscript Actions
function itemscript:OnAction(entityId, actionName) 
	if entityId == self.OwnButton then
		local button=CanvasInputRequestBus.Broadcast.GetMouseButton()
		local pressed = false
		
		if actionName == "ItemPressed" then
			if button == MouseButton.Right then	
				UiElementBus.Event.SetIsEnabled(self.ItemOptions, true)
				local Pos = UiTransformBus.Event.GetCanvasPosition(entityId)
				UiTransformBus.Event.SetCanvasPosition(self.ItemOptions,Pos)
			else	
				if pressed == false then
					pressed = true
				end			
			end
		end
		
		if actionName == "SellItem" then
			Debug.Log("LOOK HERE FOR SELL ON ITEM")
			local shopId = GameManagerRequestBus.Broadcast.GetNearestShop(SelectionRequestBus.Broadcast.GetMainSelectedUnit())
			ShopRequestBus.Event.SellItem(shopId,SelectionRequestBus.Broadcast.GetMainSelectedUnit(),self.Item)
			UiElementBus.Event.SetIsEnabled(self.ItemOptions, false)
		end
		
		if actionName == "ShowItemsInShop" then
			Debug.Log("LOOK HERE FOR SHOP ITEM IN SHOP")
			local item = self.CurrentItem
			self:ShowInShop(item)
			UiElementBus.Event.SetIsEnabled(self.ItemOptions, false)
		end
		
		if actionName == "DisasembleItem" then
			Debug.Log("LOOK HERE FOR DISASEMBLE ON ITEM")
			UiElementBus.Event.SetIsEnabled(self.ItemOptions, false)
		end
		
		if actionName == "ItemHovering" then
			local ItemButton = UiElementBus.Event.GetParent(entityId)
			local ItemButtonSlot = UiElementBus.Event.GetParent(ItemButton)
			local Itemindex = UiElementBus.Event.GetIndexOfChildByEntityId(self.InventoryPanel, ItemButtonSlot)
			
			self.CurrentItem = UnitAbilityRequestBus.Event.GetAbilityInSlot(self.selectedUnit,Slot(Slot.Inventory,Itemindex))		
			self:UpdateToolTip(self.CurrentItem)
		end
		
		if actionName == "ItemDropped" then
			local ButtonSlot = UiElementBus.Event.GetParent(self.entityId)
			if ButtonSlot ~= nil then
				local index= UiElementBus.Event.GetIndexOfChildByEntityId(self.InventoryPanel, entityId)
				if index ~= nil then
					local SelfOwner = UiElementBus.Event.FindChildByName(self.entityId, "ItemHotKey")
					self.Properties.KeyBindText = UiElementBus.Event.FindChildByName(SelfOwner, "ItemKeyText")
				end
			end
			ButtonSlot = nil
			index = nil
			SelfOwner = nil
		end
		pressed = false
	end
end

-- UI Show In Shop Function
function itemscript:ShowInShop(item)
	local itemName = AbilityRequestBus.Event.GetAbilityName(item)	
	local Child1 = UiElementBus.Event.GetChild(self.ItemTreeArea,0)
	local Child2 = UiElementBus.Event.GetChild(self.ItemTreeArea,2)
	local inspection = UiElementBus.Event.GetChild(self.ItemTreeArea,1)
	
	UiElementBus.Event.SetIsEnabled(UiElementBus.Event.GetChild(inspection,0),true)
	local childCount = UiElementBus.Event.GetChildren(Child1)
	local ParentCount = UiElementBus.Event.GetChildren(Child2)
	
	if childCount then
		for i=1, #childCount do
			UiElementBus.Event.DestroyElement(childCount[i])
		end
	end	
	
	if ParentCount then
		for i=1, #ParentCount do
			UiElementBus.Event.DestroyElement(ParentCount[i])
		end
	end	
	
	self.CurrentInspectionItem = itemName	
	Debug.Log(tostring(UiElementBus.Event.GetName(self.ItemTreeArea)))
	
	self.Info=GetItemInfo(self.CurrentInspectionItem);
	
	local childText = UiElementBus.Event.FindDescendantByName(inspection, "Text")
	UiTextBus.Event.SetText(childText, tostring(self.CurrentInspectionItem))

	local Image = UiElementBus.Event.FindDescendantByName(inspection, "Image")
	local TextureRef = tostring(tostring(UiImageBus.Event.GetSpritePathname(self.Properties.Image)))
	UiImageBus.Event.SetSpritePathname(Image, TextureRef)
	
	if self.Info.required_for ~= nil then
		self.spawnerHandler = UiSpawnerNotificationBus.Connect(self, Child1)
		for k=1, #self.Info.required_for do
			if tostring(GetKvValue(self.Info.required_for[k])) ~= nil then
				self.spawnTicketsRequired[k] = UiSpawnerBus.Event.Spawn(Child1)	
			else	
				return
			end	
		end
	end
	
	if self.Info.dependencies ~= nil then
		self.spawnerHandler = UiSpawnerNotificationBus.Connect(self, Child2)
		for j=1, #self.Info.dependencies do
			if tostring(GetKvValue(self.Info.dependencies[j])) ~= nil then
				self.spawnTicketsDependencies[j] = UiSpawnerBus.Event.Spawn(Child2)
			else	
				return
			end						
		end
	end
end

-- Item Inspection Spawn
function itemscript:OnEntitySpawned(ticket, id)
	for j = 1, #self.Info.required_for do
		if ticket == self.spawnTicketsRequired[j] then
			UiElementBus.Event.SetIsEnabled(id, true)
			local childText = UiElementBus.Event.FindDescendantByName(id, "Text")
			UiTextBus.Event.SetText(childText, tostring(GetKvValue(self.Info.required_for[j],j)))
			UiElementBus.Event.SetIsEnabled(childText, false)
			
			local ItemInfoRequired = GetItemInfo(UiTextBus.Event.GetText(childText))

			local childText2 = UiElementBus.Event.FindDescendantByName(id, "Text2")	
			UiElementBus.Event.SetIsEnabled(childText2, false)				
			
			local Image = UiElementBus.Event.FindDescendantByName(id, "Image")
			if ItemInfoRequired ~= nil then
				local Ref = GetKvValue(ItemInfoRequired["icon-file"],1)
				local TextureRef = tostring(tostring(Ref) .. ".dds")
				UiImageBus.Event.SetSpritePathname(Image, TextureRef)
			end				
		end
	end
	
	for i =1,#self.Info.dependencies do
		if ticket == self.spawnTicketsDependencies[i] then
			UiElementBus.Event.SetIsEnabled(id, true)
			local childText = UiElementBus.Event.FindDescendantByName(id, "Text")
			UiTextBus.Event.SetText(childText, tostring(GetKvValue(self.Info.dependencies[i],i)))
			UiElementBus.Event.SetIsEnabled(childText, false)	
			
			local ItemInfoDependent = GetItemInfo(UiTextBus.Event.GetText(childText))

			local childText2 = UiElementBus.Event.FindDescendantByName(id, "Text2")	
			UiElementBus.Event.SetIsEnabled(childText2, false)	

			local Image = UiElementBus.Event.FindDescendantByName(id, "Image")
			if ItemInfoDependent ~= nil then
				local Ref = GetKvValue(ItemInfoDependent["icon-file"],1)
				local TextureRef = tostring(tostring(Ref) .. ".dds")
				UiImageBus.Event.SetSpritePathname(Image, TextureRef)
			end
		end
	end
end

-- Item Inspection Spawn End
function itemscript:OnSpawnEnd(ticket)
	for i=1, #self.Info.dependencies do
		if ticket == self.spawnTicketsDependencies[i] then
			self.spawnTicketsDependencies[i] = nil
		end
	end
	for i=1, #self.Info.required_for do
		if ticket ==  self.spawnTicketsRequired[i] then
			self.spawnTicketsRequired[i] = nil
		end
	end
end

-- Item Inspection Spawn Failed
function itemscript:OnSpawnFailed(ticket)
	for i=1, #self.Info.dependencies do
		if ticket == self.spawnTicketsDependencies[i] then
			self.spawnerHandler:Disconnect()
			self.spawnTicketsDependencies[i] = nil
		end
	end
	for i=1, #self.Info.required_for do
		if ticket ==  self.spawnTicketsRequired[i] then
			self.spawnerHandler:Disconnect()
			self.spawnTicketsRequired[i] = nil
		end
	end
end

--Updates Item Stats like Mana
function itemscript:UpdateItemStats(UpdatedItem)	
	if UpdatedItem ~= nil then
		local vector=AbilityRequestBus.Event.GetCosts(UpdatedItem);
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
	-- TODO see if stacked, if so update
end

-- Item Update Tool Tip
function itemscript:UpdateToolTip(item)
	local ItemPanel = UiElementBus.Event.FindDescendantByName(self.ToolTip, "Item")
	local AbilityPanel = UiElementBus.Event.FindDescendantByName(self.ToolTip, "Ability")
	local ModifierPanel = UiElementBus.Event.FindDescendantByName(self.ToolTip, "Modifier")
	UiElementBus.Event.SetIsEnabled(ItemPanel, true)
	UiElementBus.Event.SetIsEnabled(AbilityPanel, false)
	UiElementBus.Event.SetIsEnabled(ModifierPanel, false)

	local AbilityName = AbilityRequestBus.Event.GetAbilityName(item)
	
	local StatArea = UiElementBus.Event.FindDescendantByName(ItemPanel, "ItemStats")
	
	local Image = UiElementBus.Event.FindDescendantByName(ItemPanel, "ItemImage")
	local cooldown_reduction = VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.selectedUnit, "cooldown_reduction"))
	
	local CoolDownArea = UiElementBus.Event.FindDescendantByName(ItemPanel, "CoolDownPanel")
	local ManaPanel = UiElementBus.Event.FindDescendantByName(ItemPanel, "ManaPanel")
	
	local CoolDownTextPanel = UiElementBus.Event.FindDescendantByName(ItemPanel, "CoolDownTextPanel")
	local ManaTextPanel = UiElementBus.Event.FindDescendantByName(ItemPanel, "ManaTextPanel")	
	local GoldPanel = UiElementBus.Event.FindDescendantByName(ItemPanel, "ItemCost")
	
	local info=GetAbilityInfo(item);
	
	local NamePanel = UiElementBus.Event.FindDescendantByName(ItemPanel, "ItemName")
	local DescriptionPanel = UiElementBus.Event.FindDescendantByName(ItemPanel, "ItemDescription")
	
	--UiTextBus.Event.SetText(NamePanel, info.name)
	local file_name = Ability.GetType({entityId = item})
	localizeText =  LocalizationRequestBus.Broadcast.LocalizeString("@ui_item_" .. tostring(file_name))
	UiTextBus.Event.SetText(NamePanel, localizeText)
	descriptionLocalizeText = LocalizationRequestBus.Broadcast.LocalizeString("@ui_item_" .. tostring(file_name) .. "_description")
	UiTextBus.Event.SetText(DescriptionPanel, descriptionLocalizeText)
	
	if info["icon-file"] ~= nil then
		local AbilityImage = info["icon-file"]
		local ImageString = tostring(tostring(AbilityImage) .. ".dds")
		UiImageBus.Event.SetSpritePathname(Image, ImageString)
	else
		UiImageBus.Event.SetSpritePathname(Image, "textures/core.dds")
	end
	
	local MaxLevel = Ability.GetMaxLevel({entityId = item})
    for i=1,MaxLevel do 
		local CoolDownText = UiElementBus.Event.GetChild(CoolDownTextPanel,i-1)
		local ManaText = UiElementBus.Event.GetChild(ManaTextPanel,i-1)		
		local priceText = UiElementBus.Event.FindChildByName(GoldPanel, "Text")
		local priceImage = UiElementBus.Event.FindChildByName(GoldPanel, "Image")
		
		-- Update Tool Tip Cooldown
		if info.cooldown ~= nil then
			UiElementBus.Event.SetIsEnabled(CoolDownArea, true)
			if GetKvValue(info.cooldown,i) ~= nil then	
				if i == 1 then			
					UiElementBus.Event.SetIsEnabled(CoolDownText, true)
					if cooldown_reduction > 0 then
						UiTextBus.Event.SetText(CoolDownText, GetKvValue(self.Info.cooldown,i) * (1-cooldown_reduction))
					else
						UiTextBus.Event.SetText(CoolDownText, GetKvValue(self.Info.cooldown,i))
					end
					if i == Ability.GetLevel({entityId = item}) then
						UiTextBus.Event.SetColor(CoolDownText, self.Properties.CurrentLevelTextColour)
					else
						UiTextBus.Event.SetColor(CoolDownText, self.Properties.NonCurrentLevelTextColour)
					end
				elseif i >= 2 and GetKvValue(info.cooldown,i) ~= GetKvValue(info.cooldown,1) then
					UiElementBus.Event.SetIsEnabled(CoolDownText, true)
					UiTextBus.Event.SetText(CoolDownText, GetKvValue(info.cooldown,i))
					if i == Ability.GetLevel({entityId = item}) then
						UiTextBus.Event.SetColor(CoolDownText, self.Properties.CurrentLevelTextColour)
					else
						UiTextBus.Event.SetColor(CoolDownText, self.Properties.NonCurrentLevelTextColour)
					end
				elseif i >= 2 and GetKvValue(info.cooldown,i) == GetKvValue(info.cooldown,1) then
					UiElementBus.Event.SetIsEnabled(CoolDownText, false)
				end
			else
				UiElementBus.Event.SetIsEnabled(CoolDownText, false)
			end
		else
			UiElementBus.Event.SetIsEnabled(CoolDownArea, false)	
		end
		
		-- Update Tool Tip Mana Cost
		if info.costs ~= nil then
			UiElementBus.Event.SetIsEnabled(ManaPanel, true)
			if info.costs.mana ~= nil then
				if GetKvValue(info.costs.mana,i) ~= nil then
					if i == 1 then
						UiElementBus.Event.SetIsEnabled(ManaText, true)
						UiTextBus.Event.SetText(ManaText, GetKvValue(info.costs.mana,i))
						if i == Ability.GetLevel({entityId = item}) then
							UiTextBus.Event.SetColor(ManaText, self.Properties.CurrentLevelTextColour)
						else
							UiTextBus.Event.SetColor(ManaText, self.Properties.NonCurrentLevelTextColour)
						end
					elseif i >= 2 and GetKvValue(info.costs.mana,i) ~= GetKvValue(info.costs.mana,1) then
						UiElementBus.Event.SetIsEnabled(ManaText, true)
						UiTextBus.Event.SetText(ManaText, GetKvValue(info.costs.mana,i))
						if i == Ability.GetLevel({entityId = item}) then
							UiTextBus.Event.SetColor(ManaText, self.Properties.CurrentLevelTextColour)
						else
							UiTextBus.Event.SetColor(ManaText, self.Properties.NonCurrentLevelTextColour)
						end
					elseif i >= 2 and GetKvValue(info.costs.mana,i) == GetKvValue(info.costs.mana,1) then
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
		
		-- Update Tool Tip Price
		if info.price ~= nil then
			if i == 1 then
				if info.price.gold ~= nil then
					if GetKvValue(info.price.gold,1) ~= nil then
						UiTextBus.Event.SetText(priceText, GetKvValue(info.price.gold,1))
					end	
				end
				if info.price.image ~= nil then
					if GetKvValue(info.price.image,1) ~= nil then
					local Ref = GetKvValue(info.price.image,1)
					local TextureRef = tostring(tostring(Ref) .. ".dds")
					UiImageBus.Event.SetSpritePathname(priceImage, TextureRef)
					end	
				end
			end
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
					UiTextBus.Event.SetText(name,tostring(k))
					
					local json = self.Info["special-values"][k]
					if GetKvValue(tostring(json),i) ~= nil then
						if i <= 1 then
							UiElementBus.Event.SetIsEnabled(StatText, true)
							UiTextBus.Event.SetText(StatText, GetKvValue(tostring(json),i))
							--if i == Ability.GetLevel({entityId = ability}) then
								UiTextBus.Event.SetColor(StatText, self.Properties.CurrentLevelTextColour)
							--else
								--UiTextBus.Event.SetColor(StatText, self.Properties.NonCurrentLevelTextColour)
							--end
						elseif i >= 2 and GetKvValue(tostring(json),i) ~= GetKvValue(tostring(json),1) then
							UiElementBus.Event.SetIsEnabled(StatText, true)
							UiTextBus.Event.SetText(StatText, GetKvValue(tostring(json),i))
							--if i == Ability.GetLevel({entityId = ability}) then
								UiTextBus.Event.SetColor(StatText, self.Properties.CurrentLevelTextColour)
							--else
								UiTextBus.Event.SetColor(StatText, self.Properties.NonCurrentLevelTextColour)
							--end
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

-- Item Update Cooldown
function itemscript:UpdateCooldown()
	if self.OwnButton == UiElementBus.Event.FindChildByName(self.entityId, "Button") and self.selectedUnit == SelectionRequestBus.Broadcast.GetMainSelectedUnit() then --and Ability.GetLevel({entityId = self.CurrentAbility}) >= 1 then 
		if self.OnCoolDown == true then
			local coolDownText = VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.Item,"cooldown_timer"))
			local coolDownMaxText = VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.Item,"cooldown"))
			if coolDownText >= tonumber("2.0") then
				UiTextBus.Event.SetText(self.Properties.CooldownTextRef, tostring(tonumber(math.floor(coolDownText))))
			elseif coolDownText < tonumber("2.0") and coolDownText >= tonumber("0.0") then
				UiTextBus.Event.SetText(self.Properties.CooldownTextRef, tostring(tonumber(string.format("%" .. 0.1 .. "f",coolDownText))))
			end
			UiImageBus.Event.SetFillAmount(self.Properties.CoolddownTimer,coolDownText / coolDownMaxText)
			if  UiElementBus.Event.IsEnabled(self.Properties.CooldownPanel) and VariableManagerRequestBus.Broadcast.GetValue(VariableId(self.Item,"cooldown_timer")) <= 0.01 then
				self.OnCoolDown = false
				UiElementBus.Event.SetIsEnabled(self.Properties.CooldownPanel, false)
			end
		end
	end
end

return itemscript;