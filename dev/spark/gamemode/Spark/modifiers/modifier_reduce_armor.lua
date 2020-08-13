modifier_reduce_armor = class(Modifier)

LinkLuaModifier("modifier_reduce_armor", modifier_reduce_armor)

function modifier_reduce_armor:OnAttached ()
	self.GetModifierBonus = self.GetModifierBonus
	self:SetVisible(false)
	self:AttachVariableModifier("armor");
end

function modifier_reduce_armor:GetModifierBonus_armor()
	return -self:GetAbility():GetSpecialValue("reduce_armor")
end
