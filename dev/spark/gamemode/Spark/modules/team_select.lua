
TeamSelection = CreateModule("TeamSelection", GAME_PHASE_CONFIGURE_GAME )

function TeamSelection:Init ()
	self.gameManager = GetGameManager()
	self.gameManager:RegisterVariable("TeamSelectTime", self.time)
	self.time = self:GetStartTime()

	self.timer = CreateInterval(partial(self.OnTimer, self), 1)

	self:ListenToUIEvent("LockInAndStart")
	self:ListenToUIEvent("AddBot")
	self:ListenToUIEvent("ShuffleTeams")
	self:ListenToUIEvent("KickFromTeam")
end

function TeamSelection:GetStartTime ()
	return 30
end

function TeamSelection:GetLockInTime ()
	return 3
end

function TeamSelection:OnTimer ()
	self.time = self.time - 1
	self.gameManager:SetValue("TeamSelectTime", self.time)
	Debug.Log('(team select) The time is now: ' .. self.time)
	if self.time > 0 then
		return
	end

	if self.timer then
		self.timer()
		self.timer = nil
	end

	GameManagerRequestBus.Broadcast.FinishGameConfigure()
end

function TeamSelection:GetTime ()
	self.gameManager:GetValue("TeamSelectTime")
end

function TeamSelection:SetTime (newTime)
	self.time = newTime
	self.gameManager:SetValue("TeamSelectTime", self.time)
end

function TeamSelection:LockInAndStart (data)
	self:SetTime(self:GetLockInTime())
	self.locked = true
end

function TeamSelection:AddBot (data)
	self.gameManager:CreateFakePlayer()
end

function TeamSelection:ShuffleTeams (data)
end

function TeamSelection:KickFromTeam (data)
end

function TeamSelection:SlotPressed (data)
end

function TeamSelection:BenchSlotPressed (data)
end
