require "scripts/library/timers"

local InGameShop = 
{
	CanvasEntityId = nil;
	AdvancedSettingsOpen = false;
	CurrentPanelOpen = nil;
	selectedUnit = nil;
	SearchResultsItems = {default = {nil,nil,nil,nil,nil,nil,nil,nil,nil,nil}};
	BoolAdvancedSettings = {default = {nil,nil,nil,nil,nil,nil,nil,nil,nil,nil}};
	ToolTip = nil;
	Properties =
	{
		ShopCanvas = {default = EntityId()},
		ShopSearchResultsArea = {default = EntityId()},
		ShopSearchResults = {default = EntityId()},
		ClearSearchResults = {default = EntityId()},
		SearchParam = {default = EntityId()},
		ItemCatagoryPanel = {default = EntityId()},
		QuickBuy = {default = EntityId()},
		AdvancedSettingsPanel = {default = EntityId()},
		CurrentLevelTextColour = {default = Color(255/255, 255/255, 255/255)},
		NonMatchingColour = {default = Color(150/255, 150/255, 150/255)},
		ShopPanels = 
		{
			BaseItems = {default = EntityId()},
			PanelText = {default = EntityId()},
		}
	},
}

function InGameShop:Init()	
	self.selectionHandler = SelectionNotificationBus.Connect(self)
	
	UiElementBus.Event.SetIsEnabled(self.Properties.ShopCanvas, false)
	UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.ShopCanvas, false)
	UiElementBus.Event.SetIsEnabled(self.Properties.ShopSearchResultsArea, false)
	UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.ShopSearchResultsArea, false)
	UiElementBus.Event.SetIsEnabled(self.Properties.ClearSearchResults, false)
	UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.ClearSearchResults, false)
	self.CurrentPanelOpen = self.Properties.ShopPanels.BaseItems
	
	self.CanvasEntityId = UiElementBus.Event.GetCanvas(self.entityId)	
	if( self.CanvasEntityId ) then
		if self.canvasNotificationHandler then
			self.canvasNotificationHandler:Disconnect()
			self.canvasNotificationHandler = nil
		end
		self.canvasNotificationHandler = UiCanvasNotificationBus.Connect(self, self.CanvasEntityId)
		self.ToolTip = self.Tooltip or UiCanvasBus.Event.FindElementByName(self.CanvasEntityId, "AbilityDescriptor")
	end	
end

function InGameShop:OnActivate()
	CreateTimer(function() 
        self:Init() 
    end,0.1);
end

function InGameShop:InitUnit()
	self.selectedUnit = SelectionRequestBus.Broadcast.GetMainSelectedUnit()
	self:OnMainSelectedUnitChanged(self.selectedUnit)
end

function InGameShop:OnDeactivate()
	self.canvasNotificationHandler:Disconnect()
	self.selectionHandler:Disconnect()
end

function InGameShop:OnMainSelectedUnitChanged(unit)
	self.selectedUnit = unit
end

