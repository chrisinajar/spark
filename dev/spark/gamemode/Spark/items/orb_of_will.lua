require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_will";

OrbOfWill = class(Item)

function OrbOfWill:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function OrbOfWill:GetModifiers ()
	return {
		"modifier_will"
	}
end

return OrbOfWill