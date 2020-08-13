require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_hp_regen";

RedPearl = class(Item)

function RedPearl:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function RedPearl:GetModifiers ()
	return {
		"modifier_hp_regen"
	}
end

return RedPearl