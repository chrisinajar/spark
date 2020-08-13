require "scripts.modifier";

modifier_line_stun = class(Modifier)

LinkLuaModifier("modifier_line_stun", modifier_line_stun)

function modifier_line_stun:OnCreated ()
	Debug.Log('I was created? Cool!')
	self:SetVisible(true)
end

function modifier_line_stun:GetStatus ()
	return {
		UNIT_STATUS_STUN
	}
end

return modifier_line_stun