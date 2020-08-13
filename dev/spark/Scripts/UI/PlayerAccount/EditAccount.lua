local playeraccountbus = require "Scripts/UI/PlayerAccount/playeraccountbus"

local EditAccount =
{
	Properties = 
	{
		PlayerName = {default = EntityId()},
		Email = {default = EntityId()},
		
		MainMenuSignedIn = {default = EntityId()},
	},
}

function EditAccount:OnActivate()
	self.playerAccountBus = playeraccountbus:new{
    self
    }

	self.tickBusHandler = TickBus.Connect(self)
	
	self.canvasNotificationHandler = nil
end

function EditAccount:OnTick(deltaTime, timePoint)
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

function EditAccount:OnDeactivate()
	self.tickBusHandler:Disconnect()
	if self.canvasNotificationHandler then
		self.canvasNotificationHandler:Disconnect()
	end
end

function EditAccount:OnAction(entityId, actionName)
	if actionName == "SaveAccountInfo" then
		local playerName = UiTextBus.Event.GetText(self.Properties.PlayerName)
        local email = UiTextBus.Event.GetText(self.Properties.Email)

        local playerAccount = PlayerAccount();
        playerAccount:SetPlayerName(playerName);

        local attributes = UserAttributeValues();
        attributes:SetAttribute("email", email);

        Debug.Log("Updating account...")
        local updatePlayerAsyncResult = self.playerAccountBus:UpdatePlayerAccount(playerAccount)
        local updateUserAttributesAsyncResult = self.playerAccountBus:UpdateUserAttributes(self.context.username, attributes)
        updatePlayerAsyncResult:Chain(updateUserAttributesAsyncResult):OnComplete(function(updatePlayerParameters, updateUserParameters)
            local updatePlayerResult = unpack(updatePlayerParameters)
            local updateUserResult = unpack(updateUserParameters)

            if not updatePlayerResult.wasSuccessful then
                Debug.Log("Failed to update account: " .. updatePlayerResult.errorMessage)
            end

            if not updateUserResult.wasSuccessful then
                Debug.Log("Failed to update user pool: " .. updateUserResult.errorMessage)
            end

            if updatePlayerResult.wasSuccessful and updateUserResult.wasSuccessful then
                --self.menuManager:ShowMenu("MainMenu")
				UiElementBus.Event.SetIsEnabled(self.CanvasEntityId, false)
				UiElementBus.Event.SetIsEnabled(self.Properties.MainMenuSignedIn, true)
            end
        end)
        return
	end
	
	if actionName == "CancelAccountInfoChange" then
		UiElementBus.Event.SetIsEnabled(self.CanvasEntityId, false)
		UiElementBus.Event.SetIsEnabled(self.Properties.MainMenuSignedIn, true)
	end
end

return EditAccount