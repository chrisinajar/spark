local Network = 
{
	AddressText = "";
	PortText = "";
	Properties =
	{
		AddressTextField = {default = EntityId()},
		PortTextField = {default = EntityId()},
	},
}

function Network:OnActivate()	
	self.tickBusHandler = TickBus.Connect(self);
	self.canvasNotificationHandler = nil	
end

function Network:OnTick()
	self.tickBusHandler:Disconnect()

	self.CanvasEntityId = UiElementBus.Event.GetCanvas(self.entityId)
	
	self.canvasNotificationHandler = UiCanvasNotificationBus.Connect(self, self.CanvasEntityId)
end

function Network:OnAction(entityId, actionName) 
	if actionName == "AddressEntered" or actionName == "AddressEditEnd" then
		self.AddressText = UiTextInputBus.Event.GetText(self.Properties.AddressTextField)
		Debug.Log(tostring(self.AddressText))
	end
	
	if actionName == "PortEntered" or actionName == "PortEditEnd" then
		self.PortText = UiTextInputBus.Event.GetText(self.Properties.PortTextField)
		Debug.Log(tostring(self.PortText))
	end
	
	if actionName == "NetworkStartGame" then
		if self.AddressText ~= nil and self.PortText ~= nil then
			self:OnStartNetworking(self.AddressText, self.PortText)
		end
	end
end

function Network:OnStartNetworking(address, port)
	-- Do console commands here
end

return Network;