modifier_aura_armor = class(Modifier)

LinkLuaModifier("modifier_aura_armor", modifier_aura_armor)

function modifier_aura_armor:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	--self:SetVisible(false)
end

function modifier_aura_armor:OnAttached()
	self:AttachVariableModifier("armor");
	if(self:GetCaster():GetId() == self:GetParent():GetId()) then	
		self:SetAuraRadius(20)
	else
	
	end
end

function modifier_aura_armor:GetModifierBonus_armor()
	return self:GetAbility():GetSpecialValue("bonus_aura_armor")
end

