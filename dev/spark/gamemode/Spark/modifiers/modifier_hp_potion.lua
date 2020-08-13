modifier_hp_potion = class(Modifier)

LinkLuaModifier("modifier_hp_potion", modifier_hp_potion)

function modifier_hp_potion:OnCreated()

	self:AttachVariableModifier("hp_regen")
	self:RegisterVariable("regen_percentage",self:GetAbility():GetSpecialValue("regen_percentage"));
	self:RegisterVariable("hp_max_percentage",self:GetAbility():GetSpecialValue("hp_max_percentage"));
end

function modifier_hp_potion:GetModifierBonus_hp_regen()
	return self:GetValue("regen_percentage") + ((self:GetParent():GetValue("hp_max")/100) * self:GetValue("hp_max_percentage"))
end

