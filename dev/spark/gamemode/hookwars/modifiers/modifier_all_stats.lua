modifier_all_stats = class(Modifier)

LinkLuaModifier("modifier_all_stats", modifier_all_stats)

function modifier_all_stats:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	self:SetVisible(false)
	self:AttachVariableModifier("agility");
	self:AttachVariableModifier("intelligence");
	self:AttachVariableModifier("strength");
	self:AttachVariableModifier("will");
end

function modifier_all_stats:GetModifierBonus_agility()
	return self:GetAbility():GetSpecialValue("bonus_all_stat")
end

function modifier_all_stats:GetModifierBonus_will()
	return self:GetAbility():GetSpecialValue("bonus_all_stat")
end

function modifier_all_stats:GetModifierBonus_strength()
	return self:GetAbility():GetSpecialValue("bonus_all_stat")
end

function modifier_all_stats:GetModifierBonus_intelligence()
	return self:GetAbility():GetSpecialValue("bonus_all_stat")
end
