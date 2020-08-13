require "scripts/library/timers"

local QuickBuyItems =
{
	InitComplete = false;
	BuyingItem = false;
	ToolTip = nil;
	itemList = nil;
	Properties = 
	{
		QuickBuyPanel = {default = EntityId()},
	},
}

function QuickBuyItems:OnTick(deltaTime, timePoint)
    --self.tickBusHandler:Disconnect()
	self:UpdateSlots()
end

function QuickBuyItems:OnActivate()
	self.tickBusHandler = TickBus.Connect(self);
	
	CreateTimer(function() 
        self:Init() 
    end,0.1);
end

function QuickBuyItems:OnDeactivate()
	self.tickBusHandler:Disconnect()
	self.canvasNotificationHandler:Disconnect()
	self.selectionHandler:Disconnect()
end

function QuickBuyItems:InitUnit()
	self.selectedUnit = SelectionRequestBus.Broadcast.GetMainSelectedUnit()
	self:OnMainSelectedUnitChanged(self.selectedUnit)
end

function QuickBuyItems:OnMainSelectedUnitChanged(unit)
	--Debug.Log("UNIT SELECTED CHANGED TO : " .. tostring(unit))
	self.selectedUnit = unit
end

function QuickBuyItems:Init()
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
end

function QuickBuyItems:UpdateSlots()
	if self.selectedUnit == nil then
		return
	end	
	self.itemList = UnitRequestBus.Event.GetQuickBuyItemList(self.selectedUnit);
	
	if( not self.itemList) then --or self.itemList == UnitRequestBus.Event.GetQuickBuyItemList(self.selectedUnit) then 		
		return;
	end
		
	for i=1, #self.itemList do	
		self.Info=GetItemInfo(self.itemList[i]);
		
		local itemSlot = UiElementBus.Event.GetChild(self.entityId,(i-1))
		local Child = UiElementBus.Event.GetChild(itemSlot,0)
		if (UiElementBus.Event.IsEnabled(Child) == false) then
			UiElementBus.Event.SetIsEnabled(Child, true)
			
			local Text = UiElementBus.Event.FindChildByName(Child, "Text")
			UiTextBus.Event.SetText(Text, tostring(self.itemList[i]))
			if self.Info["icon-file"] ~= nil then
				local Image = UiElementBus.Event.FindChildByName(Child, "QBImage")
				local Ref = GetKvValue(self.Info["icon-file"],1)
				local TextureRef = tostring(tostring(Ref) .. ".dds")
				UiImageBus.Event.SetSpritePathname(Image, TextureRef)		
			else
				local image = UiElementBus.Event.FindDescendantByName(Child, "QBImage")			
				UiImageBus.Event.SetSpritePathname(image, "textures/core.dds")
			end
		end
	end		
	self.InitComplete = true
end

function QuickBuyItems:OnAction(entityId, actionName) 
local button=CanvasInputRequestBus.Broadcast.GetMouseButton()
	if actionName == "QuickBuyItemPressed" then
		if button == MouseButton.Right then	
		local button = UiElementBus.Event.GetParent(entityId)
		local text =  UiElementBus.Event.FindChildByName(button, "Text")
		local Slot = UiElementBus.Event.GetParent(button)
			if text ~= nil then
				if self.BuyingItem == false then
					self:BuyItem(UiTextBus.Event.GetText(text))

					self.BuyingItem = true
					CanvasInputRequestBus.Broadcast.ConsumeEvent()
				end
			end
		end
		self.BuyingItem = false
	end
end

function QuickBuyItems:BuyItem(Item)	
	-- local shopId=GameManagerRequestBus.Broadcast.GetNearestShop(SelectionRequestBus.Broadcast.GetMainSelectedUnit());
	-- if( shopId and  shopId:IsValid()) then		
	-- 	local QuickBuyList = UnitRequestBus.Event.GetQuickBuyItemList(self.selectedUnit)	
		--AudioRequestBus.Broadcast.PlaySound("Play_sfx_buyitem");		
		--ShopRequestBus.Event.BuyItem(shopId,SelectionRequestBus.Broadcast.GetMainSelectedUnit(),Item );
		UnitRequestBus.Event.NewOrder(SelectionRequestBus.Broadcast.GetMainSelectedUnit(),BuyItemOrder(Item),false)
	-- else
	-- 	Debug.Log("hero2 : cannot get the shop");
	-- end
end

return QuickBuyItems;