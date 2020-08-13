require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_movement_speed";

Boots = class(Item)

function Boots:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function Boots:GetModifiers ()
	return {
		"modifier_movement_speed"
	}
end

return Boots
