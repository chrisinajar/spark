require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_heal_amp";

FireFlax = class(Item)

function FireFlax:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function FireFlax:GetModifiers ()
	return {
		"modifier_heal_amp"
	}
end

return FireFlax