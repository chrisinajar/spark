local playeraccountbus = require "Scripts/UI/PlayerAccount/playeraccountbus"

local SignIn =
{
	Properties = 
	{
		Username = {default = EntityId()},
		Password = {default = EntityId()},
		
		MainMenu = {default = EntityId()},
		MainMenuSignedIn = {default = EntityId()},
		ChangePassword = {default = EntityId()},
		ConfirmSignUp = {default = EntityId()},
		
		Canvas = {default = EntityId()},
	},
}

function SignIn:OnActivate()
	self.playerAccountBus = playeraccountbus:new{
    self
    }

	self.tickBusHandler = TickBus.Connect(self)
	
	self.canvasNotificationHandler = nil
end

function SignIn:OnTick(deltaTime, timePoint)
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

function SignIn:OnDeactivate()
	self.tickBusHandler:Disconnect()
	if self.canvasNotificationHandler then
		self.canvasNotificationHandler:Disconnect()
	end
end

function SignIn:OnAction(entityId, actionName)	
	if actionName == "CancelSignIn" then
		UiElementBus.Event.SetIsEnabled(self.Properties.Canvas, false)
		UiElementBus.Event.SetIsEnabled(self.Properties.MainMenu, true)
	end
	
	if actionName == "SignInConfirm" then
		local username = UiTextBus.Event.GetText(self.Properties.Username)
        local password = UiTextBus.Event.GetText(self.Properties.Password)
		
		Debug.Log("Signing in as " .. username .. "...")
        self.playerAccountBus:InitiateAuth(username, password):OnComplete(function(result)
            if (result.wasSuccessful) then
                Debug.Log("Sign in success")
               -- self.menuManager:ShowMenu("MainMenu")
				UiElementBus.Event.SetIsEnabled(self.Properties.Canvas, false)
				UiElementBus.Event.SetIsEnabled(self.Properties.MainMenuSignedIn, true)
            else
                if result.errorTypeName == "ACCOUNT_BLACKLISTED" then
                    Debug.Log("Unable to sign in: the account is blacklisted.")
                elseif result.errorTypeName == "FORCE_CHANGE_PASSWORD" then
                    Debug.Log("Unable to sign in: A password change is required.")
					UiElementBus.Event.SetIsEnabled(self.Properties.Canvas, false)
					UiElementBus.Event.SetIsEnabled(self.Properties.ChangePassword, true)
                  --  self.menuManager:ShowMenu("ForceChangePassword")
                elseif result.errorTypeName == "UserNotConfirmedException" then
                    Debug.Log("Unable to sign in: A confirmation code is required.")
					UiElementBus.Event.SetIsEnabled(self.Properties.Canvas, false)
					UiElementBus.Event.SetIsEnabled(self.Properties.ConfirmSignUp, true)
                  --  self.menuManager:ShowMenu("ConfirmSignUp")
                else
                    Debug.Log("Sign in failed: " .. result.errorMessage)
                end
            end
        end)
        return
	end
end

return SignIn