require "scripts.core.variable"

local hp = class(Variable);

function hp:OnSet(current,proposed)
	local max = self.owner:GetValue("hp_max");
	proposed = math.max( 0 , math.min( proposed, max));
	
	self.owner:SetValue("hp_percentage", proposed/max );

	if proposed == 0 then
		self.owner:Die();
	end

	return proposed;
end

function hp:GetValue()
	return self.owner:GetValue("hp_percentage")*self.owner:GetValue("hp_max");
end

return hp;
