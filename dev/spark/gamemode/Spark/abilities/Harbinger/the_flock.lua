require "scripts.core.ability"
--require "gamemode.Spark.modifiers.modifier_critical_strike";

TheFlock = class(Ability)

function TheFlock:OnCreated ()
	self:RegisterVariable("cast_time", 0.0)
	self:RegisterVariable("cast_point", 0.4)
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.IMMEDIATE, CastingBehavior.PASSIVE));
end

function TheFlock:OnSpellStart()
	
end

function TheFlock:GetModifiers ()
	return {
		--"modifier_hook_damage"
	}
end

return TheFlock
