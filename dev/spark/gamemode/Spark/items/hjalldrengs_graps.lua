require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_aura_armor"
require "gamemode.Spark.modifiers.modifier_armor"
require "gamemode.Spark.modifiers.modifier_attack_speed"

Hjalldrengs_Grasp = class(Item)

function Hjalldrengs_Grasp:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function Hjalldrengs_Grasp:GetModifiers ()
	return {
		"modifier_aura_armor",
		"modifier_armor",
		"modifier_attack_speed"
	}
end

return Hjalldrengs_Grasp