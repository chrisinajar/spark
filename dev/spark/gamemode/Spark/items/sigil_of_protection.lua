require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_status_resistance"
require "gamemode.Spark.modifiers.modifier_all_stats"

Sigil_Of_Protection = class(Item)

function Sigil_Of_Protection:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function Sigil_Of_Protection:GetModifiers ()
	return {
		"modifier_status_resistance",
		"modifier_all_stats"
	}
end

return Sigil_Of_Protection