modifier_ability_power = class(Modifier)

LinkLuaModifier("modifier_ability_power", modifier_ability_power)

function modifier_ability_power:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	self:SetVisible(false)
	self:AttachVariableModifier("ability_power");
end

function modifier_ability_power:GetModifierBonus_ability_power()
	return self:GetAbility():GetSpecialValue("bonus_ability_power")
end
