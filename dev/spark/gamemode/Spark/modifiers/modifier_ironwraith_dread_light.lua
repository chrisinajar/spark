modifier_ironwraith_dread_light = class(Modifier)

LinkLuaModifier("modifier_ironwraith_dread_light", modifier_ironwraith_dread_light)

function modifier_ironwraith_dread_light:OnAttached()
 	self.tick = 1
 	self.dpt  = self:GetAbility():GetSpecialValue("damage")
 	
 	if self:GetCaster():GetId() == self:GetParent():GetId() or self:GetCaster():GetTeamId() ~= self:GetParent():GetTeamId() then
		self:AddTimer(self.tick, tostring(self.tick))
		self:AttachVariableModifier("movement_speed");
    end
	if self:IsCaster() then
		self:SetAuraRadius(20)
	end
end

function modifier_ironwraith_dread_light:OnDetached()
	if self.timerHandler then self.timerHandler:Disconnect() end
end

function modifier_ironwraith_dread_light:AddTimer(seconds, description)
    if self.timerHandler then self.timerHandler:Disconnect() end
	local ticket=TimerRequestBus.Broadcast.ScheduleTimer(seconds,description or "")
	self.timerHandler=TimerNotificationBus.Connect(self,ticket)
end

function modifier_ironwraith_dread_light:OnTimerFinished(description)
	local damage = Damage();
	damage.source = GetId(self:GetCaster())
	damage.type = Damage.DAMAGE_TYPE_MAGICAL;
	damage.damage = self.dpt

	self:GetParent():ApplyDamage(damage)
	self:AddTimer(self.tick)	
end

function modifier_ironwraith_dread_light:GetModifierBonus_movement_speed()
	local movementSpeed = self:GetParent():GetValue("movement_speed")
	return -movementSpeed * (self:GetAbility():GetSpecialValue("movement_speed_reduction")/100)
end