modifier_agility = class(Modifier)

LinkLuaModifier("modifier_agility", modifier_agility)

function modifier_agility:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	self:SetVisible(false)
	self:AttachVariableModifier("agility");
end

function modifier_agility:GetModifierBonus_agility()
	return self:GetAbility():GetSpecialValue("bonus_agility")
end
