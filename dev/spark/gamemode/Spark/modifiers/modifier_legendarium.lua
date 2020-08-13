modifier_legendarium = class(Modifier)

LinkLuaModifier("modifier_legendarium", modifier_legendarium)

function modifier_legendarium:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	self:SetVisible(true)
	self:AttachVariableModifier("attack_speed");
	self:AttachVariableModifier("movement_speed");
end

function modifier_legendarium:GetModifierBonus_attack_speed()
	return -self:GetAbility():GetSpecialValue("reduce_attack_speed")
end

function modifier_reduce_movement_speed:GetModifierBonus_movement_speed()
	if self:GetAbility():GetSpecialValue("reduce_movement_speed") then
		return -self:GetAbility():GetSpecialValue("reduce_movement_speed") or 100
	elseif self:GetAbility():GetSpecialValue("movement_speed_reduction_percentage") then
		local movementSpeed = self:GetParent():GetValue("movement_speed")
		return -movementSpeed * (self:GetAbility():GetSpecialValue("movement_speed_reduction_percentage")/100)
	end	
end