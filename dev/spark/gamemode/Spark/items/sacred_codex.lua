require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_heal_amp"
require "gamemode.Spark.modifiers.modifier_all_stats"
require "gamemode.Spark.modifiers.modifier_mana_regen"
require "gamemode.Spark.modifiers.modifier_armor"
require "gamemode.Spark.modifiers.modifier_cooldown_reduction"

Sacred_Codex = class(Item)

function Sacred_Codex:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
	-- need to add talent stuff when in here !
end

function Sacred_Codex:GetModifiers ()
	return {
		"modifier_heal_amp",
		"modifier_all_stats",
		"modifier_mana_regen",
		"modifier_armor",
		"modifier_cooldown_reduction"
	}
end

return Sacred_Codex