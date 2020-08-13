modifier_spell_life_steal = class(Modifier)

LinkLuaModifier("modifier_spell_life_steal", modifier_spell_life_steal)

function modifier_spell_life_steal:OnCreated ()
	
	--self:AttachVariableModifier("life_steal",self:GetAbility():GetSpecialValue("bonus_life_steal"));
end

function modifier_spell_life_steal:OnAttached()
	--self:SetVisible(false)
	--self.AbilityHandler = AbilitiesNotificationBus.Connect(self)
	Debug.Log("modifier_spell_life_steal:OnCreated()");
	self.GetModifierBonus = self.GetModifierBonus
	self:ListenToEvent("OnDamageDealt");
end

function modifier_spell_life_steal:OnDamageDealt(damage,hp_removed)
	Debug.Log("modifier_solar_maximum:OnDamageDealt "..tostring(damage:ToString()))

	local parent = self:GetParent()

	if damage.ability and damage.ability:IsValid() and parent:HasAbility(damage.ability) then
		--spell lifesteal
		parent:Give("hp",hp_removed*0.25)
	end
end
