require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_intelligence";

WandOfIntelligence = class(Item)

function WandOfIntelligence:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function WandOfIntelligence:GetModifiers ()
	return {
		"modifier_intelligence"
	}
end

return WandOfIntelligence