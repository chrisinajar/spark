require "scripts.core.variable"
require "scripts.variables.level"

local experience_progress = class(Variable);

function experience_progress:GetValue()
	local experience = self.owner:GetValue("experience") or 0
	local level = self.owner:GetValue("level")

	local minXp = TotalExperienceForLevel(level)

	return experience - minXp
end

return experience_progress;
