local AdvancedHUD = 
{
	ElementSelected = nil;
	anchorOffsetChange = nil;
	Properties =
	{
		ScaleX = {default = EntityId()},
		ScaleY = {default = EntityId()},
		
		Rotation = {default = EntityId()},
		
		PivotX = {default = EntityId()},
		PivotY = {default = EntityId()},
		
		OffsetX = {default = EntityId()},
		OffsetY = {default = EntityId()},
		OffsetWidth = {default = EntityId()},
		OffsetHeight = {default = EntityId()},
		
		AnchorLeft = {default = EntityId()},
		AnchorTop = {default = EntityId()},
		AnchorRight = {default = EntityId()},
		AnchorBottom = {default = EntityId()},

		offset1 = {default = EntityId()},
		offset2 = {default = EntityId()},
		offset3 = {default = EntityId()},
		offset4 = {default = EntityId()},
	},
}
function AdvancedHUD:OnTick(deltaTime, timePoint)
    --self.tickBusHandler:Disconnect()

    -- Get the canvas entityId
    -- This is done after the OnActivate when the canvas is fully initialized
    self.canvasEntityId = UiElementBus.Event.GetCanvas(self.entityId)
	
	if( self.canvasEntityId ) then
		if self.canvasNotificationHandler then
			self.canvasNotificationHandler:Disconnect()
			self.canvasNotificationHandler = nil
		end
	    self.canvasNotificationHandler = UiCanvasNotificationBus.Connect(self, self.canvasEntityId)			
	end
	
end
function AdvancedHUD:OnActivate()
	self.tickBusHandler = TickBus.Connect(self);
	self.canvasNotificationHandler = nil
	
	self:LoadSettings()	
end

function AdvancedHUD:OnDeactivate()
	self.tickBusHandler:Disconnect()
	self.canvasNotificationHandler:Disconnect()
end

