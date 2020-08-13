modifier_piercing_light_buff = class(Modifier)

LinkLuaModifier("modifier_piercing_light_buff", modifier_piercing_light_buff)

function modifier_piercing_light_buff:OnAttached ()
	--self.GetModifierBonus = self.GetModifierBonus
	self:SetVisible(true)
	self:AttachVariableModifier("attack_speed");
	self:AttachVariableModifier("movement_speed");
end

function modifier_piercing_light_buff:GetModifierBonus_attack_speed()
	return self:GetAbility():GetSpecialValue("bonus_attack_speed")
end

function modifier_piercing_light_buff:GetModifierBonus_movement_speed()
	return self:GetAbility():GetSpecialValue("bonus_movement_speed")
end