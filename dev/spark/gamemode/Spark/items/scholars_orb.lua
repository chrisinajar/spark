require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_ability_power"
require "gamemode.Spark.modifiers.modifier_intelligence"
require "gamemode.Spark.modifiers.modifier_mana_regen"

Scholars_Orb = class(Item)

function Scholars_Orb:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function Scholars_Orb:GetModifiers ()
	return {
		"modifier_ability_power",
		"modifier_intelligence",
		"modifier_mana_regen"
	}
end

return Scholars_Orb