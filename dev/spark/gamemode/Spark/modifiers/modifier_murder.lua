modifier_murder = class(Modifier)

LinkLuaModifier("modifier_murder", modifier_murder)

function modifier_murder:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	self:SetVisible(true)
end

function modifier_murder:OnAttached()
	self:ListenToAttackEvent("OnAttackStart")
	self:ListenToAttackEvent("OnAttackLanded")
end

function modifier_murder:OnAttackStart(AttackInfo)
	self.SuccessfulCrit = true
end

function modifier_murder:OnAttackLanded(AttackInfo)
	if(AttackInfo.attacker == self:GetParent():GetId() and AttackInfo.attacked ~= nil )then
		local targetUnit = Unit({ entityId =  AttackInfo.attacked})
		if self.SuccessfulCrit == true then
			local damage = Damage()
			local initialDamage = AttackInfo.damage
			damage.type = Damage.DAMAGE_TYPE_PHYSICAL;
			damage.flag = bor(damage.flag, Damage.DAMAGE_FLAG_CRITICAL)
			local amount = self:GetAbility():GetSpecialValue("crit_percentage")
			damage.damage = initialDamage * (tonumber(amount)/100)
		end
		targetUnit:AddNewModifier(self:GetParent(), self:GetAbility(), "modifier_reduce_movement_speed", {duration = self:GetAbility():GetSpecialValue("slow_duration")})
		self:Destroy()
	end
	
end
