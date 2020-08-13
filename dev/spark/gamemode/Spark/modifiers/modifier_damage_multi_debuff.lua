modifier_damage_multi_debuff = class(Modifier)

LinkLuaModifier("modifier_damage_multi_debuff", modifier_damage_multi_debuff)

function modifier_damage_multi_debuff:OnAttached ()
	--self.GetModifierBonus = self.GetModifierBonus
	self:AddTimer(1.0,"1.0s timer")
	self:AddTimer(self:GetAbility():GetSpecialValue("burst_damage_interval"),tostring(self:GetAbility():GetSpecialValue("burst_damage_interval") .. " timer"))
	self:SetVisible(true)
	self.currentHp = self:GetParent():GetValue("hp")
end

function modifier_damage_multi_debuff:AddTimer(seconds, description)
	self.timerHandler = self.timerHandler or {}
	
	local ticket=TimerRequestBus.Broadcast.ScheduleTimer(seconds,description)
	local handler=TimerNotificationBus.Connect(self,ticket)
    table.insert(self.timerHandler, handler)
end

function modifier_damage_multi_debuff:OnTimerFinished(description)
	
	if description == "1.0s timer" then
		--Debug.Log("OnTimerFinished 1"..description)
		self:AddTimer(1.0,"1.0s timer")
		local damage = Damage();
		damage.source = GetId(self:GetCaster())
		damage.type = Damage.DAMAGE_TYPE_MAGICAL;
		damage.damage = self:GetAbility():GetSpecialValue("damage_per_second")
	
		self:GetParent():ApplyDamage(damage)
	elseif description == tostring(self:GetAbility():GetSpecialValue("burst_damage_interval") .. " timer") then
		--Debug.Log("OnTimerFinished 4"..description)
		local newHp = self:GetParent():GetValue("hp")
		local difference = self.currentHp - newHp
		
		local damage = Damage();
		damage.source = GetId(self:GetCaster())
		damage.type = Damage.DAMAGE_TYPE_MAGICAL;
		damage.damage = difference * (self:GetAbility():GetSpecialValue("lost_health_as_burst_damage")/100)
	
		self:GetParent():ApplyDamage(damage)
		self:AddTimer(self:GetAbility():GetSpecialValue("burst_damage_interval"),tostring(self:GetAbility():GetSpecialValue("burst_damage_interval") .. " timer"))
	end
end
