require "scripts/library/timers"

local ShopItems =
{
	InitComplete = false;
	BuyingItem = false;
	ToolTip = nil;
	Items = {};
	spawnTicketsDependencies = {};
	spawnTicketsRequired = {};
	CurrentInspectionItem = nil;
	CurrentShopPanelOpen = nil;
	UpdateComplete = false;
	shopId = nil;
	vector = nil;
	Properties = 
	{
		ItemPanel = {default = EntityId()},
		PanelText = {default = EntityId()},
		ItemInspection = 
		{
			ParentPanel = {default = EntityId()},
			InspectionItemSlot = {default = EntityId()},
			ChildItem = {default = EntityId()},
		},		
	},
}

function ShopItems:Init()
    self.selectionHandler = SelectionNotificationBus.Connect(self)

    self.CanvasEntityId = UiElementBus.Event.GetCanvas(self.entityId)
	if( self.CanvasEntityId ) then
		if self.canvasNotificationHandler then
			self.canvasNotificationHandler:Disconnect()
			self.canvasNotificationHandler = nil
		end
	    self.canvasNotificationHandler = UiCanvasNotificationBus.Connect(self, self.CanvasEntityId)
		self.ToolTip = self.Tooltip or UiCanvasBus.Event.FindElementByName(self.CanvasEntityId, "AbilityDescriptor")
	end
	
	--[[if self.CurrentShopPanelOpen ~= nil and self.CurrentShopPanelOpen ~= tostring(UiTextBus.Event.GetText(self.Properties.PanelText)) then
		self.UpdateComplete = false
		if self.UpdateComplete == false then
			self.CurrentShopPanelOpen = tostring(UiTextBus.Event.GetText(self.Properties.PanelText))
			self:UpdatesPanel()
		end
	end]]
end

function ShopItems:OnActivate()	
	
	CreateTimer(function() 
        self:Init() 
    end,0.1);
	CreateTimer(function() 
        self:InitUnit() 
    end,0.1);
end

function ShopItems:InitUnit()
	self.selectedUnit = SelectionRequestBus.Broadcast.GetMainSelectedUnit()
	self:OnMainSelectedUnitChanged(self.selectedUnit)
end

function ShopItems:OnMainSelectedUnitChanged(unit)
	self.selectedUnit = unit
	if self.InitComplete == false then
		self:UpdateShopPanel()
	end
	if self.shopId ~= GameManagerRequestBus.Broadcast.GetNearestShop(unit) then
		self.shopId = GameManagerRequestBus.Broadcast.GetNearestShop(unit)
		self.vector = ShopRequestBus.Event.GetItemList(self.shopId)
		self.UpdateComplete = false
		if self.UpdateComplete == false then
			self:UpdatesPanel()
		end
	end
end

function ShopItems:OnDeactivate()
	self.canvasNotificationHandler:Disconnect()
	self.selectionHandler:Disconnect()
end

-- Update Show Items And Panels
function ShopItems:UpdatesPanel()
	if( not self.vector ) then 		
		return;
	end	
	
	for i=1, 60 do 
		local itemSlot = UiElementBus.Event.GetChild(self.Properties.ItemPanel,i)
		local Slot = UiElementBus.Event.GetChild(itemSlot,0)
		if Slot ~= nil then
			UiElementBus.Event.SetIsEnabled(Slot, false)
		end
	end	
			
	local FreeSlot = 1	
	for i=1, #self.vector do
		self.Info=GetItemInfo(self.vector[i].id);
		local itemSlot = UiElementBus.Event.GetChild(self.Properties.ItemPanel,FreeSlot)
		local Slot = UiElementBus.Event.GetChild(itemSlot,0)
		if self.Info.shoptab ~= nil then
			if self.Info.shoptab == tostring(UiTextBus.Event.GetText(self.Properties.PanelText)) then
				self.Items[i] = self.vector[i].id		
				if Slot ~= nil then
					UiElementBus.Event.SetIsEnabled(Slot, true)
					FreeSlot = FreeSlot + 1
					self:UpdateSlot(i, Slot)
				end
			end
		end		
		local DropTargetShopRef = require "scripts.ui.maingame.droptargetshop"
		DropTargetShopRef.Items[i] = self.vector[i].id

		local ShopItemReference = require "scripts.ui.maingame.shopitem"
		ShopItemReference.ShopItemRef[i] = self.vector[i].id		
	end	
	self.UpdateComplete = true	
end

function ShopItems:UpdateShopPanel()
if( not self.vector ) then 		
		return;
	end	
	self:UpdatesPanel()		
	self.InitComplete = true
end

