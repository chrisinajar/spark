require "scripts.library.event"
local json = require "scripts.json"

UIEvents = CreateGlobalVariable("UIEvents", {
	handler = nil,
	events = {}
})

function UIEvents:OnActivate ()
	if self.handler then
		self.handler()
		self.handler = nil
	end

	self.handler = ConnectToEvents(GameNetSyncNotificationBus, nil, { "OnUIEvent" })
	self.handler.OnUIEvent(partial(self.OnUIEvent, self))
end

function UIEvents:OnDeactivate ()
	if self.handler then
		self.handler()
		self.handler = nil
	end
	self.events = {}
end

function UIEvents:OnUIEvent (playerEntity, name, data)
	local player = Player({ entityId = playerEntity })
	data = json.parse(data)
	if self.events[name] then
		self.events[name].broadcast(data, player)
	end
end	

function SendUIEvent (name, data)
	GameNetSyncRequestBus.Broadcast.SendUIEvent(name, json.stringify(data))
end

function ListenToUIEvent (name, fn)
	if not UIEvents.events[name] then
		UIEvents.events[name] = Event()
	end
	return UIEvents.events[name].listen(fn)
end
