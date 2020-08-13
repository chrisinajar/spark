require "scripts.modifier";

modifier_mana_leak_buff = class(Modifier)

LinkLuaModifier("modifier_mana_leak_buff", modifier_mana_leak_buff)

function modifier_mana_leak_buff:OnCreated ()
	Debug.Log('I was created? Cool!')
	self:SetVisible(true)
	self:AddTimer(1.0,"1.0s timer")
end

function modifier_mana_leak_buff:AddTimer(seconds, description)
	self.timerHandler = self.timerHandler or {}
	
	local ticket=TimerRequestBus.Broadcast.ScheduleTimer(seconds,description)
	local handler=TimerNotificationBus.Connect(self,ticket)
    table.insert(self.timerHandler, handler)
end

function modifier_mana_leak_buff:OnTimerFinished(description)
	local manaPercentagePerSecond = (self:GetParent():GetValue("mana_max")/100) * self:GetAbility():GetSpecialValue("ally_percentage_max_mana_gained_per_second")
	self:GetParent():Give("mana", self:GetAbility():GetSpecialValue("ally_base_mana_gain_per_second") + manaPercentagePerSecond)
end

return modifier_mana_leak_buff