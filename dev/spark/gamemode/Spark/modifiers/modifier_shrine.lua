modifier_shrine = class(Modifier)

LinkLuaModifier("modifier_shrine", modifier_shrine)

function modifier_shrine:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	--self:SetVisible(false)
end

function modifier_shrine:OnAttached()
	self:AttachVariableModifier("hp");
	if(self:GetCaster():GetId() == self:GetParent():GetId()) then
		self:SetAuraRadius(20)		
	else
		--self.NewHPRegen = self:GetParent():GetValue("hp_regen") + 100
		--self.NewManaRegen = self:GetParent():GetValue("mana_regen") + 100
		self:AttachVariableModifier("hp_regen")
		self:AttachVariableModifier("mana_regen")
	end
end

function modifier_shrine:GetModifierBonus_hp_regen()
	--local HPRegen = self:GetParent():GetValue("hp_regen")
	return 100
end

function modifier_shrine:GetModifierBonus_mana_regen()
	--local ManaRegen = self:GetParent():GetValue("mana_regen")
	return 100
end


