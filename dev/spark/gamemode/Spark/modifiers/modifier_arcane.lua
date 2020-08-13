modifier_arcane = class(Modifier)

LinkLuaModifier("modifier_arcane", modifier_arcane)

function modifier_arcane:OnCreated ()
	self.GetModifierBonus = self.GetModifierBonus
	--self:SetVisible(false)
	self:AttachVariableModifier("cooldown_reduction");
end
-- need to add in mana cost reduction modifier 
function modifier_arcane:GetModifierBonus_cooldown_reduction()
	return 0.2
end