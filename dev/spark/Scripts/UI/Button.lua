local Button = 
{
	pressed = false;
	Properties = 
	{
		Button = {default = EntityId()},
		Text = {default = EntityId()},
		
		TextProperties = 
		{
			Normalcolor = {default = Color(197/255, 197/255, 197/255)},
			Pressedcolor = {default = Color(255/255, 255/255, 255/255)},
			Hovercolor = {default = Color(0/255, 255/255, 255/255)},
		},
		
		ButtonProperties = 
		{
			ChangeButtonProperties = {default = false},
			Normal = {default = EntityId()},
			Pressed = {default = EntityId()},
			Hover = {default = EntityId()},
		},
	}
}
function Button:OnActivate()
	self.interactableHandler = UiInteractableNotificationBus.Connect(self, self.Properties.Button)
	self.buttonHandler = UiButtonNotificationBus.Connect(self, self.Properties.Button)

	self.tickBusHandler = TickBus.Connect(self)
	self.hovering = false
end

function Button:OnTick(deltaTime, timePoint)
	self.tickBusHandler:Disconnect()

	self.canvas = UiElementBus.Event.GetCanvas(self.entityId)	
end

function Button:OnDeactivate()
	self.interactableHandler:Disconnect()
	self.buttonHandler:Disconnect()
end

function Button:OnHoverStart()	
	if self.Properties.Text ~= nil then
		UiTextBus.Event.SetColor(self.Properties.Text, self.Properties.TextProperties.Hovercolor)
	end
	if self.Properties.ButtonProperties.ChangeButtonProperties ~= false then
		UiElementBus.Event.SetIsEnabled(self.Properties.ButtonProperties.Normal, false)
		UiElementBus.Event.SetIsEnabled(self.Properties.ButtonProperties.Hover, true)
	end
end

function Button:OnHoverEnd()
	if self.pressed == false then
		if self.Properties.Text ~= nil then
			UiTextBus.Event.SetColor(self.Properties.Text, self.Properties.TextProperties.Normalcolor)
		end
		
	end
	if self.pressed == true then
		if self.Properties.Text ~= nil then
			UiTextBus.Event.SetColor(self.Properties.Text, self.Properties.TextProperties.Pressedcolor)
		end
	end
	if self.Properties.ButtonProperties.ChangeButtonProperties ~= false then
		UiElementBus.Event.SetIsEnabled(self.Properties.ButtonProperties.Normal, true)
		UiElementBus.Event.SetIsEnabled(self.Properties.ButtonProperties.Hover, false)
		UiElementBus.Event.SetIsEnabled(self.Properties.Text, true)
	end
	self.pressed = false
end

function Button:OnButtonClick()
	self.pressed = true
	if self.Properties.Text ~= nil then
		UiTextBus.Event.SetColor(self.Properties.Text, self.Properties.TextProperties.Pressedcolor)
	end
	if self.Properties.ButtonProperties.ChangeButtonProperties ~= false then
		UiElementBus.Event.SetIsEnabled(self.Properties.ButtonProperties.Hover, false)
		UiElementBus.Event.SetIsEnabled(self.Properties.ButtonProperties.Pressed, true)
		UiElementBus.Event.SetIsEnabled(self.Properties.Text, false)
	end
end

return Button