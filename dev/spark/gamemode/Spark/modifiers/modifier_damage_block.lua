modifier_damage_block = class(Modifier)

LinkLuaModifier("modifier_damage_block", modifier_damage_block)

function modifier_damage_block:OnCreated ()
	Debug.Log("modifier_damage_block:OnCreated()");
	self.GetModifierBonus = self.GetModifierBonus
end

function modifier_damage_block:OnAttached()
	self:ListenToAttackEvent("OnAttackLanded");
	--self:ListenToAttackEvent("OnDamageTaken");
	self:SetVisible(true)
	
	self:RegisterVariable("damage_block", self:GetAbility():GetSpecialValue("damage_block"))
	self:RegisterVariable("explode_damage", self:GetAbility():GetSpecialValue("explode_damage"))
	self:RegisterVariable("shield_health_percentage", self:GetAbility():GetSpecialValue("shield_health_percentage"))
end

function modifier_damage_block:OnAttackLanded(AttackInfo)
	Debug.Log("modifier_damage_block:OnAttackSuccessful  AttackInfo:"..AttackInfo:ToString());
	
	if AttackInfo.attacked==self:GetParent():GetId() then
		-- DO DAMAGE BLOCK		
		if AttackInfo.damage <= self:GetValue("damage_block") then
			self:SetValue("damage_block",(self:GetValue("damage_block") - AttackInfo.damage))
			return false
		else
			-- do explosion
			--Debug.Log("PARENT POS IS : " .. tostring(self:GetParent():GetPosition()))
			--Debug.Log("EXPLODE AOE IS : " .. tostring(self:GetAbility():GetSpecialValue("explode_aoe")))
			CreateTimer(function()
			local units=GameManagerRequestBus.Broadcast.GetUnitsInsideSphere(self:GetParent():GetPosition(), self:GetAbility():GetSpecialValue("explode_aoe"))
				for i=1, #units do
					if UnitRequestBus.Event.GetTeamId(units[i]) ~= UnitRequestBus.Event.GetTeamId(self:GetCaster()) then
						self.targetUnit = Unit({ entityId = units[i]})
						
						local damage = Damage()
						damage.type = Damage.DAMAGE_TYPE_MAGICAL
						damage.damage = self:GetAbility():GetSpecialValue("explode_damage")
						damage.source = self:GetCaster():GetId()
						damage.ability = self.entityId
						self.targetUnit:ApplyDamage(damage)
					end
				end
				--AttachParticleSystem(thinker, "heroes.astromage.w_quasar")
				self:Destroy()
			end,0.1);
		end
	end
end

function modifier_damage_block:GetParticle()
	return "heroes.minerva.W_Consencrate"
end