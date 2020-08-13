modifier_aura_attack_speed = class(Modifier)

LinkLuaModifier("modifier_aura_attack_speed", modifier_aura_attack_speed)

function modifier_aura_attack_speed:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	self:SetVisible(false)
	--self:AttachVariableModifier("attack_speed");
end

function modifier_aura_attack_speed:OnAttached()
	self:AttachVariableModifier("attack_speed");
	if(self:GetCaster():GetId() == self:GetParent():GetId()) then
		--Debug.Log("modifier_aura_take_damage:OnAttached() is caster -> setting aura")
		self:SetAuraRadius(20)
	else
		--Debug.Log("modifier_aura_take_damage:OnAttached() is not caster -> so auras work?")
	end
end

function modifier_aura_attack_speed:GetModifierBonus_attack_speed()
	return self:GetAbility():GetSpecialValue("bonus_aura_attack_speed")
end
