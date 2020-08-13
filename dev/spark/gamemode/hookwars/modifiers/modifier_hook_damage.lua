
modifier_hook_damage = class(Modifier)

LinkLuaModifier("modifier_hook_damage", modifier_hook_damage)

function modifier_hook_damage:OnCreated ()
	local ability = self:GetCaster():GetAbilityInSlot(Slot(Slot.Ability, 0))
	
	self.hookModifier = self:ModifyAbility(ability, "damage", function ()
		return self:GetAbility():GetSpecialValue("bonus_damage")
	end)
end
