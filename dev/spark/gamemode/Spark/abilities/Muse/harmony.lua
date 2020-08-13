require "scripts.core.ability"
require "gamemode.Spark.modifiers.modifier_reduce_cooldown";

Harmony = class(Ability)

function Harmony:OnAttached()
	self:RegisterVariable("cast_time", 0.0)
	self:RegisterVariable("cast_point", 0.2)
	self:RegisterVariable("max_range", self:GetSpecialValue("range"))
	self:RegisterVariable("range", self:GetSpecialValue("range"))
	
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.UNIT_TARGET))
end

function Harmony:OnSpellStart()
	local target = self:GetCursorTarget()
	
	target:Give("hp", self:GetSpecialValue("heal_amount"))
end

function Harmony:GetModifiers ()
	return {
		"modifier_reduce_cooldown"
	}
end

return Harmony
