require "scripts.GameUtils"
local DebugRenderer = require "scripts.DebugRenderer";

local MiniMap = 
{
	bottomLeft = Vector2(0,0);
	topRight = Vector2(400, 450);
	Properties =
	{
		MiniMap = {default = EntityId()},
		CameraArea = {default = EntityId()},
	},	
}

function MiniMap:OnTick(deltaTime, timePoint)
	self:UpdateMiniMapArea()
	self:UpdateMiniMapPins()
end

function MiniMap:OnActivate()
	self.tickBusHandler = TickBus.Connect(self);
	
	self.Camera = GameManagerRequestBus.Broadcast.GetEntitiesHavingTag(Crc32("Game_Camera"))
	
	self.MouseInputHandler = MouseInputNotificationBus.Connect(self)

	--self.selectionHandler = SelectionNotificationBus.Connect(self)

	self.navAreaHandler = NavigationAreaNotificationBus.Connect(self);
	
	self:OnMainSelectedUnitChanged(SelectionRequestBus.Broadcast.GetMainSelectedUnit())
end

function MiniMap:OnMainSelectedUnitChanged(unit)
	if unit and unit:IsValid() then
		self:OnMainNavigationAreaChanged(NavigationEntityRequestBus.Event.GetCurrentNavigationArea(unit))
	end
end

function MiniMap:OnMainNavigationAreaChanged(id)
	if self.currentNavigationArea ~= id then
		self.currentNavigationArea = id

		self.bottomLeft = NavigationAreaRequestBus.Event.GetAabbMin(self.currentNavigationArea)
		self.topRight = NavigationAreaRequestBus.Event.GetAabbMax(self.currentNavigationArea)
	end
end

function MiniMap:OnDeactivate()
	self.tickBusHandler:Disconnect()
	if self.MouseInputHandler then
		self.MouseInputHandler:Disconnect()
	end
	--self.selectionHandler:Disconnect()
end

-- In Game HUD Actions
function MiniMap:OnAction(entityId, actionName) 

end

function MiniMap:onmousedown(button)
	self:UpdateSizes()
	local mouse = self:GetMinimapMousePosition()

	if(self:IsInside(mouse)) then
		if button == 0 then	
			self.LeftButtonDown = true
			self.cameraWorldPos = self:MiniMapToWorldSpace(mouse);
			--Debug.Log("MINIMAP ON MOUSE DOWN : " .. tostring(button))
			self:UpdateCameraPos()
		end
		return true
	end
	return false;
end

function MiniMap:onmouseup(button)
	self:UpdateSizes()
	local mouse = self:GetMinimapMousePosition()
	self.LeftButtonDown = false

	if(self:IsInside(mouse)) then
		return true
	end
	
	return false;
end

function MiniMap:onmousemove(x,y)
	if self.LeftButtonDown == true then
		self:UpdateSizes()
		local mouse = self:GetMinimapMousePosition()
		if(self:IsInside(mouse)) then
			self.cameraWorldPos = self:MiniMapToWorldSpace(mouse);
		end
		self:UpdateCameraPos()
	end
end


function MiniMap:UpdateSizes()
	self.MiniMapPosition = UiTransformBus.Event.GetViewportSpaceTopLeft(self.Properties.MiniMap)
	self.MiniMapSize =  UiTransformBus.Event.GetViewportSpaceBottomRight(self.Properties.MiniMap) - self.MiniMapPosition
end

function MiniMap:GetMinimapMousePosition()
	local screenSpace = MouseCursorPositionRequestBus.Broadcast.GetSystemCursorPositionPixelSpace()
	return self:ScreenToMiniMapSpace(screenSpace)
end

function MiniMap:IsInside(minimapSpace)
	return minimapSpace.x>=0 and minimapSpace.x<=1 and minimapSpace.y>=0 and minimapSpace.y<=1
end




function MiniMap:ScreenToMiniMapSpace(screenSpace)
	return (screenSpace - self.MiniMapPosition) / self.MiniMapSize;
end

function MiniMap:MiniMapToScreenSpace(screenSpace)
	return (screenSpace * self.MiniMapSize) + self.MiniMapPosition;
end


function MiniMap:MiniMapToWorldSpace(minimapSpace)
	local worldSpace = Vector3(0,0,0)
	worldSpace.x = self.bottomLeft.x + (self.topRight.x - self.bottomLeft.x) * minimapSpace.x
	worldSpace.y = self.bottomLeft.y + (self.topRight.y - self.bottomLeft.y) * (1 - minimapSpace.y)
	return worldSpace;
end

function MiniMap:WorldToMiniMapSpace(worldSpace)
	local miniMapSpace = Vector2(0,0)
	miniMapSpace.x = (worldSpace.x - self.bottomLeft.x) / (self.topRight.x - self.bottomLeft.x)
	miniMapSpace.y = 1 - (worldSpace.y - self.bottomLeft.y) / (self.topRight.y - self.bottomLeft.y)
	return miniMapSpace;
end

function MiniMap:UpdateCameraPos()
	local position = TransformBus.Event.GetWorldTranslation(self.Camera[1])
	position.x = self.cameraWorldPos.x
	position.y = self.cameraWorldPos.y

	TransformBus.Event.SetWorldTranslation(self.Camera[1], position);
end

function MiniMap:UpdateMiniMapArea()
	local position = TransformBus.Event.GetWorldTranslation(self.Camera[1])
	
	self:UpdateSizes()
	local areaPosition = self:MiniMapToScreenSpace(self:WorldToMiniMapSpace(position))
	UiTransformBus.Event.SetCanvasPosition(self.Properties.CameraArea, areaPosition)
end

function MiniMap:UpdateMiniMapPins()

	local heroes = FilterArray(GetAllUnits(),function (elem)
		return HasTag(elem,"hero")
	end);

	if #heroes >= 1 then
		DebugRenderer:Begin("heroPins",true)
	end

	for i=1,#heroes do
		local u = heroes[i]

		if UnitRequestBus.Event.IsVisible(GetId(u)) then
			local color = GetTeamColor(u:GetTeamId()) or Color(0.5,0.5,0.5,1);
			local position = TransformBus.Event.GetWorldTranslation(GetId(u))
			
			local minimap = self:WorldToMiniMapSpace(position)

			if self:IsInside(minimap) then
				local screen = self:MiniMapToScreenSpace(minimap)
				DebugRenderRequestBus.Broadcast.Draw2DRect(screen.x - 2, screen.y -2, 4,4,color,0.1);
			end
		end
	end

end

return MiniMap;