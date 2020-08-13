require "scripts.core.ability"

-- Stun Bolt ability
GlobalTeleport = class(Ability)

function GlobalTeleport:OnCreated ()
	self:RegisterVariable("max_range", self:GetSpecialValue("range"))
	self:RegisterVariable("cast_time", 0.0)
	self:RegisterVariable("cast_point", 0.3)
	self:RegisterVariable("range", self:GetSpecialValue("range"))

	self:SetCastingBehavior(CastingBehavior(CastingBehavior.POINT_TARGET));
end

function GlobalTeleport:OnSpellStart ()
	CreateTimer(function()
		local target = self:GetNearestUnits()
		local targetPosition = self:GetCursorTarget():GetPosition()
		local caster = self:GetCaster()
		local casterPosition = self:GetCaster():GetPosition()
		
		caster:SetPosition(targetPosition)
		if target ~= nil then
			target:SetPosition(targetPosition)
		end
	end,self:GetSpecialValue("channel_time"));
end

function GlobalTeleport:GetNearestUnits()
	local units=GameManagerRequestBus.Broadcast.GetUnitsInsideSphere(casterPosition, self:GetSpecialValue("aoe"))
	local effectedUnits = {}
	for i = 1, #units do
		if UnitRequestBus.Event.GetTeamId(units[i]) == UnitRequestBus.Event.GetTeamId(self:GetCaster():GetId()) then
			table.insert(effectedUnits, units[i])
		end
	end
	
	return nil
end

return GlobalTeleport
