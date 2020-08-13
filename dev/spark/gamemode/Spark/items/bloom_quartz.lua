require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_hp";

BloomQuartz = class(Item)

function BloomQuartz:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function BloomQuartz:GetModifiers ()
	return {
		"modifier_hp"
	}
end

return BloomQuartz