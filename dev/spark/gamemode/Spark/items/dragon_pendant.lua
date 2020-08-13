require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_magic_armor"
require "gamemode.Spark.modifiers.modifier_strength"
require "gamemode.Spark.modifiers.modifier_will"
require "gamemode.Spark.modifiers.modifier_hp_regen"

Dragon_Pendant = class(Item)

function Dragon_Pendant:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function Dragon_Pendant:GetModifiers ()
	return {
		"modifier_magic_armor",
		"modifier_strength",
		"modifier_will",
		"modifier_hp_regen"
	}
end

return Dragon_Pendant