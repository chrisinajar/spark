modifier_regen_rune = class(Modifier)

LinkLuaModifier("modifier_regen_rune", modifier_regen_rune)

function modifier_regen_rune:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	--self:SetVisible(false)
end

function modifier_regen_rune:OnAttached()
	self:AttachVariableModifier("hp_regen")
	self:AttachVariableModifier("mana_regen")
	self:ListenToAttackEvent("OnAttackLanded")
end

function modifier_critical_strike:OnAttackLanded(AttackInfo)
	if AttackInfo.attacked == self:GetParent():GetId() then
		local damage = Damage()
		if damage.damage > 0 then
			self:Destory()
		end
	end
end

function modifier_regen_rune:GetModifierBonus_hp_regen()
	local MaxHP = self:GetParent():GetValue("hp_max")
	return MaxHP * 0.2
end

function modifier_regen_rune:GetModifierBonus_mana_regen()
	local MaxMana = self:GetParent():GetValue("mana_max")
	return MaxMana * 0.2
end