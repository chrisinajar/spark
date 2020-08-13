require "scripts/library/timers"

local QuickBuyItem =
{
	ToolTip = nil;
	QuickBuyItemsPanel = nil;
	Info = nil;
	Properties = 
	{
		CurrentLevelTextColour = {default = Color(255/255, 255/255, 255/255)},
	},
}

function QuickBuyItem:Init()	
	self.selectionHandler = SelectionNotificationBus.Connect(self)
	self.OwnButton = UiElementBus.Event.FindChildByName(self.entityId, "Button")

    self.CanvasEntityId = UiElementBus.Event.GetCanvas(self.entityId)
	if( self.CanvasEntityId ) then
		if self.canvasNotificationHandler then
			self.canvasNotificationHandler:Disconnect()
			self.canvasNotificationHandler = nil
		end
	    self.canvasNotificationHandler = UiCanvasNotificationBus.Connect(self, self.CanvasEntityId)
		self.ToolTip = self.Tooltip or UiCanvasBus.Event.FindElementByName(self.CanvasEntityId, "AbilityDescriptor")
		self.QuickBuyItemsPanel = self.QuickBuyItemsPanel or UiCanvasBus.Event.FindElementByName(self.CanvasEntityId, "QuickBuyItems")
	end
end

function QuickBuyItem:InitUnit()
	self.selectedUnit = SelectionRequestBus.Broadcast.GetMainSelectedUnit()
	self:OnMainSelectedUnitChanged(self.selectedUnit)
end

function QuickBuyItem:OnActivate()
	CreateTimer(function()
		self:Init()
	end,0.1)
end

function QuickBuyItem:OnMainSelectedUnitChanged(unit)
	self.selectedUnit = unit
end

function QuickBuyItem:OnDeactivate()
	self.canvasNotificationHandler:Disconnect()
	self.selectionHandler:Disconnect()
end

function QuickBuyItem:OnAction(entityId, actionName) 
local button=CanvasInputRequestBus.Broadcast.GetMouseButton()
	if entityId == self.OwnButton then
		if actionName == "QuickBuyItemHovered" then
			if self.selectedUnit==nil then
				self:InitUnit() 
			end
			if self.selectedUnit==nil then
				return
			end
			
			UiTooltipDisplayBus.Event.SetAutoPosition(self.ToolTip,false)
			local QuickBuyList = UnitRequestBus.Event.GetQuickBuyItemList(self.selectedUnit)
			if( not QuickBuyList ) then 		
				return;
			end	
			
			local Button = UiElementBus.Event.GetParent(entityId)
			local Slot = UiElementBus.Event.GetParent(Button)
			local index = UiElementBus.Event.GetIndexOfChildByEntityId(self.QuickBuyItemsPanel, Slot)
					
			self:UpdateToolTip(QuickBuyList[index])
		end
		
		if actionName == "QuickBuyShopItemUnHovered" then
			UiTooltipDisplayBus.Event.SetAutoPosition(self.ToolTip,true)
		end
	end
end

