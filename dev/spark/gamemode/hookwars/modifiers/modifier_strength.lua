modifier_strength = class(Modifier)

LinkLuaModifier("modifier_strength", modifier_strength)

function modifier_strength:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	self:SetVisible(false)
	self:AttachVariableModifier("strength");
end

function modifier_strength:GetModifierBonus_strength()
	return self:GetAbility():GetSpecialValue("bonus_strength")
end
