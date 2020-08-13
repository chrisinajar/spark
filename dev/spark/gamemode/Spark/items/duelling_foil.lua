require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_agility"
require "gamemode.Spark.modifiers.modifier_damage"
require "gamemode.Spark.modifiers.modifier_movement_speed"

Duelling_Foil = class(Item)

function Duelling_Foil:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function Duelling_Foil:GetModifiers ()
	return {
		"modifier_agility",
		"modifier_damage",
		"modifier_movement_speed"		
	}
end

return Duelling_Foil