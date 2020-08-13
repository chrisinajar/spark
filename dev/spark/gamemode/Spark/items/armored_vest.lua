require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_armor";

ArmoredVest = class(Item)

function ArmoredVest:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function ArmoredVest:GetModifiers ()
	return {
		"modifier_armor"
	}
end

return ArmoredVest