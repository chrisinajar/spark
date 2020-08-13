modifier_will = class(Modifier)

LinkLuaModifier("modifier_will", modifier_will)

function modifier_will:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	self:SetVisible(false)
	self:AttachVariableModifier("will");
end

function modifier_will:GetModifierBonus_will()
	return self:GetAbility():GetSpecialValue("bonus_will")
end
