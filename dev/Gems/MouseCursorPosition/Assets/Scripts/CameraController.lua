local CameraController = 
{
	Properties = 
	{
		cameraSpeed = { default = 15, description = "How fast the camera moves across the screen based off mouse position", suffix = "m/s" },
		scrollSpeed = { default = 25, description = "How fast camera zooms in and out based off scroll mouse input", suffix = "m/s" },
		edgeWidth = { default = .01, min = .01, max = 1.0, description = "How close in normalized screen size the cursor has to be to the screen border to cause movement" },
	},
	
	scrollDirection = 0
}

function CameraController:OnActivate()
	-- Aim camera at the ground
	TransformBus.Event.SetLocalRotation(self.entityId, Vector3(Math.DegToRad(-90), 0, 0))
	
	-- Connect to the TickBus to have an update loop
	self.tickHandler = TickBus.Connect(self)
	
	-- Show the cursor to help see how its position effects camera movement
	LyShineLua.ShowMouseCursor(true)
	
	-- Create an Id and Connect so we can be notified when the scroll wheel is used
	local mousePositionInputId = InputEventNotificationId("Scroll")

	self.scrollInputListener = InputEventNotificationBus.Connect(self, mousePositionInputId)
end

function CameraController:OnDeactivate()
	
	-- Stop showing the cursor when the script finishes
	LyShineLua.ShowMouseCursor(false)
	
	-- Disconnect upon deactivating to clean up
	if self.tickHandler ~= nil then
		self.tickHandler:Disconnect()
		self.tickHandler = nil
	end
	
	if self.scrollInputListener ~= nil then
		self.scrollInputListener:Disconnect()
		self.scrollInputListener = nil
	end
end

function CameraController:OnPressed(value)
	-- The direction to move based off value of -1 or 1
	self.scrollDirection = -Math.Sign(value)
end

function CameraController:OnTick(deltaTime, timePoint)
	
	-- Call the EBus exposed through the MouseCursorPosition Gem to get the cursor position normalized between (0,0) and (1,1) and change to a range between (-1,-1) and (1,1)
	-- The initial declaration of the EBus can be found at MouseCursorPositionBus.h
	-- Its exposure through behavior context and definition can be found at MouseCursorPositionSystemComponent.cpp
 	local normalizedCursorPosition = (CursorPositionRequestBus.Broadcast.GetSystemCursorPositionNormalized() * 2) - Vector2(1,1)

	-- Offsets for scroll based off our input event and screen based off mouse screen position
	local scrollOffset = Vector3(0, 0, self.scrollDirection * self.Properties.scrollSpeed * deltaTime)
	local screenOffset = Vector3(0)
	
	-- If the mouse is touching any side of the screen move in its direction at a set speed
	-- Use edgeWidth as an epsilon value. 
	-- An edgeWidth value of 0 may not cause movment in certain directions so we restrict its min value to .01
 	if Math.IsClose(normalizedCursorPosition.x, 1, self.Properties.edgeWidth) or 
       Math.IsClose(normalizedCursorPosition.y, 1, self.Properties.edgeWidth) or 
       Math.IsClose(normalizedCursorPosition.x, -1, self.Properties.edgeWidth) or 
       Math.IsClose(normalizedCursorPosition.y, -1, self.Properties.edgeWidth) then
		screenOffset = Vector3(normalizedCursorPosition.x, -normalizedCursorPosition.y, 0) * (self.Properties.cameraSpeed * deltaTime)
 	end
	
	-- Set our new position
	TransformBus.Event.SetLocalTranslation(self.entityId, TransformBus.Event.GetLocalTranslation(self.entityId) + scrollOffset + screenOffset)
	
	-- Reset our scroll direction to 0
	self.scrollDirection = 0
end

return CameraController