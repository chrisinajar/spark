modifier_aura_heal = class(Modifier)

LinkLuaModifier("modifier_aura_heal", modifier_aura_heal)

function modifier_aura_heal:OnCreated ()
	--self.GetModifierBonus = self.GetModifierBonus
	
end

function modifier_aura_heal:OnAttached()
	if self:GetCaster():GetId() == self:GetParent():GetId() or self:GetCaster():GetTeamId() == self:GetParent():GetTeamId() then
		self:AddTimer(1.0,"1.0s timer")
		self:AttachVariableModifier("hp")
		self:SetVisible(true)
	end
	if(self:GetCaster():GetId() == self:GetParent():GetId()) then
		self:SetAuraRadius(20)
	else
	
	end
end

function modifier_aura_heal:AddTimer(seconds, description)
	self.timerHandler = self.timerHandler or {}
	
	local ticket=TimerRequestBus.Broadcast.ScheduleTimer(seconds,description)
	local handler=TimerNotificationBus.Connect(self,ticket)
    table.insert(self.timerHandler, handler)
end

function modifier_aura_heal:OnTimerFinished(description)
	self:GetParent():Give("hp",self:GetAbility():GetSpecialValue("aura_heal_amount"))
	self:AddTimer(1.0,"1.0s timer")	
end


