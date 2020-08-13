require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_damage"
require "gamemode.Spark.modifiers.modifier_movement_speed"

Windspear = class(Item)

function Windspear:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function Windspear:GetModifiers ()
	return {
		"modifier_damage",
		"modifier_movement_speed"		
	}
end

return Windspear