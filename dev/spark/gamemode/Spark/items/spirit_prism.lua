require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_hp_amp"
require "gamemode.Spark.modifiers.modifier_will"
require "gamemode.Spark.modifiers.modifier_magic_armor"

Spirit_Prism = class(Item)

function Spirit_Prism:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.POINT_TARGET,CastingBehavior.PASSIVE));
end

function Spirit_Prism:GetModifiers ()
	return {
		"modifier_hp_amp",
		"modifier_will",
		"modifier_magic_armor"
	}
end

return Spirit_Prism