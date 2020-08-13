require "scripts.library.class"

Module = class()

function Module:Init()
	Debug.Log(self.name .. ' using default init method!')
	self.gameManager = GetGameManager()
end

function Module:ListenToUIEvent (name)
	local unlisten = ListenToUIEvent(name, partial(self[name], self))
	self.unlistenAll(unlisten)
	return unlisten
end
