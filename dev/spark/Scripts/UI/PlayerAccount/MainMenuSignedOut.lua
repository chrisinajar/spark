local MainMenuSignedOut =
{
	Properties = 
	{

	},
}

function MainMenuSignedOut:OnActivate()

	self.tickBusHandler = TickBus.Connect(self)
	
	self.canvasNotificationHandler = nil
end

function MainMenuSignedOut:OnTick(deltaTime, timePoint)
	--self.tickBusHandler:Disconnect()
	
	self.CanvasEntityId = self.CanvasEntityId or UiElementBus.Event.GetCanvas(self.entityId)
	if self.CanvasEntityId ~= nil then
		if self.canvasNotificationHandler then
			self.canvasNotificationHandler:Disconnect()
			self.canvasNotificationHandler = nil
		end		
		self.canvasNotificationHandler = UiCanvasNotificationBus.Connect(self, self.CanvasEntityId)
	end
end

function MainMenuSignedOut:OnDeactivate()
	self.tickBusHandler:Disconnect()
	if self.canvasNotificationHandler then
		self.canvasNotificationHandler:Disconnect()
	end
end

return MainMenuSignedOut