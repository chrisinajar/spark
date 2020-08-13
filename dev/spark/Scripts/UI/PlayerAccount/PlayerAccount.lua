local PlayerAccount =
{
	CurrentlyOpened = nil;
	
	Properties = 
	{
		MainMenu = {default = EntityId()},
		SignIn = {default = EntityId()},
		CreateAccount = {default = EntityId()},
		ChangePassword = {default = EntityId()},
		ConfirmSignUp = {default = EntityId()},
		EditAccount = {default = EntityId()},
		ForgotPassword = {default = EntityId()},
		MainMenuSignedIn = {default = EntityId()},
	},
}

function PlayerAccount:OnActivate()

	self.tickBusHandler = TickBus.Connect(self)
	
	self.canvasNotificationHandler = nil
	
	self.CurrentlyOpened = self.Properties.MainMenu
end

function PlayerAccount:OnTick(deltaTime, timePoint)
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

function PlayerAccount:OnDeactivate()
	self.tickBusHandler:Disconnect()
	if self.canvasNotificationHandler then
		self.canvasNotificationHandler:Disconnect()
	end
end

function PlayerAccount:OnAction(entityId, actionName)
	if actionName == "SignIn" then
		UiElementBus.Event.SetIsEnabled(self.Properties.MainMenu, false)
		UiElementBus.Event.SetIsEnabled(self.Properties.SignIn, true)
		self.CurrentlyOpened = self.Properties.SignIn
	end
	
	if actionName == "CreateNewAccount" then
		UiElementBus.Event.SetIsEnabled(self.Properties.MainMenu, false)
		UiElementBus.Event.SetIsEnabled(self.Properties.CreateAccount, true)
		self.CurrentlyOpened = self.Properties.CreateAccount
	end
	
	if actionName == "EditAccount" then
		UiElementBus.Event.SetIsEnabled(self.Properties.MainMenuSignedIn, false)
		UiElementBus.Event.SetIsEnabled(self.Properties.EditAccount, true)
		self.CurrentlyOpened = self.Properties.EditAccount
	end
	
	if actionName == "ManagerChangePassword" then
		UiElementBus.Event.SetIsEnabled(self.Properties.MainMenuSignedIn, false)
		UiElementBus.Event.SetIsEnabled(self.Properties.ChangePassword, true)
		self.CurrentlyOpened = self.Properties.ChangePassword
	end
	
	if actionName == "SignOut" then
		UiElementBus.Event.SetIsEnabled(self.Properties.MainMenuSignedIn, false)
		UiElementBus.Event.SetIsEnabled(self.Properties.MainMenu, true)
		self.CurrentlyOpened = self.Properties.MainMenu
	end
	
	if actionName == "Exit" then
		
	end

	if actionName == "ForgotPassword" then
		UiElementBus.Event.SetIsEnabled(self.Properties.SignIn, false)
		UiElementBus.Event.SetIsEnabled(self.Properties.ForgotPassword, true)
		self.CurrentlyOpened = self.Properties.ForgotPassword
	end
	
	if actionName == "VerifyAccount" then
		UiElementBus.Event.SetIsEnabled(self.Properties.CreateAccount, false)
		UiElementBus.Event.SetIsEnabled(self.Properties.ConfirmSignUp, true)
		self.CurrentlyOpened = self.Properties.ConfirmSignUp
	end
end

return PlayerAccount