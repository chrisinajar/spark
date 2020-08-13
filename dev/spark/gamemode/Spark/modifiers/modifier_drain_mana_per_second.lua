modifier_drain_mana_per_second = class(Modifier)

LinkLuaModifier("modifier_drain_mana_per_second", modifier_drain_mana_per_second)

function modifier_drain_mana_per_second:OnCreated ()
	--self.GetModifierBonus = self.GetModifierBonus
end

function modifier_drain_mana_per_second:OnAttached()
	self:AddTimer(1.0,"1.0s timer")
	self:AttachVariableModifier("mana")
	self:SetVisible(false)
end

function modifier_drain_mana_per_second:AddTimer(seconds, description)
	self.timerHandler = self.timerHandler or {}
	
	local ticket=TimerRequestBus.Broadcast.ScheduleTimer(seconds,description)
	local handler=TimerNotificationBus.Connect(self,ticket)
    table.insert(self.timerHandler, handler)
end

function modifier_drain_mana_per_second:OnTimerFinished(description)
	local maxMana = self:GetParent():GetValue("mana_max")
	self:GetParent():Take("mana", maxMana * (self:GetAbility():GetSpecialValue("max_mana_percentage_drained_per_second")/100))
	self:AddTimer(1.0,"1.0s timer")
end