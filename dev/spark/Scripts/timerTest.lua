


local o={}


function o:AddTimer(seconds, description)
	self.timerHandler = self.timerHandler or {}
	
	local ticket=TimerRequestBus.Broadcast.ScheduleTimer(seconds,description)
	local handler=TimerNotificationBus.Connect(self,ticket)
    table.insert(self.timerHandler, handler)
end

function o:OnActivate()
	self:AddTimer(2,"2s timer")
	self:AddTimer(1.5,"1.5s timer")
	self:AddTimer(1,"1s timer")
	self:AddTimer(0,"0s timer")
	self:AddTimer(4);--also no description works
	self:AddTimer(3,"3s timer")
	self:AddTimer(5,"end timer")
end

function o:OnTimerFinished(description)
	Debug.Log("OnTimerFinished "..description)
end

return o;
