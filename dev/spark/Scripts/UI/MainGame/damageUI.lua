local damageUI = 
{
	Properties =
	{
		TextElement = {default = EntityId()},
	},
}

function damageUI:OnTick(deltaTime, timePoint)
	self.timer = self.timer + deltaTime
	local pos = UiTransformBus.Event.GetViewportPosition(self.Properties.TextElement)
	pos.y = pos.y - 0.25
	UiTransformBus.Event.SetViewportPosition(self.Properties.TextElement, pos)
	
	local color = UiTextBus.Event.GetColor(self.Properties.TextElement)
	color.a = color.a - 0.05
	UiTextBus.Event.SetColor(self.Properties.TextElement, color)

	if( self.timer > 1.0) then
		UiElementBus.Event.DestroyElement(self.entityId)
	end
end

function damageUI:OnActivate()
	self.timer = self.timer or 0
	self.tickBusHandler = TickBus.Connect(self)
end

function damageUI:OnDeactivate()
	self.tickBusHandler:Disconnect()
end

return damageUI;