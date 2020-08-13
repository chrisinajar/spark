require "scripts.core.variable"
require "scripts.variables.experience_progress"

local experience_max = class(Variable);

function experience_max:GetValue()
	local experience = self.owner:GetValue("experience") or 0
	local level = self.owner:GetValue("level")

	return TotalExperienceForLevel(level + 1) - TotalExperienceForLevel(level)
end

return experience_max;