-- In Game Shop Items
function InGameShop:OnAction(entityId, actionName) 
local button=CanvasInputRequestBus.Broadcast.GetMouseButton()
	if actionName == "SearchUpdated" then
		-- update shop search items
		CanvasInputRequestBus.Broadcast.ConsumeEvent()
		if UiElementBus.Event.IsEnabled(self.Properties.ShopSearchResultsArea) == false then
			self:OpenSearchResults()
		elseif UiElementBus.Event.IsEnabled(self.Properties.ShopSearchResultsArea) == true then
			self:SearchParamsChanged()
		end
	end
	
	if actionName == "ClearSearch" then
		-- Clear Search Results
		self:CloseSearchResults()
	end
	
	if actionName == "BasicItems" then
		-- open basic item shop
		UiTextBus.Event.SetText(self.Properties.ShopPanels.PanelText, "Base")
		self.CurrentPanelOpen = self.Properties.ShopPanels.BaseItems
	end
	
	if actionName == "OptionSelected" then
		-- open core X shop
		local UpgradeCoreShop = UiElementBus.Event.GetParent(entityId)
		local Upgradeindex = UiElementBus.Event.GetIndexOfChildByEntityId(UpgradeCoreShop, entityId)
		self:OpenCoreXPage(Upgradeindex)
	end
	
	if actionName == "OpenAdvancedShopSettings" then
		-- open advanced shop settings
		self:OpenAdvancedSettings(self.AdvancedSettingsOpen)
	end
	
	if actionName == "ClearQuickBuy" then
		-- clear quick buy items
		self:ClearQuickBuy()
	end
	
	-- Search Item pressed
	if actionName == "ShopItemSearchPressed" then
		if button == MouseButton.Right then	
		local slot = UiElementBus.Event.GetParent(entityId)
		local index = UiElementBus.Event.GetIndexOfChildByEntityId(self.Properties.ShopSearchResults, slot)
			if index ~= nil then
				if self.BuyingItem == false then
					self.BuyingItem = true
					if index == UiElementBus.Event.GetIndexOfChildByEntityId(self.Properties.ShopSearchResults, slot) then
						self:BuyItem(index)
						CanvasInputRequestBus.Broadcast.ConsumeEvent()
						return
					end	
				end
			end
		end
		
		if button == MouseButton.Left then
			local slot = UiElementBus.Event.GetParent(entityId)
			local index = UiElementBus.Event.GetIndexOfChildByEntityId(self.Properties.ShopSearchResults, slot)
			if index ~= nil then
				self:UpdateItemView(index)
				CanvasInputRequestBus.Broadcast.ConsumeEvent()
			end
		end		
	end
	
	--Search Item Hover
	if actionName == "SearchItemHover" then
		UiTooltipDisplayBus.Event.SetAutoPosition(self.ToolTip,false)
		local shopId=GameManagerRequestBus.Broadcast.GetNearestShop(self.selectedUnit);
		local vector = ShopRequestBus.Event.GetItemList(shopId)
		
		if( not vector ) then 		
			return;
		end		
		
		local Slot = UiElementBus.Event.GetParent(entityId)
		local index = UiElementBus.Event.GetIndexOfChildByEntityId(self.Properties.ShopSearchResults, Slot)
		local Test = self.SearchResultsItems[index]
		Debug.Log(tostring(Test))				
		self:UpdateToolTip(Test)
	end
	
	if actionName == "SearchItemUnHover" then
		UiTooltipDisplayBus.Event.SetAutoPosition(self.ToolTip,true)
	end
	
	-- Updating Advanced Paramaters
	if actionName == "ParamOptionOn" then
		-- turn on param search and options
		local Button = UiElementBus.Event.GetParent(entityId)
		local Slot = UiElementBus.Event.GetParent(Button)
		local index = UiElementBus.Event.GetIndexOfChildByEntityId(self.Properties.AdvancedSettingsPanel, Slot)		
		local text =  UiElementBus.Event.FindDescendantByName(Slot, "Text")
		
		self.BoolAdvancedSettings[index] = UiTextBus.Event.GetText(text)
	end
	
	if actionName == "ParamOptionOff" then
		-- turn on param search and options
		local Button = UiElementBus.Event.GetParent(entityId)
		local Slot = UiElementBus.Event.GetParent(Button)
		local index = UiElementBus.Event.GetIndexOfChildByEntityId(self.Properties.AdvancedSettingsPanel, ButtonSlot)
		self.BoolAdvancedSettings[index] = nil
	end
end

function InGameShop:UpdatePanelShown()
	UiTextBus.Event.SetText(self.Properties.ShopPanels.PanelText, tostring("Base"))
end

-- Update the shop with the advanced setting set
function InGameShop:UpdateShopWithAdvancedSettings()
	local shopId=GameManagerRequestBus.Broadcast.GetNearestShop(self.selectedUnit);
	local vector = ShopRequestBus.Event.GetItemList(shopId)
	if( not vector ) then 		
		return;
	end
	
	for i=1, #vector do		
		local Item = vector[i].id
		local info = GetItemInfo(Item)				
		for j=0, 9 do			
			if self.BoolAdvancedSettings[j] ~= nil then		
				if self.BoolAdvancedSettings[j] ~= info.searchparamaters then
					local Item = UiElementBus.Event.GetChild(CurrentPanelOpen,i)
					local image = UiElementBus.Event.FindDescendantByName(Item, "Image")
					UiImageBus.Event.SetColor(image, self.Properties.NonMatchingColour)
				end
			end
		end
	end
end	

function InGameShop:OpenAdvancedSettings(IsSettingsOpen)
	if IsSettingsOpen == false then
		-- Open Shop
		UiElementBus.Event.SetIsEnabled(self.Properties.AdvancedSettingsPanel, true)
		UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.AdvancedSettingsPanel, true)
		self.AdvancedSettingsOpen = true
	elseif IsSettingsOpen == true then
		-- Close Shop
		UiElementBus.Event.SetIsEnabled(self.Properties.AdvancedSettingsPanel, false)
		UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.AdvancedSettingsPanel, false)
		self.AdvancedSettingsOpen = false
	end
end

