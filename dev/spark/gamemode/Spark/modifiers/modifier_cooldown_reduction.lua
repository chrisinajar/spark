modifier_cooldown_reduction = class(Modifier)

LinkLuaModifier("modifier_cooldown_reduction", modifier_cooldown_reduction)

function modifier_cooldown_reduction:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	self:SetVisible(false)
	self:AttachVariableModifier("cooldown_reduction");
end

function modifier_cooldown_reduction:GetModifierBonus_cooldown_reduction()
	return self:GetAbility():GetSpecialValue("bonus_cooldown_reduction")/100
end