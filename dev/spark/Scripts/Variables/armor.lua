require "scripts.core.variable"

local armor = class(Variable);

function armor:GetValue()
	return self.owner:GetValue("base_armor")+self.owner:GetValue("agility")*(1 / 6.25);
end

return armor;
