local MainMenuSignedIn =
{
	Properties = 
	{

	},
}

function MainMenuSignedIn:OnActivate()

	self.tickBusHandler = TickBus.Connect(self)
	
	self.canvasNotificationHandler = nil
end

function MainMenuSignedIn:OnTick(deltaTime, timePoint)
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

function MainMenuSignedIn:OnDeactivate()
	self.tickBusHandler:Disconnect()
	if self.canvasNotificationHandler then
		self.canvasNotificationHandler:Disconnect()
	end
end

function MainMenuSignedIn:OnAction(entityId, actionName)
	if actionName == "EditAccount" then
	
	end
	
	if actionName == "ManagerChangePassword" then
	
	end
	
	if actionName == "SignOut" then
	
	end
	
	if actionName == "Exit" then
	
	end
end

return MainMenuSignedIn