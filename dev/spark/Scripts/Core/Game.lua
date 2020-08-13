require "scripts.GameUtils"
require "scripts.library.module"
require "scripts.library.globals"
require "scripts.library.loaders"
require "scripts.library.ui_events"
require "scripts.library.client_events"
require "scripts.core.gamemanager"
require "scripts.core.camera"

SystemGame = CreateGlobalVariable("SystemGame", {})

function SystemGame:GetGameMode ()
	return InfoRequestBus.Broadcast.GetGameModeName() or "";
end

function SystemGame:OnActivate ()
	Debug.Log("SystemGame:OnActivate() " .. tostring(self.IsAuthoritative))
	SystemGame.entityId = GameManagerRequestBus.Broadcast.GetGameManagerEntity()
	local gameName = self:GetGameMode()
	LuaLoaders:Activate()
	UIEvents:OnActivate()
	ClientEvents:OnActivate()
	Camera:OnActivate()

	InitModules()

	self.gameMode = require("gamemode." .. gameName .. ".game");
	self.gameMode.IsAuthoritative = self.IsAuthoritative;
	self.gameMode:OnActivate(gameName)
end

function SystemGame:OnDeactivate ()
	Debug.Log("SystemGame:OnDeactivate()")
	self.gameMode:OnDeactivate()
	UIEvents:OnDeactivate()
	ClientEvents:OnDeactivate()
	Camera:OnDeactivate()
	DestroyModules()
	DestroyTimers()
	LuaLoaders:Deactivate()
	Debug.Log("SystemGame:OnDeactivatefinished")
end

return SystemGame;
