modifier_shadow_serenade = class(Modifier)

LinkLuaModifier("modifier_shadow_serenade", modifier_shadow_serenade)

function modifier_shadow_serenade:OnCreated ()
	self:AttachVariableModifier("movement_speed")
end

function modifier_shadow_serenade:OnAttached()
	self:SetVisible(true)
	self:ListenToAttackEvent("OnAttackLanded")
	self.AbilityHandler = AbilitiesNotificationBus.Connect(self)
end

function modifier_shadow_serenade:GetModifierBonus_movement_speed()
	return self:GetAbility():GetSpecialValue("bonus_movement_speed")
end

function modifier_shadow_serenade:OnAttackLanded(AttackInfo)
	-- add in attack speed and movement modifiers
	self:Destroy()
end

function modifier_shadow_serenade:OnSpellStart(id)
	-- add in attack speed and movement modifiers
	self:Destroy()
end