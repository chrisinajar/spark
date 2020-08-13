require "scripts.core.ability"

Consume = class(Ability)

function Consume:OnCreated ()
	self:RegisterVariable("max_range", self:GetSpecialValue("range"))
	self:RegisterVariable("cast_time", 0.0)
	self:RegisterVariable("cast_point", 0.3)
	self:RegisterVariable("range", self:GetSpecialValue("range"))
	
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.UNIT_TARGET))
	
	self.Toggled = false;
end

function Consume:OnSpellStart()
	local target = self:GetCursorTarget()
	local caster = self:GetCaster()
	
	if target:GetId() == self:GetCaster():GetId() then
		return
	end
	
	self.targetUnit = Unit({ entityId = target:GetId()})
	
	self.hiddenModifier = self.targetUnit:AddNewModifier(caster, self, "modifier_status_hidden", {duration = self:GetSpecialValue("duration")})
	self.stunModifier = self.targetUnit:AddNewModifier(caster, self, "modifier_stun", {duration = self:GetSpecialValue("duration")})
	-- change to invunurable when it is in 
	-- self.hiddenModifier = self.targetUnit:AddNewModifier(caster, self, "modifier_status_hidden", {duration = self:GetSpecialValue("duration")})
	
	self.movementModifier = caster:AddNewModifier(caster, self, "modifier_reduce_movement_speed", {duration = self:GetSpecialValue("duration")})
	if UnitRequestBus.Event.GetTeamId(target:GetId()) ~= UnitRequestBus.Event.GetTeamId(caster:GetId()) then
		self.damageModifier = self.targetUnit:AddNewModifier(caster, self, "modifier_consume", {duration = self:GetSpecialValue("duration")})
	else
		
	end
end

return Consume