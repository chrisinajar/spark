modifier_status_resistance = class(Modifier)

LinkLuaModifier("modifier_status_resistance", modifier_status_resistance)

function modifier_status_resistance:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	self:SetVisible(false)
	self:AttachVariableModifier("status_resistance");
end

function modifier_status_resistance:GetModifierBonus_life_steal()
	return self:GetAbility():GetSpecialValue("bonus_status_resistance")
end
