modifier_empyrean = class(Modifier)

LinkLuaModifier("modifier_empyrean", modifier_empyrean)

function modifier_empyrean:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
end

function modifier_empyrean:OnAttached ()
	self:SetVisible(true)
	--self:ListenToDamageEvent("OnDamageTaken")
	self:ListenToAttackEvent("OnAttackLanded");
end

function modifier_empyrean:OnAttackLanded(AttackInfo)
	if AttackInfo.attacked==self:GetParent():GetId() then
		Debug.Log("ON ATTACK LANDED AND ULT TRIGGERED !")
		local damage = AttackInfo.damage
		
		self:GetParent():Give("hp",(damage * (self:GetAbility():GetSpecialValue("damage_to_health_percentage")/100)))
		return false
	
	end
end