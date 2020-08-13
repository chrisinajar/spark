require "scripts.core.variable"

local hp_regen = class(Variable);


function hp_regen:GetValue(deltaTime)
	local hp_percentage = self.owner:GetValue("hp_percentage")
	if hp_percentage < 100 and self.owner:IsAlive() then
		self.owner:SetValue("hp", hp_percentage * self.owner:GetValue("hp_max")+self.owner:GetValue("hp_regen")*deltaTime );
	end
	
	return self.owner:GetValue("strength")*0.055 + self.owner:GetValue("base_hp_regen");
end

return hp_regen;
