modifier_aura_take_damage = class(Modifier)

LinkLuaModifier("modifier_aura_take_damage", modifier_aura_take_damage)

function modifier_aura_take_damage:OnCreated ()
	--Debug.Log('I was created? Cool!')
end

function modifier_aura_take_damage:OnAttached()
	self:AddTimer(0.5,"0.5s timer")
	self:AttachVariableModifier("movement_speed");
	if(self:GetCaster():GetId() == self:GetParent():GetId()) then
		--Debug.Log("modifier_aura_take_damage:OnAttached() is caster -> setting aura")
		self:SetAuraRadius(20)
	else
		--Debug.Log("modifier_aura_take_damage:OnAttached() is not caster -> so auras work?")
	end
end

function modifier_aura_take_damage:AddTimer(seconds, description)
	self.timerHandler = self.timerHandler or {}
	
	local ticket=TimerRequestBus.Broadcast.ScheduleTimer(seconds,description)
	local handler=TimerNotificationBus.Connect(self,ticket)
    table.insert(self.timerHandler, handler)
end

function modifier_aura_take_damage:OnTimerFinished(description)
	--Debug.Log("OnTimerFinished "..description)
			
	local damage = Damage();
	damage.type = Damage.DAMAGE_TYPE_PHYSICAL;
	damage.damage = self:GetAbility():GetSpecialValue("aura_damage") or 10;

	self:GetParent():ApplyDamage(damage)
	self:AddTimer(0.5,"0.5s timer")	
end

function modifier_aura_take_damage:GetModifierBonus_movement_speed()
	return self:GetAbility():GetSpecialValue("aura_movement_speed_reduction") or -50;
end