modifier_hp_regen = class(Modifier)

LinkLuaModifier("modifier_hp_regen", modifier_hp_regen)

function modifier_hp_regen:OnCreated()
	self:AttachVariableModifier("hp_regen")
	self:RegisterVariable("regen_amount",self:GetAbility():GetSpecialValue("regen_amount"));
end

function modifier_hp_regen:GetModifierBonus_hp_regen()
	return self:GetValue("regen_amount")
end
