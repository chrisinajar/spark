modifier_consume = class(Modifier)

LinkLuaModifier("modifier_consume", modifier_consume)

function modifier_consume:OnCreated ()
	--Debug.Log('I was created? Cool!')
end

function modifier_consume:OnAttached()
	self:AddTimer(1.0,"1.0s timer")
end

function modifier_consume:AddTimer(seconds, description)
	self.timerHandler = self.timerHandler or {}
	
	local ticket=TimerRequestBus.Broadcast.ScheduleTimer(seconds,description)
	local handler=TimerNotificationBus.Connect(self,ticket)
    table.insert(self.timerHandler, handler)
end

function modifier_consume:OnTimerFinished(description)
			
	local damage = Damage();
	damage.type = Damage.DAMAGE_TYPE_PHYSICAL;
	damage.damage = self:GetAbility():GetSpecialValue("damage") + (self:GetAbility():GetSpecialValue("damage") * self:GetAbility():GetSpecialValue("bonus_damage_strength_multiplier")) ;
	self:GetParent():ApplyDamage(damage)
	self:AddTimer(1.0,"1.0s timer")	
end