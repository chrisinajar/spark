modifier_damage_return = class(Modifier)

LinkLuaModifier("modifier_damage_return", modifier_damage_return)

function modifier_damage_return:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	--self:SetVisible(false)
	 self:ListenToDamageEvent("OnDamageTaken")
end

function modifier_damage_return:OnDamageTaken(damage)
	if damage.damage > 0  then
		local returnDamage = damage
		returnDamage.type = damage.type;
		Debug.Log(tostring(returnDamage.type))
		returnDamage.damage = damage.damage;
		Debug.Log(tostring(returnDamage.damage))
		local source = Unit({entityId=damage.source})
		Debug.Log(tostring(source))
		source:ApplyDamage(returnDamage)
	end
end