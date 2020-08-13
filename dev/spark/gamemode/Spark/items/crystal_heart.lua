require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_hp"
require "gamemode.Spark.modifiers.modifier_strength"
require "gamemode.Spark.modifiers.modifier_hp_regen"

Crystal_Heart = class(Item)

function Crystal_Heart:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function Crystal_Heart:GetModifiers ()
	return {
		"modifier_hp",
		"modifier_strength",
		"modifier_hp_regen"
	}
end

return Crystal_Heart