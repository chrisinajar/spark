modifier_aura_movement_speed = class(Modifier)

LinkLuaModifier("modifier_aura_movement_speed", modifier_aura_movement_speed)

function modifier_aura_movement_speed:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	--self:SetVisible(false)
end

function modifier_aura_movement_speed:OnAttached()
	self:AttachVariableModifier("movement_speed");
	if(self:GetCaster():GetId() == self:GetParent():GetId()) then	
		self:SetAuraRadius(20)
	else
	
	end
end

function modifier_aura_movement_speed:GetModifierBonus_movement_speed()
	return self:GetAbility():GetSpecialValue("bonus_aura_movement_speed")
end

