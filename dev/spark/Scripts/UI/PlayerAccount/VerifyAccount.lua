local playeraccountbus = require "Scripts/UI/PlayerAccount/playeraccountbus"

local VerifyAccount =
{
	Properties = 
	{
		Username = {default = EntityId()},
		Code = {default = EntityId()},
		
		MainMenu = {default = EntityId()},
		SignIn = {default = EntityId()},
		Canvas = {default = EntityId()},
	},
}

function VerifyAccount:OnActivate()
	self.playerAccountBus = playeraccountbus:new{
    self
    }

	self.tickBusHandler = TickBus.Connect(self)
	
	self.canvasNotificationHandler = nil
end

function VerifyAccount:OnTick(deltaTime, timePoint)
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

function VerifyAccount:OnDeactivate()
	self.tickBusHandler:Disconnect()
	if self.canvasNotificationHandler then
		self.canvasNotificationHandler:Disconnect()
	end
end

function VerifyAccount:OnAction(entityId, actionName)
	if actionName == "VerifyAccount" then
		local username = UiTextBus.Event.GetText(self.Properties.Username)
        local code = UiTextBus.Event.GetText(self.Properties.Code)
		
        Debug.Log("Sending confirmation code for " .. username .. "...")
        self.playerAccountBus:ConfirmSignUp(username, code:match("^%s*(.-)%s*$")):OnComplete(function(result)
            if (result.wasSuccessful) then
                Debug.Log("Confirm signup success")
				UiElementBus.Event.SetIsEnabled(self.Properties.Canvas, false)
				UiElementBus.Event.SetIsEnabled(self.Properties.SignIn, true)
                --self.menuManager:ShowMenu("SignIn", {username = self.username})
            else
                Debug.Log("Confirm signup failed: " .. result.errorMessage)
            end
        end)
        return
	end
	
	if actionName == "CancelVerify" then
		UiElementBus.Event.SetIsEnabled(self.Properties.Canvas, false)
		UiElementBus.Event.SetIsEnabled(self.Properties.MainMenu, true)
	end
end

return VerifyAccount