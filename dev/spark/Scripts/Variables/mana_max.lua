
require "scripts.core.variable"

local mana_max = class(Variable);

function mana_max:GetValue()
	local baseMana = 75
	if self.owner:HasValue("base_mana") then
		baseMana = self.owner:GetValue("base_mana")
	end
	return baseMana + self.owner:GetValue("intelligence")*12;
end

return mana_max;
