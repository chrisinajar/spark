require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_armor";

ArmoredBelt = class(Item)

function ArmoredBelt:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function ArmoredBelt:GetModifiers ()
	return {
		"modifier_armor"
	}
end

return ArmoredBelt