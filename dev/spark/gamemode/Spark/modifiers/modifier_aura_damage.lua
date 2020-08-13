modifier_aura_damage = class(Modifier)

LinkLuaModifier("modifier_aura_damage", modifier_aura_damage)

function modifier_aura_damage:OnCreated ()
	--self.GetModifierBonus = self.GetModifierBonus
end

function modifier_aura_damage:OnAttached()
	
	if self:GetCaster():GetTeamId() == self:GetParent():GetTeamId() then
		self:SetVisible(true)
		self:AttachVariableModifier("damage");
	else
		self:SetVisible(false)
	end
	
	if(self:GetCaster():GetId() == self:GetParent():GetId()) then
		self:SetAuraRadius(20)
	else
	
	end
end

function modifier_aura_damage:GetModifierBonus_damage()
	local currentDamage = self:GetParent():GetValue("damage")
	return currentDamage * (self:GetAbility():GetSpecialValue("bonus_aura_damage")/100)
end
