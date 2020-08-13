modifier_hp_potion = class(Modifier)

LinkLuaModifier("modifier_hp_potion", modifier_hp_potion)

function modifier_hp_potion:OnCreated()

	self:AttachVariableModifier("hp_regen")
	self:RegisterVariable("regen_percentage",self:GetAbility():GetSpecialValue("regen_percentage"));
	--self:AttachVariableModifier("hp")
	--self.currentUnit = self:GetParent()
end

function modifier_hp_potion:GetModifierBonus_hp_regen()
	return self:GetValue("regen_percentage")
end
--[[
function modifier_hp_potion:GetModifierBonus_hp()
	--Add movement_speed
	local currentHpMax = self:GetParent():GetValue("hp_max")
	
	local HpIncrease = currentHpMax * self:GetAbility():GetSpecialValue("hp_max_percentage")
	
	return HpIncrease
end
--]]
