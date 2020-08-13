require "scripts.library.event"

--[[

Create table of event objects connected to named events on an EBUS

If entityId is nil then it will connect to an unaddressed bus

Example:

	self.triggerAreaEvents = ConnectToEvents(TriggerAreaNotificationBus, self.entityId, {
		"OnTriggerAreaEntered",
		"OnTriggerAreaExited",
	})


The returned table has listener functions named after each event
Executing the table as a function destroys all events and unlistens from the bus

Example:
	local handler = ConnectToEvents(SomeAwesomeBus, entityId, { "OnMyCoolEvent" })
	local unlisten = handler.OnMyCoolEvent(myCallback)
	-- unlisten() unregisters that specific event listener
	-- handler() closes down everything
]]
function ConnectToEvents (bus, entityId, events)
	local proxy = {}
	local apiEvents = {}
	local handler = nil
	local unlistenAll = Event()

	for i,eventName in ipairs(events) do
		local localEvent = Event()
		local function proxyBroadcast (obj, ...)
			localEvent.broadcast(...)
		end
		proxy[eventName] = proxyBroadcast
		apiEvents[eventName] = localEvent.listen
		unlistenAll.listen(function ()
			localEvent.unlistenAll()
		end)
	end

	if entityId ~= nil then
		handler = bus.Connect(proxy, entityId)
	else
		handler = bus.Connect(proxy)
	end

	local mt = {}
	mt.__call = function ()
		if handler then
			handler:Disconnect()
			handler = nil
		end
		unlistenAll.broadcast({})
		unlistenAll.unlistenAll()
	end

    setmetatable(apiEvents, mt)

    apiEvents.proxy = proxy

	return apiEvents
end

function CreatePropertyGetter (obj, name, typeName)
	local eventId = GameplayNotificationId(obj.entityId, "GetPropertyValue" .. name, typeName)
	local handler = ConnectToEvents(GameplayNotificationBus, eventId, { "OnEventBegin" })
	handler.name = name
	handler.eventId = eventId
	handler.OnEventBegin(function ()
		GameplayNotificationBus.Event.OnEventUpdating(eventId, obj.Properties[name])
	end)

	return handler
end

function GetComponentProperty (entityId, name, typeName)
	local eventId = GameplayNotificationId(entityId, "GetPropertyValue" .. name, typeName)
	local handler = ConnectToEvents(GameplayNotificationBus, eventId, { "OnEventUpdating" })
	local result = nil
	handler.OnEventUpdating(function (value)
		result = value
	end)
	GameplayNotificationBus.Event.OnEventBegin(eventId, nil)
	handler()

	return result
end
