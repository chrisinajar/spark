CreepPower = CreateModule("CreepPower")

function CreepPower:Init ()
	self.gameManager = GetGameManager()
	 local maxTeamPlayerCount = 10 -- TODO: Make maxTeamPlayerCount based on values set in settings.lua (?)
	self.numPlayersXPFactor = 1 -- PlayerResource:GetTeamPlayerCount() / maxTeamPlayerCount
	self.BootGoldFactor = 1--_G.BOOT_GOLD_FACTOR
	--self.gameManager:RegisterVariable("time", self.time)

	--self.timer = CreateInterval(partial(self.OnTimer, self), 1)
end

function CreepPower:GetPowerForMinute (minute)
  if minute == 0 then
    return {   0,        1.0,      1.0,      1.0,      1.0,      1.0 * self.BootGoldFactor,      1.0 * self.numPlayersXPFactor}
  end

  return CreepPower:GetBasePowerForMinute(minute)
end

function CreepPower:GetBasePowerForMinute (minute)
  if minute == 0 then
    return {   0,        1.0,      1.0,      1.0,      1.0,      1.0 * self.BootGoldFactor,      1.0 * self.numPlayersXPFactor}
  end

  return {
    minute,                                   -- minute
    (0 * ((minute / 100) ^ 4) - 0 * ((minute/100) ^ 3) + 30 * ((minute/100) ^ 2) + 3 * (minute/100)) + 1,   -- hp
    (0 * ((minute / 100) ^ 4) - 0 * ((minute/100) ^ 3) + 30 * ((minute/100) ^ 2) + 3 * (minute/100)) + 1,   -- mana
    (0 * ((minute / 100) ^ 4) - 0 * ((minute/100) ^ 3) + 60 * ((minute/100) ^ 2) + 6 * (minute/100)) + 1,     -- damage
    (0 * (minute / 26) ^ 2 + minute / 6) + 1,       -- armor
    ((0 * minute ^ 2 + 6*4 * minute + 7*15)/(6*15)) * 2/3 * self.BootGoldFactor,                      -- gold
    ((9 * minute ^ 2 + 17 * minute + 607) / 607) * 2/3 * self.numPlayersXPFactor                      -- xp
  }
end

