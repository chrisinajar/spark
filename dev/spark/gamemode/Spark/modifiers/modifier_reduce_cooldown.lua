modifier_reduce_cooldown = class(Modifier)

LinkLuaModifier("modifier_reduce_cooldown", modifier_reduce_cooldown)

function modifier_reduce_cooldown:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	self:SetVisible(false)
	--self:AttachVariableModifier("cooldown_reduction");
	self:ListenToAttackEvent("OnAttackLanded");
end

function modifier_reduce_cooldown:OnAttackLanded(AttackInfo)
	Debug.Log("modifier_reduce_cooldown:OnAttackSuccessful  AttackInfo:"..AttackInfo:ToString());
	
	if AttackInfo.attacker == self:GetParent():GetId() and AttackInfo.attacked ~= nil then
		local currentCoolDown = self:GetAbility():GetValue("cooldown_timer")

		self:GetAbility():SetValue("cooldown_timer",currentCoolDown - self:GetAbility():GetSpecialValue("cooldown_reduction"))
	end
end