require "scripts.core.variable"

local cooldown_timer = class(Variable);

function cooldown_timer:GetValue(deltaTime)
	local old_value=self.owner:GetValue("cooldown_timer");
	local new_value=math.max(0,old_value-deltaTime*0.5);
	
	if(old_value ~= 0 and new_value==0 ) then
		self.owner:SetValue("cooldown_current",self.owner:GetValue("cooldown"));			
	end
	
	return new_value;
end

return cooldown_timer;
