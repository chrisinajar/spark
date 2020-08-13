require "scripts.core.variable"

local armor_reduction = class(Variable);

function armor_reduction:GetValue()
	local armor = self.owner:GetValue("armor")
	return 1.0 - (0.05 * armor) / (1.0 + 0.05 * math.abs(armor));
end

return armor_reduction;
