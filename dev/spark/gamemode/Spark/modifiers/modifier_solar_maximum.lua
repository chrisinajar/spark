
modifier_solar_maximum = class(Modifier)

LinkLuaModifier("modifier_solar_maximum", modifier_solar_maximum)

function modifier_solar_maximum:OnCreated ()
	
	--self.GetModifierBonus = self.GetModifierBonus	
	--self.StackAmount = 0
	self:AttachVariableModifier("attack_speed");
	self:AttachVariableModifier("movement_speed");
	self:AttachVariableModifier("ability_power");
end

function modifier_solar_maximum:OnAttached()
	self:SetVisible(false)
	self:RegisterVariable("current_stacked_amount", 0)
	self.AbilityHandler = AbilitiesNotificationBus.Connect(self)
end

function modifier_solar_maximum:AddTimer(seconds, description)
	self.timerHandler = self.timerHandler or {}
	
	local ticket=TimerRequestBus.Broadcast.ScheduleTimer(seconds,description)
	local handler=TimerNotificationBus.Connect(self,ticket)
    table.insert(self.timerHandler, handler)
end

function modifier_solar_maximum:OnTimerFinished(description)
	self:SetValue("current_stacked_amount",0)
	self:SetVisible(false)
end

function modifier_solar_maximum:OnSpellStart(id)
	local ability = Ability({entityId=id})
	if ability and ability:IsItem() == false then
		local currentStack = self:GetValue("current_stacked_amount")
		if currentStack < self:GetAbility():GetSpecialValue("max_stack_amount") then
			self:SetValue("current_stacked_amount",currentStack + 1)
		else
			local maxStack = self:GetAbility():GetSpecialValue("max_stack_amount")
			self:SetValue("current_stacked_amount", maxStack)
		end
		self:SetVisible(true)
		self:AddTimer(10.0,"10.0s timer")
	end
end

function modifier_solar_maximum:GetModifierBonus_attack_speed()
	local AttackIncrease = tonumber(self:GetAbility():GetSpecialValue("bonus_attack_speed")) * self:GetValue("current_stacked_amount")
	return AttackIncrease
end

function modifier_solar_maximum:GetModifierBonus_movement_speed()
	local MovementIncrease = tonumber(self:GetAbility():GetSpecialValue("bonus_movement_speed")) * self:GetValue("current_stacked_amount") 
	return MovementIncrease
end

function modifier_solar_maximum:GetModifierBonus_ability_power() 
	local AbilityPowerIncrease = tonumber(self:GetAbility():GetSpecialValue("bonus_ability_power")) * self:GetValue("current_stacked_amount")
	return AbilityPowerIncrease
end

function modifier_solar_maximum:GetParticle()
	local particle = tostring("e_solar_maximum_" .. tostring(self:GetAbility():GetLevel()))
	return particle
end
