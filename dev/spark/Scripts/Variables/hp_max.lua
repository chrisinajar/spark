
require "scripts.core.variable"

local hp_max = class(Variable);

function hp_max:GetValue()
	local baseHp = 200
	if self.owner:HasValue("base_hp") then
		baseHp = self.owner:GetValue("base_hp")
	end
	return baseHp + self.owner:GetValue("strength")*18;
end

return hp_max;
