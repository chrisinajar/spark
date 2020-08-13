modifier_armor = class(Modifier)

LinkLuaModifier("modifier_armor", modifier_armor)

function modifier_armor:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	self:SetVisible(false)
	self:AttachVariableModifier("armor");
end

function modifier_armor:GetModifierBonus_armor()
	return self:GetAbility():GetSpecialValue("bonus_armor")
end
