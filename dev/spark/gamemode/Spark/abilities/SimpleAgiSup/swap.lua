require "scripts.core.ability"

-- Stun Bolt ability
Swap = class(Ability)

function Swap:OnCreated ()
	self:RegisterVariable("max_range", self:GetSpecialValue("range"))
	self:RegisterVariable("cast_time", 0.0)
	self:RegisterVariable("cast_point", 0.3)
	self:RegisterVariable("range", self:GetSpecialValue("range"))

	self:SetCastingBehavior(CastingBehavior(CastingBehavior.UNIT_TARGET));
end

function Swap:OnSpellStart ()
	local target = self:GetCursorTarget()
	local targetPosition = self:GetCursorTarget():GetPosition()
	local caster = self:GetCaster()
	local casterPosition = self:GetCaster():GetPosition()
	
	caster:SetPosition(targetPosition)
	target:SetPosition(casterPosition)
end


return Swap
