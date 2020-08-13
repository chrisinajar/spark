require "scripts.core.ability"
require "gamemode.Spark.modifiers.modifier_attack_speed";

Flicker_Strike = class(Ability)

function Flicker_Strike:OnCreated()
	self:RegisterVariable("cast_time", 0.0)
	self:RegisterVariable("cast_point", 0.1)
	self:RegisterVariable("max_range", self:GetSpecialValue("range"))
	self:RegisterVariable("range", self:GetSpecialValue("range"))
	
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.UNIT_TARGET))
end

function Flicker_Strike:OnSpellStart()
	--AudioRequestBus.Broadcast.PlaySound("Play_sfx_blink_vanish");
	local target = self:GetCursorTarget()
	local caster = self:GetCaster()	
	local origin = self:GetCaster():GetPosition()
	local casterTeam = UnitRequestBus.Event.GetTeamId(caster:GetId())
	local targetTeam = UnitRequestBus.Event.GetTeamId(target:GetId())
	
	caster:SetPosition(target:GetPosition())
	AudioRequestBus.Broadcast.PlaySound("Play_sfx_blink_appear");	
	
	-- Check here for if enemy
	if casterTeam ~= targetTeam then
		self.attackSpeedModifier = caster:AddNewModifier(caster, self, "modifier_attack_speed", {duration = self:GetSpecialValue("attack_speed_duration")})
	end
end

return Flicker_Strike
