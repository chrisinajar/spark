require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_cooldown_reduction"
require "gamemode.Spark.modifiers.modifier_will"
require "gamemode.Spark.modifiers.modifier_magic_armor"

Witch_Clock = class(Item)

function Witch_Clock:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function Witch_Clock:GetModifiers ()
	return {
		"modifier_cooldown_reduction",
		"modifier_will",
		"modifier_magic_armor"
	}
end

return Witch_Clock