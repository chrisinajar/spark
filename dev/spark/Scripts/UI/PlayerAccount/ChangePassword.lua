local playeraccountbus = require "Scripts/UI/PlayerAccount/playeraccountbus"

local ChangePassword =
{
	Properties = 
	{
		Username = {default = EntityId()},
		OldPassword = {default = EntityId()},
		NewPassword = {default = EntityId()},
		ConfirmNewPassword = {default = EntityId()},
		
		MainMenu = {default = EntityId()},
		Canvas = {default = EntityId()},
	},
}

function ChangePassword:OnActivate()
	self.playerAccountBus = playeraccountbus:new{
		self
		}

	self.tickBusHandler = TickBus.Connect(self)
	
	self.canvasNotificationHandler = nil
end

function ChangePassword:OnTick(deltaTime, timePoint)
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

function ChangePassword:OnDeactivate()
	self.tickBusHandler:Disconnect()
	if self.canvasNotificationHandler then
		self.canvasNotificationHandler:Disconnect()
	end
end

function ChangePassword:OnAction(entityId, actionName)
	if actionName == "ChangePassword" then
		local username = UiTextBus.Event.GetText(self.Properties.Username)
        local oldPassword = UiTextBus.Event.GetText(self.Properties.OldPassword)
        local newPassword = UiTextBus.Event.GetText(self.Properties.NewPassword)
		local confirmNewPassword = UiTextBus.Event.GetText(self.Properties.ConfirmNewPassword)
        
        Debug.Log("Changing password for " .. username .. "...")
        self.playerAccountBus:ChangePassword(username, oldPassword, newPassword):OnComplete(function(result)
            if (result.wasSuccessful) then
                Debug.Log("Change password success")
				UiElementBus.Event.SetIsEnabled(self.Properties.Canvas, false)
				UiElementBus.Event.SetIsEnabled(self.Properties.MainMenu, true)
                --self.menuManager:ShowMenu("MainMenu")				
            else
                Debug.Log("Change password failed: " .. result.errorMessage)
            end
        end)
        return
	end
	
	if actionName == "CancelChangePassword" then
		UiElementBus.Event.SetIsEnabled(self.Properties.Canvas, false)
	end
end

return ChangePassword