function AdvancedHUD:OnAction(entityId, actionName) 
	
	
	if actionName == "ElementPressed" then
		self.ElementSelected = UiElementBus.Event.GetParent(entityId)
		self:UpdateElementTransform(self.ElementSelected)
	end
	
	if actionName == "PivotOptionPressed" then
		local parent = UiElementBus.Event.GetParent(entityId)
		for i = 1, 9 do 
			if UiElementBus.Event.GetChild(parent, i-1) == entityId then
				UiImageBus.Event.SetColor(entityId, Color(0/255, 255/255, 255/255))
				self:UpdatePivotValues(i-1)
			else
				UiImageBus.Event.SetColor(UiElementBus.Event.GetChild(parent, i-1), Color(255/255, 255/255, 255/255))
			end
		end
	end
	
	if actionName == "AnchorOptionPressed" then
		local parent = UiElementBus.Event.GetParent(entityId)
		for i = 1, 16 do 
			if UiElementBus.Event.GetChild(parent, i-1) == entityId then
				UiImageBus.Event.SetColor(entityId, Color(0/255, 255/255, 255/255))
				self:UpdateAnchorValues(i-1)
			else
				UiImageBus.Event.SetColor(UiElementBus.Event.GetChild(parent, i-1), Color(255/255, 255/255, 255/255))
			end
		end
	end	
	
	if actionName == "UpdateRotation" then
		local Text = UiTextBus.Event.GetText(self.Properties.Rotation)
		if self.ElementSelected ~= nil then
			UiTransformBus.Event.SetZRotation(self.ElementSelected, tonumber(Text))
			
		end
		UiTextBus.Event.SetText(self.Properties.Rotation, tostring(Text))	
	end
	
	if actionName == "RotationEntered" then
		local Text = UiTextBus.Event.GetText(self.Properties.Rotation)
		UiTextBus.Event.SetText(self.Properties.Rotation, tostring(Text))
		--UiTextBus.Event.SetText(self.Properties.Rotation, tostring(Text) .. " degrees")	
	end
	
	if actionName == "ScaleXChanged" then
		local TextX = tonumber(UiTextBus.Event.GetText(self.Properties.ScaleX))
		local TextY = tonumber(UiTextBus.Event.GetText(self.Properties.ScaleY))
		if self.ElementSelected ~= nil then
			UiTransformBus.Event.SetScale(self.ElementSelected, Vector2(TextX,TextY))
		end
	end
	
	if actionName == "ScaleYChanged" then
		local TextX = tonumber(UiTextBus.Event.GetText(self.Properties.ScaleY))
		local TextY = tonumber(UiTextBus.Event.GetText(self.Properties.ScaleX))
		if self.ElementSelected ~= nil then
			UiTransformBus.Event.SetScale(self.ElementSelected, Vector2(TextX,TextY))
		end
	end
	
	if actionName == "PivotXChanged" then
		local TextX = tonumber(UiTextBus.Event.GetText(self.Properties.PivotX))
		local TextY = tonumber(UiTextBus.Event.GetText(self.Properties.PivotY))
		if self.ElementSelected ~= nil then
			UiTransformBus.Event.SetPivot(self.ElementSelected, Vector2(TextX,TextY))
		end
	end
	
	if actionName == "PivotYChanged" then
		local TextX = tonumber(UiTextBus.Event.GetText(self.Properties.PivotX))
		local TextY = tonumber(UiTextBus.Event.GetText(self.Properties.PivotY))
		if self.ElementSelected ~= nil then
			UiTransformBus.Event.SetPivot(self.ElementSelected, Vector2(TextX,TextY))
		end
	end
	
	if actionName == "AnchorLeftChanged" then
		local left = tonumber(UiTextBus.Event.GetText(self.Properties.AnchorLeft))
		local top = tonumber(UiTextBus.Event.GetText(self.Properties.AnchorTop))	
		local right = tonumber(UiTextBus.Event.GetText(self.Properties.AnchorRight))
		local bottom = tonumber(UiTextBus.Event.GetText(self.Properties.AnchorBottom))
		
		if self.ElementSelected ~= nil then
			UiTransform2dBus.Event.SetAnchors(self.ElementSelected, UiAnchors(left,top,right,bottom), false, false)
		end
	end
	
	if actionName == "AnchorTopChanged" then
		local left = tonumber(UiTextBus.Event.GetText(self.Properties.AnchorLeft))
		local top = tonumber(UiTextBus.Event.GetText(self.Properties.AnchorTop))	
		local right = tonumber(UiTextBus.Event.GetText(self.Properties.AnchorRight))
		local bottom = tonumber(UiTextBus.Event.GetText(self.Properties.AnchorBottom))
		
		if self.ElementSelected ~= nil then
			UiTransform2dBus.Event.SetAnchors(self.ElementSelected, UiAnchors(left,top,right,bottom), false, false)
		end
	end
	
	if actionName == "AnchorRightChanged" then
		local left = tonumber(UiTextBus.Event.GetText(self.Properties.AnchorLeft))
		local top = tonumber(UiTextBus.Event.GetText(self.Properties.AnchorTop))	
		local right = tonumber(UiTextBus.Event.GetText(self.Properties.AnchorRight))
		local bottom = tonumber(UiTextBus.Event.GetText(self.Properties.AnchorBottom))
		
		if self.ElementSelected ~= nil then
			UiTransform2dBus.Event.SetAnchors(self.ElementSelected, UiAnchors(left,top,right,bottom), false, false)
		end
	end
	
	if actionName == "AnchorBottomChanged" then
		local left = tonumber(UiTextBus.Event.GetText(self.Properties.AnchorLeft))
		local top = tonumber(UiTextBus.Event.GetText(self.Properties.AnchorTop))	
		local right = tonumber(UiTextBus.Event.GetText(self.Properties.AnchorRight))
		local bottom = tonumber(UiTextBus.Event.GetText(self.Properties.AnchorBottom))
		
		if self.ElementSelected ~= nil then
			UiTransform2dBus.Event.SetAnchors(self.ElementSelected, UiAnchors(left,top,right,bottom), false, false)
		end
	end
	
	if actionName == "OffsetLeftChanged" then
		local left = tonumber(UiTextBus.Event.GetText(self.Properties.OffsetX))
		local top = tonumber(UiTextBus.Event.GetText(self.Properties.OffsetY))	
		local right = tonumber(UiTextBus.Event.GetText(self.Properties.OffsetWidth))
		local bottom = tonumber(UiTextBus.Event.GetText(self.Properties.OffsetHeight))
		
		if self.ElementSelected ~= nil then
			UiTransform2dBus.Event.SetOffsets(self.ElementSelected, UiOffsets(left,top,right,bottom))
		end
	end
	
	if actionName == "OffsetTopChanged" then
		local left = tonumber(UiTextBus.Event.GetText(self.Properties.OffsetX))
		local top = tonumber(UiTextBus.Event.GetText(self.Properties.OffsetY))	
		local right = tonumber(UiTextBus.Event.GetText(self.Properties.OffsetWidth))
		local bottom = tonumber(UiTextBus.Event.GetText(self.Properties.OffsetHeight))
		
		if self.ElementSelected ~= nil then
			UiTransform2dBus.Event.SetOffsets(self.ElementSelected, UiOffsets(left,top,right,bottom))
		end
	end
	
	if actionName == "OffsetRightChanged" then
		local left = tonumber(UiTextBus.Event.GetText(self.Properties.OffsetX))
		local top = tonumber(UiTextBus.Event.GetText(self.Properties.OffsetY))	
		local right = tonumber(UiTextBus.Event.GetText(self.Properties.OffsetWidth))
		local bottom = tonumber(UiTextBus.Event.GetText(self.Properties.OffsetHeight))
		
		if self.ElementSelected ~= nil then
			UiTransform2dBus.Event.SetOffsets(self.ElementSelected, UiOffsets(left,top,right,bottom))
		end
	end
	
	if actionName == "OffsetBottomChanged" then
		local left = tonumber(UiTextBus.Event.GetText(self.Properties.OffsetX))
		local top = tonumber(UiTextBus.Event.GetText(self.Properties.OffsetY))	
		local right = tonumber(UiTextBus.Event.GetText(self.Properties.OffsetWidth))
		local bottom = tonumber(UiTextBus.Event.GetText(self.Properties.OffsetHeight))
		
		if self.ElementSelected ~= nil then
			UiTransform2dBus.Event.SetOffsets(self.ElementSelected, UiOffsets(left,top,right,bottom))
		end
	end
