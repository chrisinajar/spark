require "scripts.core.ability"
require "gamemode.Spark.modifiers.modifier_solar_maximum";

SolarMaximum = class(Ability)

local Z_OFFSET = 3

function SolarMaximum:OnCreated ()
	self:RegisterVariable("max_stack_amount", self:GetSpecialValue("max_stack_amount"))
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));

end

function SolarMaximum:GetModifiers ()
	return {
		"modifier_solar_maximum"
	}
end

return SolarMaximum