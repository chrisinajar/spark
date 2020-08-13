require "scripts.library.event"
local json = require "scripts.json"

ClientEvents = CreateGlobalVariable("ClientEvents", {
	handler = nil,
	events = {}
})

function ClientEvents:OnActivate ()
	Debug.Log('Initializing client events!')
	if self.readyHandler then
		self.readyHandler()
		self.readyHandler = nil
	end
	
	self.readyHandler = ConnectToEvents(GameManagerNotificationBus, nil, { "OnGameManagerReady" })
	self.readyHandler.OnGameManagerReady(partial(self.OnGameManagerReady, self))
end

function ClientEvents:OnGameManagerReady()
	if self.readyHandler then
		self.readyHandler()
		self.readyHandler = nil
	end
	if self.handler then
		self.handler()
		self.handler = nil
	end
	self.handler = ConnectToEvents(GamePlayerNotificationBus, GetGameManager():GetLocalPlayer().entityId, { "OnClientEvent" })
	self.handler.OnClientEvent(partial(self.OnClientEvent, self))
end

function ClientEvents:OnDeactivate ()
	if self.handler then
		self.handler()
		self.handler = nil
	end
	self.events = {}
end

function ClientEvents:OnClientEvent (name, data, entity)
	data = json.parse(data)
	Debug.Log(name .. " event with " .. tostring(data.text))
	if self.events[name] then
		self.events[name].broadcast(data, entity)
	end
end

function ListenToClientEvent (name, fn)
	if not ClientEvents.events[name] then
		ClientEvents.events[name] = Event()
	end
	return ClientEvents.events[name].listen(fn)
end
