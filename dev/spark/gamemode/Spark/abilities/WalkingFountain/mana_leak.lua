require "scripts.core.ability"
require "gamemode.Spark.modifiers.modifier_aura_damage";

ManaLeak = class(Ability)

function ManaLeak:OnCreated ()
	self:RegisterVariable("max_range", self:GetSpecialValue("range"))
	self:RegisterVariable("cast_time", 0.0)
	self:RegisterVariable("cast_point", 0.3)
	self:RegisterVariable("range", self:GetSpecialValue("range"))
	
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.UNIT_TARGET))
end

function ManaLeak:OnSpellStart ()
	local target = self:GetCursorTarget()
	local caster = self:GetCaster()
	local origin = self:GetCaster():GetPosition()
	
	self.attachedUnit = Unit({ entityId = target:GetId()})
	
	if UnitRequestBus.Event.GetTeamId(target:GetId()) == UnitRequestBus.Event.GetTeamId(self:GetCaster():GetId()) then
		self.leakBuffModifier = self.attachedUnit:AddNewModifier(caster, self, "modifier_mana_leak_buff", {duration = self:GetSpecialValue("buff_duration")})
	elseif UnitRequestBus.Event.GetTeamId(target:GetId()) == UnitRequestBus.Event.GetTeamId(self:GetCaster():GetId()) and HasTag(self.attachedUnit, "hero") then
		self.leakDebuffModifier = self.attachedUnit:AddNewModifier(caster, self, "modifier_mana_leak_debuff", {duration = self:GetSpecialValue("debuff_duration")})
	end
end

return ManaLeak
