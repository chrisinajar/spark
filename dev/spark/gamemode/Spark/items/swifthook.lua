require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_attack_speed";

Swifthook = class(Item)

function Swifthook:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function Swifthook:GetModifiers ()
	return {
		"modifier_attack_speed"
	}
end

return Swifthook