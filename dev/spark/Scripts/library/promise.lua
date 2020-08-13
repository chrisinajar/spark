Event = require("scripts.library.event")

function newPromise ()
	local resolved = false
	local value = nil
	local isError = false
	local resolveEvent = Event()
	local rejectEvent = Event()
	local api = {}

	local function resolve (data)
		if resolved then
			return
		end
		resolved = true

		value = data
		resolveEvent.broadcast(value)
		resolveEvent.unlistenAll()
		rejectEvent.unlistenAll()
	end

	local function reject (error)
		if resolved then
			return
		end
		resolved = true

		isError = true
		value = error
		rejectEvent.broadcast(value)
		rejectEvent.unlistenAll()
		resolveEvent.unlistenAll()
	end

	function valueMethod (handler)
		if resolved then
			if isError then
				return
			end
			return Promise.wrap(handler(value))
		end
		local promise = Promise()

		resolveEvent.listen(function (value)
			Promise.wrap(handler(value)).value(promise.resolve)
		end)

		return promise
	end

	function catch (handler)
		if resolved then
			if not isError then
				return
			end
			handler(value)
			return
		end
		rejectEvent.listen(handler)

		return api
	end

	api.resolve = resolve
	api.reject = reject
	api.value = valueMethod
	api.catch = catch
	api.isPromise = true

	return api
end

Promise = {}
local mt = {}
mt.__call = newPromise
setmetatable(Promise, mt)

Promise.new = function (factory)
	local promise = Promise()

	factory(promise.resolve, promise.reject)

	return promise
end

Promise.resolve = function (value)
	local promise = Promise()
	promise.resolve(value)
	return promise
end

Promise.wrap = function (value)
	if not value or not value.isPromise then
		return Promise.resolve(value)
	end
	return value
end

return Promise
