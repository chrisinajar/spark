require "scripts.core.ability"
require "gamemode.Spark.modifiers.modifier_aura_damage";

AuraDamage = class(Ability)

function AuraDamage:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE))
end

function AuraDamage:GetModifiers ()
	return {
		"modifier_aura_damage"
	}
end

return AuraDamage
