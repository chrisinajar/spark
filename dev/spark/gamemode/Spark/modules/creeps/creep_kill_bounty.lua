CreepKillBounty = CreateModule("CreepKillBounty")

function CreepKillBounty:Init ()
	self.gameManager = GetGameManager()
	--self.gameManager:RegisterVariable("time", self.time)

	--self.timer = CreateInterval(partial(self.OnTimer, self), 1)
end

function CreepKillBounty:HasCreepBountyMulti()

end

function CreepKillBounty:OnKilled()

end