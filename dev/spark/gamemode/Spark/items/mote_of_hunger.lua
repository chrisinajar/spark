require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_life_steal";

MoteOfHunger = class(Item)

function MoteOfHunger:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function MoteOfHunger:GetModifiers ()
	return {
		"modifier_life_steal"
	}
end

return MoteOfHunger