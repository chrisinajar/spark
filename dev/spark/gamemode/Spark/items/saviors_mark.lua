require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_armor"
require "gamemode.Spark.modifiers.modifier_all_stats"
require "gamemode.Spark.modifiers.modifier_mana_regen"
require "gamemode.Spark.modifiers.modifier_heal_amp"

Saviors_Mark = class(Item)

function Saviors_Mark:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function Saviors_Mark:GetModifiers ()
	return {
		"modifier_armor",
		"modifier_all_stats",
		"modifier_mana_regen",
		"modifier_heal_amp"
	}
end

return Saviors_Mark