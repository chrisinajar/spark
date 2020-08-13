require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_ability_power"
require "gamemode.Spark.modifiers.modifier_hp"
require "gamemode.Spark.modifiers.modifier_movement_speed"

Quick_Silver_Band = class(Item)

function Quick_Silver_Band:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function Quick_Silver_Band:GetModifiers ()
	return {
		"modifier_ability_power",
		"modifier_hp",
		"modifier_movement_speed"		
	}
end

return Quick_Silver_Band