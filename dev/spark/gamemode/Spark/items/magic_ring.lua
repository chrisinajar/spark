require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_ability_power";

MagicRing = class(Item)

function MagicRing:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function MagicRing:GetModifiers ()
	return {
		"modifier_ability_power"
	}
end

return MagicRing