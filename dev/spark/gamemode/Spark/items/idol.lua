require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_all_stats";

Idol = class(Item)

function Idol:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function Idol:GetModifiers ()
	return {
		"modifier_all_stats"
	}
end

return Idol