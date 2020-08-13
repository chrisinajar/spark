modifier_heal_amp = class(Modifier)

LinkLuaModifier("modifier_heal_amp", modifier_heal_amp)

function modifier_heal_amp:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	self:SetVisible(false)
	self:AttachVariableModifier("heal_amp");
end

function modifier_heal_amp:GetModifierBonus_heal_amp()
	return self:GetAbility():GetSpecialValue("bonus_heal_amp")
end
