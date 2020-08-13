modifier_intelligence = class(Modifier)

LinkLuaModifier("modifier_intelligence", modifier_intelligence)

function modifier_intelligence:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	self:SetVisible(false)
	self:AttachVariableModifier("intelligence");
end

function modifier_intelligence:GetModifierBonus_intelligence()
	return self:GetAbility():GetSpecialValue("bonus_intelligence")
end
