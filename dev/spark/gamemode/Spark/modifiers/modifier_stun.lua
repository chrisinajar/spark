require "scripts.modifier";

modifier_stun = class(Modifier)

LinkLuaModifier("modifier_stun", modifier_stun)

function modifier_stun:OnCreated ()
	Debug.Log('I was created? Cool!')
	self:SetVisible(true)
end

function modifier_stun:GetStatus ()
	return {
		UNIT_STATUS_STUN
	}
end

return modifier_stun