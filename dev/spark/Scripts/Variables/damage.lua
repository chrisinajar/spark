require "scripts.core.variable"

local damage = class(Variable);

function damage:GetValue()
	local damageValue = self.owner:GetValue("base_damage")
	local bonusDamage = 0
	if self.owner:HasValue("attribute_primary") then
		local attribute = self.owner:GetValue("attribute_primary")
		if attribute > 0 then
			if attribute == 1 then
				bonusDamage = self.owner:GetValue("strength")
			elseif attribute == 2 then
				bonusDamage = self.owner:GetValue("agility")
			elseif attribute == 3 then
				bonusDamage = self.owner:GetValue("intelligence")
			elseif attribute == 4 then
				bonusDamage = self.owner:GetValue("will")
			end
		end
	end

	return damageValue + bonusDamage
end

return damage;
