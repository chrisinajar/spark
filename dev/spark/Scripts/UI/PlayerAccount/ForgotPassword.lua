local playeraccountbus = require "Scripts/UI/PlayerAccount/playeraccountbus"

local ForgotPassword =
{
	Properties = 
	{
		Username = {default = EntityId()},
		Password = {default = EntityId()},
		Code = {default = EntityId()},
		
		SignIn = {default = EntityId()},
		Canvas = {default = EntityId()},
	},
}

function ForgotPassword:OnActivate()
	self.playerAccountBus = playeraccountbus:new{
    self
    }

	self.tickBusHandler = TickBus.Connect(self)
	
	self.canvasNotificationHandler = nil
end

function ForgotPassword:OnTick(deltaTime, timePoint)
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

function ForgotPassword:OnDeactivate()
	self.tickBusHandler:Disconnect()
	if self.canvasNotificationHandler then
		self.canvasNotificationHandler:Disconnect()
	end
end

function ForgotPassword:OnAction(entityId, actionName)
	if actionName == "ChangeForgottenPassword" then
		local username = UiTextBus.Event.GetText(self.Properties.Username)
        local password = UiTextBus.Event.GetText(self.Properties.Password)
        local code = UiTextBus.Event.GetText(self.Properties.Code)
        
        Debug.Log("Changing forgotten password for " .. username .. "...")
        self.playerAccountBus:ConfirmForgotPassword(username, password, code:match("^%s*(.-)%s*$")):OnComplete(function(result)
            if (result.wasSuccessful) then
                Debug.Log("Change forgotten password success")
				UiElementBus.Event.SetIsEnabled(self.Properties.Canvas, false)
				UiElementBus.Event.SetIsEnabled(self.Properties.SignIn, true)
				
                --self.menuManager:ShowMenu("SignIn")
            else
                Debug.Log("Change forgotten password failed: " .. result.errorMessage)
            end
        end)
        return
	end
	
	if actionName == "CancelForgottenPassword" then
		UiElementBus.Event.SetIsEnabled(self.Properties.Canvas, false)
		UiElementBus.Event.SetIsEnabled(self.Properties.SignIn, true)
	end
	
	if actionName == "SendCodeForgotPassword" then
		local username = UiTextBus.Event.GetText(self.Properties.Username)
        self.playerAccountBus:ForgotPassword(username):OnComplete(function(result)
            if (result.wasSuccessful) then
                Debug.Log("Send code success")
            else
                Debug.Log("Send code failed: " .. result.errorMessage)
            end
        end)
        return
	end
end

return ForgotPassword