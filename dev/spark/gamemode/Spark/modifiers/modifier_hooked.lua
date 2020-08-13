require "scripts.modifier";

modifier_hooked = class(Modifier)

LinkLuaModifier("modifier_hooked", modifier_hooked)

function modifier_hooked:OnCreated ()
	Debug.Log('I was created? Cool!')
end

function modifier_hooked:GetStatus ()
	return {
		UNIT_STATUS_STUN
	}
end

return modifier_hooked
