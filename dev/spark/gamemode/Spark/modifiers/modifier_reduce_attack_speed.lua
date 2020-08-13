modifier_reduce_attack_speed = class(Modifier)

LinkLuaModifier("modifier_reduce_attack_speed", modifier_reduce_attack_speed)

function modifier_reduce_attack_speed:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	self:SetVisible(false)
	self:AttachVariableModifier("attack_speed");
end

function modifier_reduce_attack_speed:GetModifierBonus_attack_speed()
	return -self:GetAbility():GetSpecialValue("reduce_attack_speed")
end
