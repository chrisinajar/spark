
modifier_ravenous_spirit = class(Modifier)

LinkLuaModifier("modifier_ravenous_spirit", modifier_ravenous_spirit)

function modifier_ravenous_spirit:OnCreated ()
	self:RegisterVariable("current_stacked_amount", 0)
	self:AttachVariableModifier("strength");
	self:AttachVariableModifier("magic_armor");
end

function modifier_ravenous_spirit:GetModifierBonus_strength()
	local StrengthIncrease = tonumber(self:GetAbility():GetSpecialValue("bonus_strength")) * self:GetValue("current_stacked_amount")
	return StrengthIncrease
end

function modifier_ravenous_spirit:GetModifierBonus_magic_armor() 
	return self:GetAbility():GetSpecialValue("bonus_magic_armor")
end
