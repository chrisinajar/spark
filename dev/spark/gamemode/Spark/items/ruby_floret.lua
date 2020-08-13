require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_hp";

RubyFloret = class(Item)

function RubyFloret:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function RubyFloret:GetModifiers ()
	return {
		"modifier_hp"
	}
end

return RubyFloret