function QuickBuyItem:UpdateToolTip(item)
	local ItemPanel = UiElementBus.Event.FindDescendantByName(self.ToolTip, "Item")
	local AbilityPanel = UiElementBus.Event.FindDescendantByName(self.ToolTip, "Ability")
	local ModifierPanel = UiElementBus.Event.FindDescendantByName(self.ToolTip, "Modifier")
	UiElementBus.Event.SetIsEnabled(ItemPanel, true)
	UiElementBus.Event.SetIsEnabled(AbilityPanel, false)
	UiElementBus.Event.SetIsEnabled(ModifierPanel, false)
	
	local StatArea = UiElementBus.Event.FindDescendantByName(ItemPanel, "ItemStats")

	local NamePanel = UiElementBus.Event.FindDescendantByName(ItemPanel, "ItemName")
	UiTextBus.Event.SetText(NamePanel, item)
	--[[local DescriptionPanel = UiElementBus.Event.FindDescendantByName(AbilityPanel, "ItemDescription")
	
	--UiTextBus.Event.SetText(NamePanel, info.name)
	local file_name = Item.GetType({entityId = item})
	localizeText =  LocalizationRequestBus.Broadcast.LocalizeString("@ui_item_" .. tostring(file_name))
	UiTextBus.Event.SetText(NamePanel, localizeText)
	descriptionLocalizeText = LocalizationRequestBus.Broadcast.LocalizeString("@ui_item_" .. tostring(file_name) .. "_description")
	UiTextBus.Event.SetText(DescriptionPanel, descriptionLocalizeText)]]
	
	local Image = UiElementBus.Event.FindDescendantByName(ItemPanel, "ItemImage")
	
	local CoolDownPanel = UiElementBus.Event.FindDescendantByName(self.ToolTip, "CoolDownPanel")
	local ManaPanel = UiElementBus.Event.FindDescendantByName(self.ToolTip, "ManaPanel")
	
	local CoolDownTextPanel = UiElementBus.Event.FindDescendantByName(self.ToolTip, "CoolDownTextPanel")
	local ManaTextPanel = UiElementBus.Event.FindDescendantByName(self.ToolTip, "ManaTextPanel")
	local GoldPanel = UiElementBus.Event.FindDescendantByName(ItemPanel, "ItemCost")	
	self.Info=GetItemInfo(item);
	
	-- Set Icon
	if self.Info["icon-file"] ~= nil then
		local AbilityImage = self.Info["icon-file"]
		local ImageString = tostring(tostring(AbilityImage) .. ".dds")
		UiImageBus.Event.SetSpritePathname(Image, ImageString)
	else
		UiImageBus.Event.SetSpritePathname(Image, "textures/core.dds")
	end
	
    for i=1,1 do 
		local CoolDownText = UiElementBus.Event.GetChild(CoolDownTextPanel,i-1)
		local ManaText = UiElementBus.Event.GetChild(ManaTextPanel,i-1)		
		local priceText = UiElementBus.Event.FindChildByName(GoldPanel, "Text")
		local priceImage = UiElementBus.Event.FindChildByName(GoldPanel, "Image")
		
		-- Update Quick Buy Item Cooldown
		if self.Info.cooldown ~= nil then
			UiElementBus.Event.SetIsEnabled(CoolDownPanel, true)
			if GetKvValue(self.Info.cooldown,i) ~= nil then				
				UiElementBus.Event.SetIsEnabled(CoolDownText, true)
				UiTextBus.Event.SetText(CoolDownText, GetKvValue(self.Info.cooldown,i))
				UiTextBus.Event.SetColor(CoolDownText, self.Properties.CurrentLevelTextColour)
			else
				UiElementBus.Event.SetIsEnabled(CoolDownText, false)
			end
		else
			UiElementBus.Event.SetIsEnabled(CoolDownPanel, false)	
		end
		
		-- Update Quick Buy Item Cost
		if self.Info.costs ~= nil then
			UiElementBus.Event.SetIsEnabled(ManaPanel, true)
			if self.Info.costs.mana ~= nil then
				if GetKvValue(self.Info.costs.mana,i) ~= nil then
					UiElementBus.Event.SetIsEnabled(ManaText, true)
					UiTextBus.Event.SetText(ManaText, GetKvValue(self.Info.costs.mana,i))
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
		
		-- Update Quick Buy Item Price
		if self.Info.price ~= nil then
			if i == 1 then
				if self.Info.price.gold ~= nil then
					if GetKvValue(self.Info.price.gold,1) ~= nil then
						UiTextBus.Event.SetText(priceText, GetKvValue(self.Info.price.gold,1))
					end	
				end
				if self.Info.price.image ~= nil then
					if GetKvValue(self.Info.price.image,1) ~= nil then
					local Ref = GetKvValue(self.Info.price.image,1)
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


return QuickBuyItem;