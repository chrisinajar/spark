local SettingUpdates = 
{
	init = false;
	Properties =
	{
		ShopCanvas = {default = EntityId()},
		MiniMap = {default = EntityId()},
		Shop_Money = {default = EntityId()},
		MainArea = {default = EntityId()},
		ToolTip = {default = EntityId()},
		Chat = {default = EntityId()},
		ItemOptions = {default = EntityId()},
		TopBar = {default = EntityId()},
		ScoreBoard = {default = EntityId()},
	},
}

function SettingUpdates:Init()
   	self.settingHandler = SettingsNotificationBus.Connect(self)
   	
	self.CanvasEntityId = UiElementBus.Event.GetCanvas(self.entityId)
	if( self.CanvasEntityId ) then
		if self.canvasNotificationHandler then
			self.canvasNotificationHandler:Disconnect()
			self.canvasNotificationHandler = nil
		end
		self.canvasNotificationHandler = UiCanvasNotificationBus.Connect(self, self.CanvasEntityId)
	end
	
	self:UpdateHUDScale()
end

function SettingUpdates:OnActivate()	
	CreateTimer(function() 
        self:Init() 
    end,0.1);
end

function SettingUpdates:OnDeactivate()
	self.canvasNotificationHandler:Disconnect()
end

function SettingUpdates:OnSettingChanged(key, value)
	self.settingChanged = key
	if key == "minimapLocation" then
		self:UpdateMiniMapFromSettings()
	end
	if key == "HUDScale" then
		self:UpdateHUDScale()
	end
end

function SettingUpdates:UpdateMiniMapFromSettings()
	if self.init == false or self.settingChanged == "minimapLocation"  then
		local minimapLocation = SettingsRequestBus.Broadcast.GetSettingValue("minimapLocation")
		Debug.Log("ANCHORS FOR MINI MAP ARE : " .. tostring(UiTransformBus.Event.GetViewportPosition(self.Properties.MiniMap)))
		local anchors = nil
		local offsets = nil
		if minimapLocation == "right" then
			anchors = UiTransform2dBus.Event.GetAnchors(self.Properties.MiniMap)
			UiTransform2dBus.Event.SetAnchors(self.Properties.MiniMap,anchors(1.0,1.0,1.0,1.0),false,false)
			
			offsets = UiTransform2dBus.Event.GetOffsets(self.Properties.MiniMap)
			offsets.left = offsets.left - 200
			offsets.right = offsets.right - 200
			UiTransform2dBus.Event.SetOffsets(self.Properties.MiniMap, offsets)
			
			anchors = UiTransform2dBus.Event.GetAnchors(self.Properties.Shop_Money)
			UiTransform2dBus.Event.SetAnchors(self.Properties.Shop_Money,anchors(0.0,1.0,0.0,1.0),false,false)
			
			offsets = UiTransform2dBus.Event.GetOffsets(self.Properties.Shop_Money)
			offsets.left = offsets.left + 200
			offsets.right = offsets.right + 200
			UiTransform2dBus.Event.SetOffsets(self.Properties.Shop_Money, offsets)
			
			anchors = UiTransform2dBus.Event.GetAnchors(self.Properties.ShopCanvas)
			UiTransform2dBus.Event.SetAnchors(self.Properties.ShopCanvas,anchors(0.0,0.0,0.0,1.0),false,false)
			
			offsets = UiTransform2dBus.Event.GetOffsets(self.Properties.ShopCanvas)
			offsets.left = offsets.left + 250
			offsets.right = offsets.right + 250
			offsets.top = 50
			offsets.bottom = -85
			UiTransform2dBus.Event.SetOffsets(self.Properties.ShopCanvas, offsets)
			
		else
			anchors = UiTransform2dBus.Event.GetAnchors(self.Properties.MiniMap)
			UiTransform2dBus.Event.SetAnchors(self.Properties.MiniMap,anchors(0.0,1.0,0.0,1.0),false,false)
			
			offsets = UiTransform2dBus.Event.GetOffsets(self.Properties.MiniMap)
			offsets.left = offsets.left + 200
			offsets.right = offsets.right + 200
			UiTransform2dBus.Event.SetOffsets(self.Properties.MiniMap, offsets)
			
			anchors = UiTransform2dBus.Event.GetAnchors(self.Properties.Shop_Money)
			UiTransform2dBus.Event.SetAnchors(self.Properties.Shop_Money,anchors(1.0,1.0,1.0,1.0),false,false)
			
			offsets = UiTransform2dBus.Event.GetOffsets(self.Properties.Shop_Money)
			offsets.left = offsets.left - 200
			offsets.right = offsets.right - 200
			UiTransform2dBus.Event.SetOffsets(self.Properties.Shop_Money, offsets)
			
			anchors = UiTransform2dBus.Event.GetAnchors(self.Properties.ShopCanvas)
			UiTransform2dBus.Event.SetAnchors(self.Properties.ShopCanvas,anchors(1.0,0.0,1.0,1.0),false,false)
			
			offsets = UiTransform2dBus.Event.GetOffsets(self.Properties.ShopCanvas)
			offsets.left = offsets.left - 250
			offsets.right = offsets.right - 250
			offsets.top = 50
			offsets.bottom = -85
			UiTransform2dBus.Event.SetOffsets(self.Properties.ShopCanvas, offsets)
		end
	end
	self.init = true
end

function SettingUpdates:UpdateHUDScale()
	if self.init == false or self.settingChanged == "HUDScale" then
		local HudScale = SettingsRequestBus.Broadcast.GetSettingValue("HUDScale", "interface")
		UiTransformBus.Event.SetScale(self.Properties.ShopCanvas, Vector2(HudScale / 100, HudScale / 100))
		UiTransformBus.Event.SetScale(self.Properties.ShopCanvas, Vector2(HudScale / 100, HudScale / 100))
		UiTransformBus.Event.SetScale(self.Properties.MiniMap, Vector2(HudScale / 100, HudScale / 100))
		UiTransformBus.Event.SetScale(self.Properties.Shop_Money, Vector2(HudScale / 100, HudScale / 100))
		UiTransformBus.Event.SetScale(self.Properties.MainArea, Vector2(HudScale / 100, HudScale / 100))
		UiTransformBus.Event.SetScale(self.Properties.ToolTip, Vector2(HudScale / 100, HudScale / 100))
		UiTransformBus.Event.SetScale(self.Properties.Chat, Vector2(HudScale / 100, HudScale / 100))
		UiTransformBus.Event.SetScale(self.Properties.ItemOptions, Vector2(HudScale / 100, HudScale / 100))
		UiTransformBus.Event.SetScale(self.Properties.TopBar, Vector2(HudScale / 100, HudScale / 100))
		UiTransformBus.Event.SetScale(self.Properties.ScoreBoard, Vector2(HudScale / 100, HudScale / 100))
	end
end

return SettingUpdates