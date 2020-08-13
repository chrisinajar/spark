require "gamemode.Spark.modules.hudtimer";

local GameTime = 
{
	CurrentUiTime = -1;
	Seconds = 0;
	Minutes = 0;
	Hours = 0;
	Day = 0;
	Weeks = 0;
	Months = 0;
	Years = 0;
	Properties = 
	{
		Time = {default = EntityId()},
		Cycle = {default = EntityId()},
	},
}

function GameTime:OnActivate()
	self.tickBusHandler = TickBus.Connect(self)
end

function GameTime:OnDeactivate()
	self.tickBusHandler:Disconnect()
end

function GameTime:SortGameTime(seconds)
	local seconds = tonumber(seconds)
	local time = ""
	
	if seconds == 0 then
		time = "00:00";
	elseif seconds < 0 then
		self.Seconds = string.format("%02.f", math.floor(seconds));
	else
		self.Hours = string.format("%02.f", math.floor(seconds/3600))
		self.Minutes = string.format("%02.f", math.floor(seconds/60 - (self.Hours*60)))
		self.Seconds = string.format("%02.f", math.floor(seconds - self.Hours*3600 - self.Minutes *60))
	end
	
	if seconds > 0 then
		if self.Hours == "00" then
			time = self.Minutes .. ":" .. self.Seconds
		else
			time = self.Hours .. ":" .. self.Minutes .. ":" .. self.Seconds
		end
	elseif seconds < 0 then
		if math.abs(seconds) >= 10 then
			time = "-00:" .. math.abs(self.Seconds)
		else
			time = "-00:0" .. math.abs(self.Seconds)
		end
	end
	self:SetGameTime(time)
end

function GameTime:SetGameTime(CurrentTime)
	UiTextBus.Event.SetText(self.Properties.Time, CurrentTime)
end

function GameTime:OnTick(deltaTime, timePoint)
	local time = GetGameManager():GetValue("time")
	if self.CurrentUiTime ~= time then
		self.CurrentUiTime = time
		self:SortGameTime(self.CurrentUiTime)
	end	
end

return GameTime