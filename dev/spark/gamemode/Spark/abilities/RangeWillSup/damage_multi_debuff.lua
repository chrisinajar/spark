require "scripts.core.ability"
require "gamemode.Spark.modifiers.modifier_damage_multi_debuff";

-- Stun Bolt ability
DamageMultiDebuff = class(Ability)

local Z_OFFSET = 3

function DamageMultiDebuff:OnCreated ()
	self:RegisterVariable("max_range", self:GetSpecialValue("range"))
	self:RegisterVariable("cast_time", 0.0)
	self:RegisterVariable("cast_point", 0.35)
	self:RegisterVariable("range", self:GetSpecialValue("range"))

	self:SetCastingBehavior(CastingBehavior(CastingBehavior.POINT_TARGET));
end

function DamageMultiDebuff:OnSpellStart ()
	--AudioRequestBus.Broadcast.PlaySound("Play_sfx_hook_throw");
	local target = self:GetCursorPosition()
	local caster = self:GetCaster()
	
	local units=GameManagerRequestBus.Broadcast.GetUnitsInsideSphere(self:GetCursorPosition(), self:GetSpecialValue("aoe"))
	if #units > 0 then
		for i = 1, #units do
			if UnitRequestBus.Event.GetTeamId(units[i]) ~= UnitRequestBus.Event.GetTeamId(self:GetCaster():GetId()) and units[i] ~= self:GetCaster():GetId() then
				self.targetUnit = Unit({ entityId = units[i]})
				
				self.Modifier = self.targetUnit:AddNewModifier(self:GetCaster(), self, "modifier_damage_multi_debuff", {duration = self:GetSpecialValue("debuff_duration")})
			end
		end
	end
end

return DamageMultiDebuff