function InGameShop:OpenSearchResults()
	-- open search results and search for first entry
	UiElementBus.Event.SetIsEnabled(self.Properties.ShopSearchResultsArea, true)
	UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.ShopSearchResultsArea, true)
	UiElementBus.Event.SetIsEnabled(self.Properties.ClearSearchResults, true)
	UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.ClearSearchResults, true)
	UiElementBus.Event.SetIsEnabled(self.Properties.ItemCatagoryPanel, false)
	UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.ItemCatagoryPanel, false)	
	self:UpdateSearchResults(UiTextInputBus.Event.GetText(self.Properties.SearchParam))
end

-- Update Search Results
function InGameShop:UpdateSearchResults(currentInput)
	local shopId = GameManagerRequestBus.Broadcast.GetNearestShop(self.selectedUnit);
	local vector = ShopRequestBus.Event.GetItemList(shopId)
	
	if currentInput ~= "" then
		for i=1, #vector do
			local Item = vector[i].id
			local info = GetItemInfo(Item)
			if info.name ~= nil then
				if UiElementBus.Event.IsEnabled(self.Properties.ShopSearchResultsArea) == false	then	
					UiElementBus.Event.SetIsEnabled(self.Properties.ShopSearchResultsArea, true)
					UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.ShopSearchResultsArea, true)
				end
				for i=0, 9 do	
					 if string.find(string.upper(info.name), currentInput) or string.find(string.lower(info.name), currentInput) then							
						local searchSlot = UiElementBus.Event.GetChild(self.Properties.ShopSearchResults, i)
						if UiElementBus.Event.IsEnabled(searchSlot) == false then 
							local image = UiElementBus.Event.FindDescendantByName(searchSlot, "Image")
							local text = UiElementBus.Event.FindDescendantByName(searchSlot, "Name")
							local cost = UiElementBus.Event.FindDescendantByName(searchSlot, "Cost")
							
							for i=0, 9 do
								if self.SearchResultsItems[i] == Item then
									return
								end
							end
							UiTextBus.Event.SetText(text, info.name)
							
							UiElementBus.Event.SetIsEnabled(searchSlot, true)
							UiInteractableBus.Event.SetIsHandlingEvents(searchSlot, true)
							
							self.SearchResultsItems[i] = Item
							break
						end
					else
						local searchSlot = UiElementBus.Event.GetChild(self.Properties.ShopSearchResults, i)
						UiElementBus.Event.SetIsEnabled(searchSlot, false)
						UiInteractableBus.Event.SetIsHandlingEvents(searchSlot, false)
						self.SearchResultsItems[i] = nil
					end			
						
				end				
			end
		end
	else
		-- Remove Search Results 
		for i=0, 9 do
			local searchSlot = UiElementBus.Event.GetChild(self.Properties.ShopSearchResults, i)
			
			UiElementBus.Event.SetIsEnabled(searchSlot, false)
			UiInteractableBus.Event.SetIsHandlingEvents(searchSlot, false)
			UiElementBus.Event.SetIsEnabled(self.Properties.ShopSearchResultsArea, false)
			UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.ShopSearchResultsArea, false)
			UiElementBus.Event.SetIsEnabled(self.Properties.ClearSearchResults, false)
			UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.ClearSearchResults, false)
		
			self.SearchResultsItems[i] = nil
		end
	end
end

function InGameShop:CloseSearchResults()
	-- close all search results stuff
	UiElementBus.Event.SetIsEnabled(self.Properties.ShopSearchResultsArea, false)
	UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.ShopSearchResultsArea, false)
	UiElementBus.Event.SetIsEnabled(self.Properties.ClearSearchResults, false)
	UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.ClearSearchResults, false)
	UiTextInputBus.Event.SetText(self.Properties.SearchParam, "")
	UiElementBus.Event.SetIsEnabled(self.Properties.ItemCatagoryPanel, true)
	UiInteractableBus.Event.SetIsHandlingEvents(self.Properties.ItemCatagoryPanel, true)
	
	for i=0, 9 do
		local searchSlot = UiElementBus.Event.GetChild(self.Properties.ShopSearchResults, i)
		
		UiElementBus.Event.SetIsEnabled(searchSlot, false)
		UiInteractableBus.Event.SetIsHandlingEvents(searchSlot, false)
		
		self.SearchResultsItems[i] = nil
	end
end

function InGameShop:SearchParamsChanged()
	-- update search results
	self:UpdateSearchResults(UiTextInputBus.Event.GetText(self.Properties.SearchParam))
end

function InGameShop:AdvancedSettingsChanged(index)
	-- update advanced settings
end

