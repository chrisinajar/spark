modifier_mana = class(Modifier)

LinkLuaModifier("modifier_mana", modifier_mana)

function modifier_mana:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	self:SetVisible(false)
	self:AttachVariableModifier("mana_max");
end

function modifier_mana:GetModifierBonus_mana_max()
	return self:GetAbility():GetSpecialValue("bonus_mana")
end