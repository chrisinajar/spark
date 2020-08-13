modifier_hp = class(Modifier)

LinkLuaModifier("modifier_hp", modifier_hp)

function modifier_hp:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	self:SetVisible(false)
	self:AttachVariableModifier("hp_max");
end

function modifier_hp:GetModifierBonus_hp_max()
	return self:GetAbility():GetSpecialValue("bonus_health")
end