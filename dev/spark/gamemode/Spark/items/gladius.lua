require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_damage";

Gladius = class(Item)

function Gladius:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function Gladius:GetModifiers ()
	return {
		"modifier_damage"
	}
end

return Gladius