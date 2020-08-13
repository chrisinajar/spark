require "scripts.modifier";

modifier_mana_leak_debuff = class(Modifier)

LinkLuaModifier("modifier_mana_leak_debuff", modifier_mana_leak_debuff)

function modifier_mana_leak_debuff:OnCreated ()
	Debug.Log('I was created? Cool!')
	self:SetVisible(true)
	self.distancedMoved = 0
end

function modifier_mana_leak_debuff:OnAttached()
	self.navNotificationHandler = NavigationEntityNotificationBus.Connect(self, self:GetParent():GetId())
end

function modifier_mana_leak_debuff:OnPositionChanged(previous, current)
	Debug.Log("UNIT IS MOVING !!!" .. tostring(previous) .. " TO " .. tostring(current))
	Debug.Log("DISTANCE MOVED IS : " .. tostring(Distance2D(previous, current)))
	self.distancedMoved = self.distancedMoved + Distance2D(previous, current)
	if self.distancedMoved >= 1 then
		self.distancedMoved = 0
		self:GetParent():Take("mana", (self:GetParent():GetValue("mana_max")/100) * self:GetAbility():GetSpecialValue("max_mana_lost_percentage"))
		if self:GetParent():GetValue("mana") <= 0 then
			local targetUnit = Unit({ entityId = self:GetParent():GetId()})
			self.stunModifier = targetUnit:AddNewModifier(self:GetCaster(), self, "modifier_stun", {duration = self:GetAbility():GetSpecialValue("stun_duration")})
		end
	end
end

return modifier_mana_leak_debuff