modifier_life_steal = class(Modifier)

LinkLuaModifier("modifier_life_steal", modifier_life_steal)

function modifier_life_steal:OnCreated ()
	Debug.Log("modifier_life_steal:OnCreated()");
	self.GetModifierBonus = self.GetModifierBonus
	self:SetVisible(false)
	self:AttachVariableModifier("life_steal",self:GetAbility():GetSpecialValue("bonus_life_steal"));
	self:ListenToAttackEvent("OnAttackLanded");
end


function modifier_life_steal:OnAttackLanded(AttackInfo)
	Debug.Log("modifier_life_steal:OnAttackSuccessful  AttackInfo:"..AttackInfo:ToString());
	
	if(AttackInfo.attacker==self:GetParent():GetId()  and AttackInfo.attacked~=nil )then
		local currentHp = self:GetParent():GetValue("hp")
		local damage = AttackInfo.damage
		AttackInfo.damage = 300
		local life_steal = damage * (self:GetAbility():GetSpecialValue("bonus_life_steal") /100)
		self:GetParent():SetValue("hp",currentHp+life_steal)
		Debug.Log("CURRENT HP IS:"..currentHp);
		Debug.Log("DAMAGE IS:"..damage);
		Debug.Log("LIFE STEAL TOTAL IS:"..life_steal);
		
	end
end
