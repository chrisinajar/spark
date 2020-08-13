
HudTimer = CreateModule("HudTimer", GAME_PHASE_PRE_GAME)

function HudTimer:Init ()
	self.gameManager = GetGameManager()
	self.gameManager:RegisterVariable("time", self.time)
	self.time = self:GetStartTime()

	self.timer = CreateInterval(partial(self.OnTimer, self), 1)
end

function HudTimer:GetStartTime ()
	return -5
end

function HudTimer:OnTimer ()
	self.time = self.time + 1
	self.gameManager:SetValue("time", self.time)
	Debug.Log('The time is now: ' .. self.time)

	if self.hasGameStarted then
		return
	end
	if self.time < 0 then
		return
	end
	-- set game in progress at 0:00
	self.hasGameStarted = true
	self.gameManager:SetGamePhase(GAME_PHASE_GAME)
end

function HudTimer:GetTime ()
	self.gameManager:GetValue("time")
end

function HudTimer:SetTime (newTime)
	self.time = newTime
	self.gameManager:SetValue("time", self.time)
end
