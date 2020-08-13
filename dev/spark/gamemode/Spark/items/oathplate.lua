require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_armor"
require "gamemode.Spark.modifiers.modifier_all_stats";

OathPlate = class(Item)

function OathPlate:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function OathPlate:GetModifiers ()
	return {
		"modifier_armor",
		"modifier_all_stats"
	}
end

return OathPlate