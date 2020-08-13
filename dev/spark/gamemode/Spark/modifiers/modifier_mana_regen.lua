modifier_mana_regen = class(Modifier)

LinkLuaModifier("modifier_mana_regen", modifier_mana_regen)

function modifier_mana_regen:OnAttached()
	self:AttachVariableModifier("mana_regen")
	self:RegisterVariable("regen_amount",self:GetAbility():GetSpecialValue("regen_amount"));
end

function modifier_mana_regen:GetModifierBonus_mana_regen()
	return self:GetValue("regen_amount")
end
