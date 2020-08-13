require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_status_resistance";

ResilientBauble = class(Item)

function ResilientBauble:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function ResilientBauble:GetModifiers ()
	return {
		"modifier_status_resistance"
	}
end

return ResilientBauble