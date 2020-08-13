modifier_attack_speed = class(Modifier)

LinkLuaModifier("modifier_attack_speed", modifier_attack_speed)

function modifier_attack_speed:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	self:SetVisible(false)
	self:AttachVariableModifier("attack_speed");
end

function modifier_attack_speed:GetModifierBonus_attack_speed()
	return self:GetAbility():GetSpecialValue("bonus_attack_speed")
end
