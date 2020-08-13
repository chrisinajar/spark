modifier_mana_into_health = class(Modifier)

LinkLuaModifier("modifier_mana_into_health", modifier_mana_into_health)

function modifier_mana_into_health:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	self:SetVisible(true)
end

function modifier_mana_into_health:OnAttached()
	if(self:GetCaster():GetId() == self:GetParent():GetId()) then	
		self:SetAuraRadius(20)
	else
		self.AbilityHandler = AbilitiesNotificationBus.Connect(self)
	end
end

-- Do checks here for mana spent and add health 
function modifier_mana_into_health:OnSpellStartFilter(id)
	self.OldMana = self:GetParent():GetValue("mana")
end

function modifier_mana_into_health:OnSpellStart(id)
	local currentMana = self:GetParent():GetValue("mana")
	local manaSpent = self.OldMana - currentMana
	local healAmount = self:GetAbility():GetSpecialValue("mana_to_health_percentage") / 100
	self:GetParent():SetValue("hp",manaSpent * healAmount)
end