end

function AdvancedHUD:UpdatePivotValues(i)
local x = nil
local y = nil	
	if i == 0 then
		x = 0 
		y = 0		
	elseif i == 1 then
		x = 0.5
		y = 0
	elseif i == 2 then
		x = 1.0 
		y = 0
	elseif i == 3 then
		x = 0 
		y = 0.5
	elseif i == 4 then
		x = 0.5 
		y = 0.5
	elseif i == 5 then
		x = 1.0 
		y = 0.5
	elseif i == 6 then
		x = 0 
		y = 1.0
	elseif i == 7 then
		x = 0.5 
		y = 1.0
	elseif i == 8 then
		x = 1.0 
		y = 1.0
	end
	UiTextBus.Event.SetText(self.Properties.PivotX, x)
	UiTextBus.Event.SetText(self.Properties.PivotY, y)
	if self.ElementSelected ~= nil then
		UiTransform2dBus.Event.SetPivotAndAdjustOffsets(self.ElementSelected, Vector2(x,y))
		self:UpdateElementTransform(self.ElementSelected)
	end
end

function AdvancedHUD:UpdateAnchorValues(i)
local left = nil
local top = nil	
local right = nil
local bottom = nil
local PivotIndex = nil
local Offsets = UiTransform2dBus.Event.GetOffsets(self.ElementSelected)
	if i == 0 then
		left = 0.0
		top = 0.0
		right = 0.0
		bottom = 0.0
		PivotIndex = 0
		self.anchorOffsetChange = "Normal"				
	elseif i == 1 then
		left = 0.5
		top = 0.0
		right = 0.5
		bottom = 0.0
		PivotIndex = 1
		self.anchorOffsetChange = "Normal"		
	elseif i == 2 then
		left = 1.0
		top = 0.0
		right = 1.0
		bottom = 0.0
		PivotIndex = 2
		self.anchorOffsetChange = "Normal"
	elseif i == 3 then
		left = 0.0
		top = 0.0
		right = 1.0
		bottom = 0.0
		PivotIndex = 1
		self.anchorOffsetChange = "LeftAndRight"
	elseif i == 4 then
		left = 0.0
		top = 0.5
		right = 0.0
		bottom = 0.5
		PivotIndex = 3
		self.anchorOffsetChange = "Normal"
	elseif i == 5 then
		left = 0.5
		top = 0.5
		right = 0.5
		bottom = 0.5
		PivotIndex = 4
		self.anchorOffsetChange = "Normal"
	elseif i == 6 then
		left = 1.0
		top = 0.5
		right = 1.0
		bottom = 0.5
		PivotIndex = 5
		self.anchorOffsetChange = "Normal"
	elseif i == 7 then
		left = 0.0
		top = 0.5
		right = 1.0
		bottom = 0.5
		PivotIndex = 4
		self.anchorOffsetChange = "LeftAndRight"
	elseif i == 8 then
		left = 0.0
		top = 1.0
		right = 0.0
		bottom = 1.0	
		PivotIndex = 6
		self.anchorOffsetChange = "Normal"
	elseif i == 9 then
		left = 0.5
		top = 1.0
		right = 0.5
		bottom = 1.0	
		PivotIndex = 7
		self.anchorOffsetChange = "Normal"
	elseif i == 10 then
		left = 1.0
		top = 1.0
		right = 1.0
		bottom = 1.0	
		PivotIndex = 8
		self.anchorOffsetChange = "Normal"
	elseif i == 11 then
		left = 0.0
		top = 1.0
		right = 1.0
		bottom = 1.0
		PivotIndex = 7	
		self.anchorOffsetChange = "LeftAndRight"		
	elseif i == 12 then
		left = 0.0
		top = 0.0
		right = 0.0
		bottom = 1.0
		PivotIndex = 3
		self.anchorOffsetChange = "TopAndBottom"
	elseif i == 13 then
		left = 0.5
		top = 0.0
		right = 0.5
		bottom = 1.0
		PivotIndex = 4		
		self.anchorOffsetChange = "TopAndBottom"
	elseif i == 14 then
		left = 1.0
		top = 0.0
		right = 1.0
		bottom = 1.0	
		PivotIndex = 5
		self.anchorOffsetChange = "TopAndBottom"
	elseif i == 15 then
		left = 0.0
		top = 0.0
		right = 1.0
		bottom = 1.0
		PivotIndex = 4
		self.anchorOffsetChange = "All"
	end
	UiTextBus.Event.SetText(self.Properties.AnchorLeft, left)
	UiTextBus.Event.SetText(self.Properties.AnchorTop, top)
	UiTextBus.Event.SetText(self.Properties.AnchorRight, right)
	UiTextBus.Event.SetText(self.Properties.AnchorBottom, bottom)
	if self.ElementSelected ~= nil then				
		if self.anchorOffsetChange == "Normal" then
			UiTextBus.Event.SetText(self.Properties.offset1, "X Pos")
			UiTextBus.Event.SetText(self.Properties.offset2, "Y Pos")
			UiTextBus.Event.SetText(self.Properties.offset3, "Width")
			UiTextBus.Event.SetText(self.Properties.offset4, "Height")	
		elseif self.anchorOffsetChange == "LeftAndRight" then
			UiTextBus.Event.SetText(self.Properties.offset1, "Left")
			UiTextBus.Event.SetText(self.Properties.offset2, "Y Pos")
			UiTextBus.Event.SetText(self.Properties.offset3, "Right")
			UiTextBus.Event.SetText(self.Properties.offset4, "Height")
		elseif self.anchorOffsetChange == "TopAndBottom" then
			UiTextBus.Event.SetText(self.Properties.offset1, "X Pos")
			UiTextBus.Event.SetText(self.Properties.offset2, "Top")
			UiTextBus.Event.SetText(self.Properties.offset3, "Width")
			UiTextBus.Event.SetText(self.Properties.offset4, "Bottom")
		elseif self.anchorOffsetChange == "All" then
			UiTextBus.Event.SetText(self.Properties.offset1, "Left")
			UiTextBus.Event.SetText(self.Properties.offset2, "Top")
			UiTextBus.Event.SetText(self.Properties.offset3, "Right")
			UiTextBus.Event.SetText(self.Properties.offset4, "Bottom")
		end
		UiTransform2dBus.Event.SetOffsets(self.ElementSelected,Offsets)
		self:UpdatePivotValues(PivotIndex)
		UiTransform2dBus.Event.SetAnchors(self.ElementSelected, UiAnchors(left,top,right,bottom), false, false)		
		
		self:UpdateElementTransform(self.ElementSelected)
	end
	self.anchorOffsetChange = nil
