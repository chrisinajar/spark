require "scripts.core.variable"

local mana = class(Variable);

function mana:OnSet(current,proposed)
	local max = self.owner:GetValue("mana_max");
	proposed = math.max( 0 , math.min( proposed, max));
	
	self.owner:SetValue("mana_percentage", proposed/max );
	return proposed;
end

function mana:GetValue()
	return self.owner:GetValue("mana_percentage")*self.owner:GetValue("mana_max");
end

return mana;
