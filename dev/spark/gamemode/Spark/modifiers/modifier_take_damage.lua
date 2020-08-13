modifier_take_damage = class(Modifier)

LinkLuaModifier("modifier_take_damage", modifier_take_damage)

function modifier_take_damage:OnCreated ()
	--Debug.Log('I was created? Cool!')
end

function modifier_take_damage:OnAttached()
	self:AddTimer(0.5,"0.5s timer")
	if(self:GetCaster():GetId() == self:GetParent():GetId()) then
		--Debug.Log("modifier_take_damage:OnAttached() is caster -> setting aura")
		self:SetAuraRadius(20)
	else
		--Debug.Log("modifier_take_damage:OnAttached() is not caster -> so auras work?")
	end
end

function modifier_take_damage:AddTimer(seconds, description)
	self.timerHandler = self.timerHandler or {}
	
	local ticket=TimerRequestBus.Broadcast.ScheduleTimer(seconds,description)
	local handler=TimerNotificationBus.Connect(self,ticket)
    table.insert(self.timerHandler, handler)
end

function modifier_take_damage:OnTimerFinished(description)
	--Debug.Log("OnTimerFinished "..description)
			
	local damage = Damage();
	damage.type = Damage.DAMAGE_TYPE_PHYSICAL;
	damage.damage = self:GetAbility():GetSpecialValue("damage");
	self:GetParent():ApplyDamage(damage)
	self:AddTimer(0.5,"0.5s timer")	
end