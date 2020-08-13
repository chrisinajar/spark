modifier_movement_speed = class(Modifier)

LinkLuaModifier("modifier_movement_speed", modifier_movement_speed)

function modifier_movement_speed:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	self:SetVisible(false)
	self:AttachVariableModifier("movement_speed");
end

function modifier_movement_speed:GetModifierBonus_movement_speed()
	return self:GetAbility():GetSpecialValue("bonus_movement_speed")
end
