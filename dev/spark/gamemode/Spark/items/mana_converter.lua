require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_mana_into_health"
require "gamemode.Spark.modifiers.modifier_will"
require "gamemode.Spark.modifiers.modifier_magic_armor"
require "gamemode.Spark.modifiers.modifier_heal_amp"

Mana_Converter = class(Item)

function Mana_Converter:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function Mana_Converter:GetModifiers ()
	return {
		"modifier_mana_into_health",	
		"modifier_will",
		"modifier_magic_armor",
		"modifier_heal_amp"
	}
end

return Mana_Converter