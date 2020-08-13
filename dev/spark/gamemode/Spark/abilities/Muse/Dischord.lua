require "scripts.core.ability"
require "gamemode.Spark.modifiers.modifier_stun";

Dischord = class(Ability)

function Dischord:OnCreated()
	self:RegisterVariable("cast_time", 0.0)
	self:RegisterVariable("cast_point", 0.4)
	self:RegisterVariable("max_range", self:GetSpecialValue("range"))
	self:RegisterVariable("range", self:GetSpecialValue("range"))
	self:RegisterVariable("stun_duration", self:GetSpecialValue("stun_duration"))
	
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.POINT_TARGET))
end

function Dischord:OnSpellStart()
	local target = self:GetCursorPosition()
	local caster = self:GetCaster()
	
	CreateTimer(function()
		local units=GameManagerRequestBus.Broadcast.GetUnitsInsideSphere(target , self:GetSpecialValue("radius")); -- self:GetSpecialValue("collision_radius")
		for i=1, #units do
			if UnitRequestBus.Event.GetTeamId(units[i]) ~= UnitRequestBus.Event.GetTeamId(caster:GetId()) then
				self.targetUnit = Unit({ entityId = units[i]})
				
				local damage = Damage()
				damage.type = Damage.DAMAGE_TYPE_MAGICAL
				damage.damage = self:GetSpecialValue("damage")
				damage.source = GetId(caster)
				damage.ability = self.entityId
				self.targetUnit:ApplyDamage(damage)
				
				self.stunModifier = self.targetUnit:AddNewModifier(caster, self, "modifier_stun", {duration = self:GetSpecialValue("stun_duration")})
			end
		end
		--AttachParticleSystem(thinker, "heroes.astromage.w_quasar")
	end,self:GetSpecialValue("delay"));
end

return Dischord