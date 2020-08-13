local o = {
    Properties = 
    {
		time = { default = 1.0, description = "time before de-spawning"}    
    }
}

function o:OnActivate()
    self.tickBusHandler = TickBus.Connect(self);
    self.timer = self.Properties.time;
end

function o:OnDeactivate()
    if self.tickBusHandler then
        self.tickBusHandler:Disconnect()
    end
end

function o:OnTick(deltaTime, timePoint)
    self.timer = self.timer - deltaTime
    if(self.timer<=0) then 
        self.tickBusHandler:Disconnect()
        GameEntityContextRequestBus.Broadcast.DestroyGameEntity(self.entityId);
    end
end

return o;