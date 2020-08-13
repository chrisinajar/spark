modifier_hp_booster = class(Modifier)

LinkLuaModifier("modifier_hp_booster", modifier_hp_booster)

function modifier_hp_booster:OnCreated ()
	Debug.Log('modifier_hp_booster:OnCreated')
	self.GetModifierBonus = self.GetModifierBonus
	self:SetVisible(false)
	self:AttachVariableModifier("hp_max");
end

function modifier_hp_booster:GetModifierBonus_hp_max()
	return self:GetAbility():GetSpecialValue("bonus_health")
end

function modifier_hp_booster:GetParticle()
	Debug.Log('modifier_hp_booster:GetParticle')
	return "hookwars.fire_ball"
end
