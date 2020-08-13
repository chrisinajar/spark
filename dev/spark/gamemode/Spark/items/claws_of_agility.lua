require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_agility";

ClawsOfAgility = class(Item)

function ClawsOfAgility:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function ClawsOfAgility:GetModifiers ()
	return {
		"modifier_agility"
	}
end

return ClawsOfAgility