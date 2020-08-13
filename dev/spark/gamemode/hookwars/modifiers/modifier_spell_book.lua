modifier_Spell_Book = class(Modifier)

LinkLuaModifier("modifier_Spell_Book", modifier_Spell_Book)

function modifier_Spell_Book:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	self:SetVisible(false)
	self:AttachVariableModifier("cooldown_reduction");
end

function modifier_Spell_Book:GetModifierBonus_cooldown_reduction()
	return self:GetAbility():GetSpecialValue("bonus_cooldown_reduction")/100
end