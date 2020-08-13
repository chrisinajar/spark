modifier_movement_speed = class(Modifier)

LinkLuaModifier("modifier_movement_speed", modifier_movement_speed)

function modifier_movement_speed:OnAttached ()
	
	Debug.Log("modifier_movement_speed:OnAttached()");
	
	self:AttachVariableModifier("movement_speed");
end

function modifier_movement_speed:GetModifierBonus_movement_speed()
	return self:GetAbility():GetSpecialValue("bonus_movement_speed") or 100
end