end

function AdvancedHUD:UpdateElementTransform(Element)	
	local Anchors = UiTransform2dBus.Event.GetAnchors(Element)
	local Offsets = UiTransform2dBus.Event.GetOffsets(Element)
	local Pivot = UiTransformBus.Event.GetPivot(Element)
	local Rotation = UiTransformBus.Event.GetZRotation(Element)
	local Scale = UiTransformBus.Event.GetScale(Element)
	
	UiTextBus.Event.SetText(self.Properties.ScaleX, tostring(tonumber(Scale.x * 2)))
	UiTextBus.Event.SetText(self.Properties.ScaleY, tostring(tonumber(Scale.y * 2)))
	
	UiTextBus.Event.SetText(self.Properties.Rotation, tostring(Rotation))
	
	UiTextBus.Event.SetText(self.Properties.PivotX, tostring(Pivot.x))
	UiTextBus.Event.SetText(self.Properties.PivotY, tostring(Pivot.y))
	
	--UiTextBus.Event.SetText(self.Properties.OffsetX, tostring(tonumber(math.abs(Offsets.left))))
	--UiTextBus.Event.SetText(self.Properties.OffsetY, tostring(tonumber(math.abs(Offsets.bottom))))
	UiTextBus.Event.SetText(self.Properties.OffsetWidth, tostring(tonumber(math.abs(UiTransform2dBus.Event.GetLocalWidth(Element)))))
	UiTextBus.Event.SetText(self.Properties.OffsetHeight, tostring(tonumber(math.abs(UiTransform2dBus.Event.GetLocalHeight(Element)))))
	
	UiTextBus.Event.SetText(self.Properties.AnchorLeft, tostring(tonumber(math.abs(Anchors.left))))
	UiTextBus.Event.SetText(self.Properties.AnchorTop, tostring(tonumber(math.abs(Anchors.top))))
	UiTextBus.Event.SetText(self.Properties.AnchorRight, tostring(tonumber(math.abs(Anchors.right))))
	UiTextBus.Event.SetText(self.Properties.AnchorBottom, tostring(tonumber(math.abs(Anchors.bottom))))
	
	local pos = UiTransformBus.Event.GetLocalPosition(Element)
	pos.y = pos.y - pos.y
	pos.x = pos.x - pos.x
	UiTransformBus.Event.SetLocalPosition(Element, Vector2(pos.x, pos.y))
	
	UiTransform2dBus.Event.SetPivotAndAdjustOffsets(Element, Pivot)	
	
	Debug.Log(tostring(UiElementBus.Event.GetName(Element)))
	if tostring(UiElementBus.Event.GetName(Element)) == "MiniMapPanel" then
		SettingsRequestBus.Broadcast.SetSettingValue("AdvancedMiniMapAnchor",tostring(Anchor), "AdvancedHUD")
		SettingsRequestBus.Broadcast.SetSettingValue("AdvancedMiniMapPivot",tostring(Pivot), "AdvancedHUD")
		SettingsRequestBus.Broadcast.SetSettingValue("AdvancedMiniMapRotation",tostring(Rotation), "AdvancedHUD")
		SettingsRequestBus.Broadcast.SetSettingValue("AdvancedMiniMapScale",tostring(Scale), "AdvancedHUD")
		--SettingsRequestBus.Broadcast.SetSettingValue("AdvancedMiniMapOffset",tostring())
	end
	
	if tostring(UiElementBus.Event.GetName(Element)) == "MainAreaPanel" then
		SettingsRequestBus.Broadcast.SetSettingValue("AdvancedMainAreaAnchor",tostring(Anchor), "AdvancedHUD")
		SettingsRequestBus.Broadcast.SetSettingValue("AdvancedMainAreaPivot",tostring(Pivot), "AdvancedHUD")
		SettingsRequestBus.Broadcast.SetSettingValue("AdvancedMainAreaRotation",tostring(Rotation), "AdvancedHUD")
		SettingsRequestBus.Broadcast.SetSettingValue("AdvancedMainAreaScale",tostring(Scale), "AdvancedHUD")
		--SettingsRequestBus.Broadcast.SetSettingValue("AdvancedMainAreaOffset",tostring())
	end
	
	if tostring(UiElementBus.Event.GetName(Element)) == "Shop/MoneyPanel" then
		SettingsRequestBus.Broadcast.SetSettingValue("AdvancedShopMoneyAnchor",tostring(Anchor), "AdvancedHUD")
		SettingsRequestBus.Broadcast.SetSettingValue("AdvancedShopMoneyPivot",tostring(Pivot), "AdvancedHUD")
		SettingsRequestBus.Broadcast.SetSettingValue("AdvancedShopMoneyRotation",tostring(Rotation), "AdvancedHUD")
		SettingsRequestBus.Broadcast.SetSettingValue("AdvancedShopMoneyScale",tostring(Scale), "AdvancedHUD")
		--SettingsRequestBus.Broadcast.SetSettingValue("AdvancedShopMoneyOffset",tostring())
	end
	
	if tostring(UiElementBus.Event.GetName(Element)) == "ShopPanel" then
		SettingsRequestBus.Broadcast.SetSettingValue("AdvancedShopAnchor",tostring(Anchor), "AdvancedHUD")
		SettingsRequestBus.Broadcast.SetSettingValue("AdvancedShopPivot",tostring(Pivot), "AdvancedHUD")
		SettingsRequestBus.Broadcast.SetSettingValue("AdvancedShopRotation",tostring(Rotation), "AdvancedHUD")
		SettingsRequestBus.Broadcast.SetSettingValue("AdvancedShopScale",tostring(Scale), "AdvancedHUD")
		--SettingsRequestBus.Broadcast.SetSettingValue("AdvancedShopOffset",tostring())
	end
	
	if tostring(UiElementBus.Event.GetName(Element)) == "TopBarPanel" then
		SettingsRequestBus.Broadcast.SetSettingValue("AdvancedTopBarAnchor",tostring(Anchor), "AdvancedHUD")
		SettingsRequestBus.Broadcast.SetSettingValue("AdvancedTopBarPivot",tostring(Pivot), "AdvancedHUD")
		SettingsRequestBus.Broadcast.SetSettingValue("AdvancedTopBarRotation",tostring(Rotation), "AdvancedHUD")
		SettingsRequestBus.Broadcast.SetSettingValue("AdvancedTopBarScale",tostring(Scale), "AdvancedHUD")
		--SettingsRequestBus.Broadcast.SetSettingValue("AdvancedTopBarOffset",tostring())
	end
	
	if tostring(UiElementBus.Event.GetName(Element)) == "ChatSystemPanel" then
		SettingsRequestBus.Broadcast.SetSettingValue("AdvancedChatAnchor",tostring(Anchor), "AdvancedHUD")
		SettingsRequestBus.Broadcast.SetSettingValue("AdvancedChatPivot",tostring(Pivot), "AdvancedHUD")
		SettingsRequestBus.Broadcast.SetSettingValue("AdvancedChatRotation",tostring(Rotation), "AdvancedHUD")
		SettingsRequestBus.Broadcast.SetSettingValue("AdvancedChatScale",tostring(Scale), "AdvancedHUD")
		--SettingsRequestBus.Broadcast.SetSettingValue("AdvancedChatOffset",tostring())
	end
end

function AdvancedHUD:DefaultSettings()
	--Set Default Settings
end

function AdvancedHUD:LoadSettings()
	--Load Current Settings On Load

end

return AdvancedHUD;