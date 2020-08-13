modifier_well_effect = class(Modifier)

LinkLuaModifier("modifier_well_effect", modifier_well_effect)

function modifier_well_effect:OnCreated ()
	self:AttachVariableModifier("hp_regen")
	self:AttachVariableModifier("mana_regen")
end

function modifier_well_effect:GetModifierBonus_hp_regen()
	local MaxHP = self:GetParent():GetValue("hp_max")
	return MaxHP * 0.02
end

function modifier_well_effect:GetModifierBonus_mana_regen()
	local MaxMana = self:GetParent():GetValue("mana_max")
	return MaxMana * 0.02
end