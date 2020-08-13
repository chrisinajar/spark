require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_damage";

Zweihander = class(Item)

function Zweihander:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function Zweihander:GetModifiers ()
	return {
		"modifier_damage"
	}
end

return Zweihander