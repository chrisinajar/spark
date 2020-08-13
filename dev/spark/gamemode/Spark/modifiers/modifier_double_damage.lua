modifier_double_damage = class(Modifier)

LinkLuaModifier("modifier_double_damage", modifier_double_damage)

function modifier_double_damage:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	--self:SetVisible(false)
end

function modifier_double_damage:OnAttached()
	self:AttachVariableModifier("damage");
	self.bonus_damage = VariableManagerRequestBus.Broadcast.GetValue(VariableId(self:GetCaster():GetId(), "damage"));
end

function modifier_double_damage:GetModifierBonus_damage()
	return self.bonus_damage
end