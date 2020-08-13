require "scripts.core.variable"

local mana_regen = class(Variable);

function mana_regen:GetValue(deltaTime)
	self.owner:SetValue("mana", self.owner:GetValue("mana")+self.owner:GetValue("mana_regen")*deltaTime );

	return self.owner:GetValue("intelligence")*0.0055 + self.owner:GetValue("base_mana_regen");
end

return mana_regen;
