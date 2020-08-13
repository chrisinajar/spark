require "scripts.library.ebus"
require "scripts.library.globals"

CreateGlobalVariable("__timerID", 0)
CreateGlobalVariable("__timerLinkedList", {
	next = nil,
	prev = nil
})
if __timerLinkedList.prev == nil then
	__timerLinkedList.prev = __timerLinkedList
end

local function createCallback (cb, cancelEvent, interval)
	local function wrapper ()
		local retVal = cb()
		if interval then
			retVal = interval
		end
		if type(retVal) == "number" then
			cancelEvent(scheduleCallback(wrapper, retVal))
		end
	end

	return wrapper
end

function scheduleCallback (cb, interval)
	local retTable = {
		handle = nil,
		handlerObject = nil
	}
	retTable.removeFromLinkedList = function ()
		retTable.handlerObject = nil
		retTable.handle = nil
		if retTable.next then
			retTable.next.prev = retTable.prev
		end
		retTable.prev.next = retTable.next
	end
	retTable.handlerObject = {
		OnTimerFinished = function (self, name)
			-- Debug.Log('Running timer event for ' .. name)
			if retTable.handle then
				retTable.handle:Disconnect()
				retTable.removeFromLinkedList()
				cb()
			end
		end
	}

	if interval == nil then
		interval = 0
	end
	__timerID = __timerID + 1
	local timerId = "Timer" .. __timerID
	retTable.handle = TimerNotificationBus.Connect(retTable.handlerObject, TimerRequestBus.Broadcast.ScheduleTimer(interval, "Timer" .. __timerID))
	-- Debug.Log("Created new timer " .. timerId)

	local mt = {}
	mt.__call = function ()
		-- Debug.Log('Handler called, disconnecting from timer bus ' .. timerId)
		if retTable.handle then
			retTable.handle:Disconnect()
			retTable.removeFromLinkedList()
		end
	end

	setmetatable(retTable, mt)

	retTable.next = __timerLinkedList.next
	retTable.prev = __timerLinkedList
	
	if __timerLinkedList.next then
		__timerLinkedList.next.prev = retTable
	end

	__timerLinkedList.next = retTable


	return retTable
end

function CreateTimer (cb, interval, callbackInterval)
	local currentHandler = nil
	local isCanceled = false
	local function wrapper ()
		-- Debug.Log('Canceling this timer! ' .. tostring(currentHandler))
		isCanceled = true
		if currentHandler then
			currentHandler()
			currentHandler = nil
		end
	end
	local function setHandler (handler)
		-- Debug.Log('Handler update!')
		if isCanceled then
			handler()
		else
			currentHandler = handler
		end
	end
	setHandler(scheduleCallback(createCallback(cb, setHandler, callbackInterval), interval))

	return wrapper
end

function CreateInterval (cb, interval)
	return CreateTimer(cb, interval, interval)
end

function DestroyTimers ()
	while __timerLinkedList.next do
		__timerLinkedList.next()
	end
end

return {
	__timerLinkedList = __timerLinkedList,
	CreateTimer = CreateTimer,
	CreateInterval = CreateInterval,
}
