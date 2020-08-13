require "scripts.core.ability"
require "gamemode.Spark.modifiers.modifier_reduce_attack_speed";
require "gamemode.Spark.modifiers.modifier_reduce_movement_speed";

RunicBurst = class(Ability)

function RunicBurst:OnCreated ()
	self:RegisterVariable("max_range", self:GetSpecialValue("range"))
	self:RegisterVariable("cast_time", 0.0)
	self:RegisterVariable("cast_point", 0.3)
	self:RegisterVariable("range", self:GetSpecialValue("range"))
	
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.POINT_TARGET))
	
	self.UnitsNotificationsHandler = UnitsNotificationBus.Connect(self)
end

function RunicBurst:OnSpellStart()
	local target = self:GetCursorPosition()
	local caster = self:GetCaster()
	
	local units=GameManagerRequestBus.Broadcast.GetUnitsInsideSphere(self:GetCursorPosition(), self:GetSpecialValue("aoe"))
	for i = 1, #units do
		if UnitRequestBus.Event.GetTeamId(units[i]) ~= UnitRequestBus.Event.GetTeamId(self:GetCaster():GetId()) then
			self.targetUnit = Unit({ entityId = units[i]})
		
			self.attackSpeedModifier = self.targetUnit:AddNewModifier(self:GetCaster(), self, "modifier_reduce_attack_speed", {duration = self:GetSpecialValue("debuff_duration")})
			self.moveSpeedModifier = self.targetUnit:AddNewModifier(self:GetCaster(), self, "modifier_reduce_movement_speed", {duration = self:GetSpecialValue("debuff_duration")})
			
			local damage = Damage();
			damage.type = Damage.DAMAGE_TYPE_MAGICAL;
			damage.damage = self:GetSpecialValue("damage");
			damage.source = GetId(self:GetCaster())
			self.targetUnit:ApplyDamage(damage)
		end
	end
end

return RunicBurst