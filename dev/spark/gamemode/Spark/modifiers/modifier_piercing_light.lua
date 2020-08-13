require "gamemode.Spark.modifiers.modifier_piercing_light_debuff"

modifier_piercing_light = class(Modifier)

LinkLuaModifier("modifier_piercing_light", modifier_piercing_light)

function modifier_piercing_light:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
end

function modifier_piercing_light:OnAttached ()
	self:SetVisible(true)
	--self:ListenToDamageEvent("OnDamageTaken")
	self:ListenToAttackEvent("OnAttackLanded");
end

function modifier_piercing_light:OnAttackLanded(AttackInfo)
	if AttackInfo.attacker == self:GetParent():GetId() then
		local targetUnit = Unit({ entityId =  AttackInfo.attacked})
		
		if targetUnit:FindModifierByTypeId("modifier_piercing_light_debuff") then
		
		else
			targetUnit:AddNewModifier(self:GetParent(),self:GetAbility(),"modifier_piercing_light_debuff",{duration = self:GetAbility():GetSpecialValue("debuff_duration")})
		end
	end
end