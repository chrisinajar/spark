require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_movement_speed";

BottleOfQuicksilver = class(Item)

function BottleOfQuicksilver:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function BottleOfQuicksilver:GetModifiers ()
	return {
		"modifier_movement_speed"
	}
end

return BottleOfQuicksilver
