
modifier_hook_speed = class(Modifier)

LinkLuaModifier("modifier_hook_speed", modifier_hook_speed)

function modifier_hook_speed:OnCreated ()
	local ability = self:GetCaster():GetAbilityInSlot(Slot(Slot.Ability, 0))
	
	self.hookModifier = self:ModifyAbility(ability, "speed", function ()
		return self:GetAbility():GetSpecialValue("bonus_speed")
	end)
end
