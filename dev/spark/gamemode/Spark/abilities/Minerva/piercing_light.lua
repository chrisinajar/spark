require "scripts.core.ability"
require "gamemode.Spark.modifiers.modifier_piercing_light"

PiercingLight = class(Ability)

function PiercingLight:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function PiercingLight:GetModifiers ()
	return {
		"modifier_piercing_light"
	}
end

return PiercingLight