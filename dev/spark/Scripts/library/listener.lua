require "scripts.library.class"
require "scripts.common"

EventListener = class();


function get_mem_addr (object)
	local str = tostring(object)  
	local index = str:find(' ')  
    return index and str:sub( index + 1) or str
end

function EventListener:ListenEvent(bus,event,id)
    
    if type(bus)=='string' then
        bus = _G[bus]
    end

    if bus==nil then 
        Debug.Warning("EventListener:ListenEvent called with an invalid bus. Perhaps it doesn't exist or is not serialized")
        return
    end

    if type(event)~='string' then
        Debug.Warning("EventListener:ListenEvent called with an invalid event name")
        return
    end

    id = id or self.entityId

    self.handlers = self.handlers or {}
    
    local bus_index = get_mem_addr(bus)

   
    local handler = self.handlers[bus_index] or {}

    handler.owner = self

    if handler.Disconnect then handler:Disconnect() end

    Debug.Log("setting event callback for : "..event)
    handler[event] = function(self,a,b,c,d,e,f)
        --Debug.Log("calling "..event)
        if self.owner and self.owner[event] then self.owner[event](self.owner,a,b,c,d,e,f) end
    end

    handler.handler = bus.Connect(handler,id)
    handler.Disconnect = function(self)
        self.handler:Disconnect()
    end

    self.handlers[bus_index] = handler
end

function EventListener:UnlistenEvent(bus,event,id)
    if type(bus)=='string' then
        bus = _G[bus]
    end

    if bus==nil then 
        Debug.Warning("EventListener:UnlistenEvent called with an invalid bus. Perhaps it doesn't exist or is not serialized")
        return
    end

    if type(event)~='string' then
        Debug.Warning("EventListener:UnlistenEvent called with an invalid event name")
        return
    end

    self.handlers = self.handlers or {}
    
    local bus_index = get_mem_addr(bus)
    local bus_handler = self.handlers[bus_index]

    if bus_handler~=nil then
        bus_handler[event] = nil
    
        local count = 0;
        for k,v in pairs(bus_handler) do
            count = count + 1
        end
        Debug.Log("EventListener:UnlistenEvent count is now: "..count)

        if count <= 3 then
            self:UnlistenFromBus(bus)
        end
    end
end

function EventListener:UnlistenFromBus(bus)
    if type(bus)=='string' then
        bus = _G[bus]
    end

    if bus==nil then 
        Debug.Warning("EventListener:UnlistenFromBus called with an invalid bus. Perhaps it doesn't exist or is not serialized")
        return
    end

    self.handlers = self.handlers or {}
    
    local bus_index = get_mem_addr(bus)
    local bus_handler = self.handlers[bus_index]

    if bus_handler~=nil then
        bus_handler:Disconnect()
        self.handlers[bus_index] = nil
        Debug.Log("EventListener:UnlistenFromBus disconnected from bus")
    end
end

function EventListener:DisconnectFromAllEvents ()
    if self.handlers~=nil then
        for k,v in pairs(self.handlers) do
            if v.handler and v.handler.Disconnect then 
                v.handler:Disconnect() 
                v.handler = nil
            end
        end
    end
end

function EventListener:OnDestroy()
    --Debug.Log("EventListener:OnDestroy()")
    self:DisconnectFromAllEvents()
end


