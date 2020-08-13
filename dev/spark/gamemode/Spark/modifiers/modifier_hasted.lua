modifier_hasted = class(Modifier)

LinkLuaModifier("modifier_hasted", modifier_hasted)

function modifier_hasted:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	--self:SetVisible(false)
end

function modifier_hasted:OnAttached()
	self:AttachVariableModifier("movement_speed");
	self.bonus_move_speed = VariableManagerRequestBus.Broadcast.GetValue(VariableId(self:GetCaster():GetId(), "max_move_speed")) - VariableManagerRequestBus.Broadcast.GetValue(VariableId(self:GetCaster():GetId(), "movement_speed"));
end

function modifier_hasted:GetModifierBonus_movement_speed()
	return self.bonus_move_speed
end