-- Update Shop Slots
function ShopItems:UpdateSlot(i, Slot)
	local Text = UiElementBus.Event.FindChildByName(Slot, "Text")
	UiTextBus.Event.SetText(Text, tostring(self.Items[i]))
	self.Info=GetItemInfo(self.Items[i]);
	if self.Info.shoptab ~= nil then
		local Text2 = UiElementBus.Event.FindChildByName(Slot, "Text2")
		UiTextBus.Event.SetText(Text2, tostring(GetKvValue(self.Info.shoptab,i)))
	end
	if self.Info["icon-file"] ~= nil then
		local Image = UiElementBus.Event.FindChildByName(Slot, "Image")
		local Ref = GetKvValue(self.Info["icon-file"],1)
		local TextureRef = tostring(tostring(Ref) .. ".dds")
		UiImageBus.Event.SetSpritePathname(Image, TextureRef)		
	end
end

function ShopItems:OnAction(entityId, actionName) 

	CanvasInputRequestBus.Broadcast.ConsumeEvent() -- always consuming when clicking on the ui

	local mouseEvent = CanvasInputRequestBus.Broadcast.GetMouseEventType()
	if mouseEvent ~= MouseEventType.onmouseclick then return end --only consider click events

	local button=CanvasInputRequestBus.Broadcast.GetMouseButton()
	local buttonParent = UiElementBus.Event.GetParent(entityId)
	local text =  UiElementBus.Event.FindChildByName(buttonParent, "Text")
	local Slot = UiElementBus.Event.GetParent(buttonParent)
	
	if actionName == "Consume" then
		CanvasInputRequestBus.Broadcast.ConsumeEvent()
	end

	-- Buy Item
	if actionName == "ShopItemPressed" then
		if button == MouseButton.Right then	
			if text ~= nil then
				if self.BuyingItem == false then
					self:BuyItem(UiTextBus.Event.GetText(text))
					self.BuyingItem = true
					CanvasInputRequestBus.Broadcast.ConsumeEvent()
				end
			end
		
		self.BuyingItem = false
		
		-- Item Selection For Inspection
		else
			if self.CurrentInspectionItem == nil or self.CurrentInspectionItem ~= UiTextBus.Event.GetText(text) then
				self.CurrentInspectionItem = UiTextBus.Event.GetText(text)
				local childCount = UiElementBus.Event.GetChildren(self.Properties.ItemInspection.ChildItem)
				local ParentCount = UiElementBus.Event.GetChildren(self.Properties.ItemInspection.ParentPanel)
				
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
				
				UiElementBus.Event.SetIsEnabled(self.Properties.ItemInspection.InspectionItemSlot,true)
				self.Info=GetItemInfo(self.CurrentInspectionItem);
				
				local childText = UiElementBus.Event.FindDescendantByName(self.Properties.ItemInspection.InspectionItemSlot, "Text")
				UiTextBus.Event.SetText(childText, tostring(self.CurrentInspectionItem))

				local Image = UiElementBus.Event.FindDescendantByName(self.Properties.ItemInspection.InspectionItemSlot, "Image")
				local Ref = GetKvValue(self.Info["icon-file"],1)
				local TextureRef = tostring(tostring(Ref) .. ".dds")
				UiImageBus.Event.SetSpritePathname(Image, TextureRef)
				
				if self.Info.required_for ~= nil then
					self.spawnerHandler = UiSpawnerNotificationBus.Connect(self, self.Properties.ItemInspection.ParentPanel)
					for k=1, #self.Info.required_for do
						if tostring(GetKvValue(self.Info.required_for[k])) ~= nil then
							self.spawnTicketsRequired[k] = UiSpawnerBus.Event.Spawn(self.Properties.ItemInspection.ParentPanel)	
						else	
							return
						end	
					end
				end
				if self.Info.dependencies ~= nil then
					self.spawnerHandler = UiSpawnerNotificationBus.Connect(self, self.Properties.ItemInspection.ChildItem)
					for j=1, #self.Info.dependencies do
						if tostring(GetKvValue(self.Info.dependencies[j])) ~= nil then
							self.spawnTicketsDependencies[j] = UiSpawnerBus.Event.Spawn(self.Properties.ItemInspection.ChildItem)
						else	
							return
						end						
					end
				end
			end
		end		
	end
end

-- Shop Item Inspection Spawn
function ShopItems:OnEntitySpawned(ticket, id)
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

-- Shop Item Inspection Spawn End
function ShopItems:OnSpawnEnd(ticket)
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

-- Shop Item Inspection Spawn Fail
function ShopItems:OnSpawnFailed(ticket)
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

function ShopItems:BuyItem(Item)
	if( self.shopId and  self.shopId:IsValid()) then
		
		UnitRequestBus.Event.NewOrder(SelectionRequestBus.Broadcast.GetMainSelectedUnit(),BuyItemOrder(Item),false)
		--local itemId = ShopRequestBus.Event.BuyItem(self.shopId,self.selectedUnit,Item)

		-- if itemId and itemId:IsValid() then 
		-- 	AudioRequestBus.Broadcast.PlaySound("Play_sfx_buyitem")
		-- end

		CanvasInputRequestBus.Broadcast.ConsumeEvent()
	else
		Debug.Log("hero2 : cannot get the shop")
	end
end

return ShopItems;