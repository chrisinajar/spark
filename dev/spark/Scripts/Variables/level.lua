require "scripts.core.variable"
require "gamemode.Spark.modules.hero_level_xp"

local level = class(Variable);

function ExperienceForLevel (level)
	return LevelingXP[level][2]
end

function TotalExperienceForLevel (level)
	local xp = ExperienceForLevel(level)

	return xp
end

function level:OnSet(current,proposed)
	self:LevelUp(proposed-current, current)
	return proposed
end

function level:LevelUp(levels, oldLevel)
	if levels and levels>0 then
		Debug.Log('LEVEL UP!!!!!')
		AudioRequestBus.Broadcast.PlaySound("Play_sfx_levelup");
		for i=1, levels do
			if oldLevel == 0 then
				self.owner:Give("ability_points", 1)
				break
			
			elseif LevelingXP[tonumber(i + oldLevel)][3] == "yes" then
				self.owner:Give("ability_points", 1)
			end
		end
	end
end

function level:GetValue()
	local previousLevel = self.owner:GetValue("level") or 0
	local newLevel = previousLevel
	local experience = self.owner:GetValue("experience")
	local experienceNeeded = TotalExperienceForLevel(newLevel + 1)

	while experience >= experienceNeeded do
		newLevel = newLevel + 1
		experienceNeeded = ExperienceForLevel(newLevel + 1)
	end

	if newLevel > previousLevel and newLevel ~= 1 then
		self:LevelUp(newLevel - previousLevel, previousLevel)
	end

	return newLevel
end

return level;
