
modifier_hook_range = class(Modifier)

LinkLuaModifier("modifier_hook_range", modifier_hook_range)

function modifier_hook_range:OnCreated ()
	local ability = self:GetCaster():GetAbilityInSlot(Slot(Slot.Ability, 0))
	
	self.hookModifier = self:ModifyAbility(ability, "range", function ()
		return self:GetAbility():GetSpecialValue("bonus_range")
	end)
end