function InGameShop:ClearQuickBuy()
	-- clear quick buy
	local Panel = UiElementBus.Event.FindDescendantByName(self.Properties.QuickBuy, "QuickBuyItems")
	
	for i=0, 9 do
		local QuickBuySlot = UiElementBus.Event.GetChild(Panel,i)
		local ItemSlot = UiElementBus.Event.GetChild(QuickBuySlot,0)
		
		if ItemSlot ~= nil then		
			UiElementBus.Event.SetIsEnabled(ItemSlot, false)			
		end
	end
	UnitRequestBus.Event.SetQuickBuyItemList(self.selectedUnit,vector_basic_string_char_char_traits_char());
end

function InGameShop:OpenCoreXPage(Index)
	-- open core X page
	if Index == 0 then
	UiTextBus.Event.SetText(self.Properties.ShopPanels.PanelText, "Upgrades1")
	elseif Index == 1 then
	UiTextBus.Event.SetText(self.Properties.ShopPanels.PanelText, "Upgrades2")
	elseif Index == 2 then
	UiTextBus.Event.SetText(self.Properties.ShopPanels.PanelText, "Upgrades3")
	elseif Index == 3 then	
	UiTextBus.Event.SetText(self.Properties.ShopPanels.PanelText, "Upgrades4")
	elseif Index == 4 then	
	UiTextBus.Event.SetText(self.Properties.ShopPanels.PanelText, "Upgrades5")
	end
end

function InGameShop:UpdateItemView(ItemIndex)
	-- Update Quick Buy Box 
end

function InGameShop:BuyItem(ItemIndex)	
	-- local shopId=GameManagerRequestBus.Broadcast.GetNearestShop(SelectionRequestBus.Broadcast.GetMainSelectedUnit());
	-- if( shopId and  shopId:IsValid()) then
	-- 	Debug.Log("hero2 : in shop range");
		--local vector = ShopRequestBus.Event.GetItemList(shopId)
		self.BuyingItem = false
		UnitRequestBus.Event.NewOrder(SelectionRequestBus.Broadcast.GetMainSelectedUnit(),BuyItemOrder(self.SearchResultsItems[ItemIndex]),false)
	-- 	--ShopRequestBus.Event.BuyItem(shopId,SelectionRequestBus.Broadcast.GetMainSelectedUnit(),self.SearchResultsItems[ItemIndex]);
	-- 	return
	-- end
end

-- Update Tool Tip
function InGameShop:UpdateToolTip(item)
	local CoolDownPanel = UiElementBus.Event.FindDescendantByName(self.ToolTip, "CoolDownPanel")
	local ManaPanel = UiElementBus.Event.FindDescendantByName(self.ToolTip, "ManaPanel")
	
	local CoolDownTextPanel = UiElementBus.Event.FindDescendantByName(self.ToolTip, "CoolDownTextPanel")
	local ManaTextPanel = UiElementBus.Event.FindDescendantByName(self.ToolTip, "ManaTextPanel")	
	
	-- Get Item Info
	local info=GetItemInfo(item);
    for i=1,1 do 
		local CoolDownText = UiElementBus.Event.GetChild(CoolDownTextPanel,i-1)
		local ManaText = UiElementBus.Event.GetChild(ManaTextPanel,i-1)
		if info.name ~= nil then
			local NamePanel = UiElementBus.Event.FindDescendantByName(self.ToolTip, "AbilityName")
			UiTextBus.Event.SetText(NamePanel, info.name)
		end
		
		-- Update Item Tool Tip Cooldown
		if info.cooldown ~= nil then
			UiElementBus.Event.SetIsEnabled(CoolDownPanel, true)
			if GetKvValue(info.cooldown,i) ~= nil then				
				UiElementBus.Event.SetIsEnabled(CoolDownText, true)
				UiTextBus.Event.SetText(CoolDownText, GetKvValue(info.cooldown,i))
				UiTextBus.Event.SetColor(CoolDownText, self.Properties.CurrentLevelTextColour)
			else
				UiElementBus.Event.SetIsEnabled(CoolDownText, false)
			end
		else
			UiElementBus.Event.SetIsEnabled(CoolDownPanel, false)	
		end
		
		-- Update Item Tool Tip Cost
		if info.costs ~= nil then
			UiElementBus.Event.SetIsEnabled(ManaPanel, true)
			if info.costs.mana ~= nil then
				if GetKvValue(info.costs.mana,i) ~= nil then
					UiElementBus.Event.SetIsEnabled(ManaText, true)
					UiTextBus.Event.SetText(ManaText, GetKvValue(info.costs.mana,i))
					UiTextBus.Event.SetColor(ManaText, self.Properties.CurrentLevelTextColour)
				else
					UiElementBus.Event.SetIsEnabled(ManaText, false)
				end	
			else
				UiElementBus.Event.SetIsEnabled(ManaPanel, false)	
			end	
		else
			UiElementBus.Event.SetIsEnabled(ManaPanel, false)	
		end
    end	
end

return InGameShop;