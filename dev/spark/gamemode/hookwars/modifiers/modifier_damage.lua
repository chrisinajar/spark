modifier_damage = class(Modifier)

LinkLuaModifier("modifier_damage", modifier_damage)

function modifier_damage:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	self:SetVisible(false)
	self:AttachVariableModifier("damage");
end

function modifier_damage:GetModifierBonus_damage()
	return self:GetAbility():GetSpecialValue("bonus_damage")
end
