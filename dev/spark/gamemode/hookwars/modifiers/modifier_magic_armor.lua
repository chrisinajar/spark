modifier_magic_armor = class(Modifier)

LinkLuaModifier("modifier_magic_armor", modifier_magic_armor)

function modifier_magic_armor:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	self:SetVisible(false)
	self:AttachVariableModifier("magic_armor");
end

function modifier_magic_armor:GetModifierBonus_magic_armor()
	return self:GetAbility():GetSpecialValue("bonus_magic_armor")
end
