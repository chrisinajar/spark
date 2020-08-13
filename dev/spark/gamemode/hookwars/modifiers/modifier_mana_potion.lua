require "scripts.modifier"

modifier_mana_potion = class(Modifier)

LinkLuaModifier("modifier_mana_potion", modifier_mana_potion)

function modifier_mana_potion:OnCreated()

	self:AttachVariableModifier("mana_regen")
	self:RegisterVariable("regen_percentage",self:GetAbility():GetSpecialValue("regen_percentage"));
	--self:AttachVariableModifier("mana")
	--self.currentUnit = self:GetParent()
end

function modifier_mana_potion:GetModifierBonus_mana_regen()
	return self:GetValue("regen_percentage")--self:GetAbility():GetSpecialValue("regen_percentage")
end

--[[
function modifier_mana_potion:GetModifierBonus_mana()
	local currentManaMax = self:GetParent():GetValue("mana_max")
	
	local ManaIncrease = currentManaMax * self:GetAbility():GetSpecialValue("mana_max_percentage")
	
	return ManaIncrease
end
--]]
