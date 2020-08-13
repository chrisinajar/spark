require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_mana_regen";

ManaSignet = class(Item)

function ManaSignet:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function ManaSignet:GetModifiers ()
	return {
		"modifier_mana_regen"
	}
end

return ManaSignet