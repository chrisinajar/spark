require "scripts.core.ability"
require "gamemode.Spark.modifiers.modifier_status_root"
require "gamemode.Spark.modifiers.modifier_status_disarm"

InkPit = class(Ability)

function InkPit:OnCreated ()
	self:RegisterVariable("max_range", self:GetSpecialValue("range"))
	self:RegisterVariable("cast_time", 0.0)
	self:RegisterVariable("cast_point", 0.45)
	self:RegisterVariable("range", self:GetSpecialValue("range"))
	
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.UNIT_TARGET))
	
	self.UnitsNotificationsHandler = UnitsNotificationBus.Connect(self)
end

function InkPit:OnSpellStart()
	local target = self:GetCursorTarget()
	local caster = self:GetCaster()
	
	if HasTag(target,"hero") then
		self.attackSpeedModifier = target:AddNewModifier(self:GetCaster(), self, "modifier_status_root", {duration = self:GetSpecialValue("duration")})
		self.moveSpeedModifier = target:AddNewModifier(self:GetCaster(), self, "modifier_status_disarm", {duration = self:GetSpecialValue("duration")})
	elseif HasTag(target,"creep") then
		self.attackSpeedModifier = target:AddNewModifier(self:GetCaster(), self, "modifier_status_root", {duration = self:GetSpecialValue("creep_duration")})
		self.moveSpeedModifier = target:AddNewModifier(self:GetCaster(), self, "modifier_status_disarm", {duration = self:GetSpecialValue("creep_duration")})
	end
	
	local damage = Damage();
	damage.type = Damage.DAMAGE_TYPE_MAGICAL;
	damage.damage = self:GetSpecialValue("total_damage");
	damage.source = GetId(self:GetCaster())
	target:ApplyDamage(damage)

end

return InkPit