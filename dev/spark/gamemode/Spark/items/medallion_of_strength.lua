require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_strength";

MedallionOfStrength = class(Item)

function MedallionOfStrength:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function MedallionOfStrength:GetModifiers ()
	return {
		"modifier_strength"
	}
end

return MedallionOfStrength