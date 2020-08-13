local playeraccountbus = require "Scripts/UI/PlayerAccount/playeraccountbus"

local CreateAccount =
{
	Properties = 
	{
		Username = {default = EntityId()},
		Password = {default = EntityId()},
		Email = {default = EntityId()},
		
		MainScreen = {default = EntityId()},
		ConfirmationScreen = {default = EntityId()},
		Canvas = {default = EntityId()},
	},
}

function CreateAccount:OnActivate()
	self.playerAccountBus = playeraccountbus:new{
    self
    }

	self.tickBusHandler = TickBus.Connect(self)
	
	self.canvasNotificationHandler = nil
	
	
end

function CreateAccount:OnTick(deltaTime, timePoint)
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

function CreateAccount:OnDeactivate()
	self.tickBusHandler:Disconnect()
	if self.canvasNotificationHandler then
		self.canvasNotificationHandler:Disconnect()
	end
end

function CreateAccount:OnAction(entityId, actionName)
	if actionName == "CreateAccount" then
		local username = UiTextBus.Event.GetText(self.Properties.Username)
        local password = UiTextBus.Event.GetText(self.Properties.Password)
        local email = UiTextBus.Event.GetText(self.Properties.Email)
		
		local attributes = UserAttributeValues();
        attributes:SetAttribute("email", email);
		
		Debug.Log("Signing up as " .. username .. "...")
		
		 self.playerAccountBus:SignUp(username, password, attributes):OnComplete(function(result)
            if (result.wasSuccessful) then
                Debug.Log("Sign up success")
                --self.menuManager:ShowMenu("ConfirmSignUp")
				UiElementBus.Event.SetIsEnabled(self.Properties.Canvas, false)
				UiElementBus.Event.SetIsEnabled(self.Properties.ConfirmationScreen, true)
            else
                Debug.Log("Sign up failed: " .. result.errorMessage)
            end
        end)
        return		
	end
	
	if actionName == "CancelCreateAccount" then
		UiElementBus.Event.SetIsEnabled(self.Properties.Canvas, false)
		UiElementBus.Event.SetIsEnabled(self.Properties.MainScreen, true)
	end
end

return CreateAccount