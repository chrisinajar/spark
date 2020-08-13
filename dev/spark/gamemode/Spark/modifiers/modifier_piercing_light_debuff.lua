modifier_piercing_light_debuff = class(Modifier)

LinkLuaModifier("modifier_piercing_light_debuff", modifier_piercing_light_debuff)

function modifier_piercing_light_debuff:OnAttached ()
	--self.GetModifierBonus = self.GetModifierBonus
	self:SetVisible(true)
	self:AttachVariableModifier("attack_speed");
	self:AttachVariableModifier("movement_speed");
	self:ListenToAttackEvent("OnAttackLanded");
end

function modifier_piercing_light_debuff:GetModifierBonus_attack_speed()
	return -self:GetAbility():GetSpecialValue("reduce_attack_speed")
end

function modifier_piercing_light_debuff:GetModifierBonus_movement_speed()
	return -self:GetAbility():GetSpecialValue("reduce_movement_speed")
end

function modifier_piercing_light_debuff:OnAttackLanded(AttackInfo)
	if AttackInfo.attacked == self:GetParent():GetId() then
		local targetUnit = Unit({ entityId =  AttackInfo.attacker})
		
		if UnitRequestBus.Event.GetTeamId(AttackInfo.attacker) == self:GetCaster():GetTeamId() then		
			targetUnit:AddNewModifier(self:GetCaster(),self:GetAbility(),"modifier_piercing_light_buff",{duration = self:GetAbility():GetSpecialValue("duration")})
		end
	end
end