
local Camera = {
    Properties =
	{
        elevation = {default = 10 , description = "elevation from the terrain"}, 
        height_probe = {default = EntityId()},
        probe_size = {default = 10},
	},
}

function Camera:OnActivate()
    self.tickBusHandler = TickBus.Connect(self)	
end

function Camera:OnDeactivate()
    if self.tickBusHandler~=nil then 
        self.tickBusHandler:Disconnect()
    end
end

function lerp(v0, v1, t)
    return v0 + t * (v1 - v0)
end

function Camera:GetElevation()
    local center = TransformBus.Event.GetWorldTranslation(self.Properties.height_probe)

    local delta = Vector3(self.Properties.probe_size,self.Properties.probe_size,0)
    local avg = 0;

    avg = avg + MapRequestBus.Broadcast.GetTerrainHeightAt(center) * 2
    avg = avg + MapRequestBus.Broadcast.GetTerrainHeightAt(center + delta)
    avg = avg + MapRequestBus.Broadcast.GetTerrainHeightAt(center - delta)
    delta.x = -delta.x
    avg = avg + MapRequestBus.Broadcast.GetTerrainHeightAt(center + delta)
    avg = avg + MapRequestBus.Broadcast.GetTerrainHeightAt(center - delta)

    avg = avg / 6

    return avg;
end

function Camera:OnTick(deltaTime, timePoint)
    local pos = TransformBus.Event.GetWorldTranslation(self.entityId)
    
    pos.z = lerp(pos.z,self:GetElevation() + self.Properties.elevation,0.1);

    TransformBus.Event.SetWorldTranslation(self.entityId,pos)

end

return Camera;