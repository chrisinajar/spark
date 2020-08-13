require "scripts.modifier";

modifier_Stone_Touch = class(Modifier)

LinkLuaModifier("modifier_Stone_Touch", modifier_Stone_Touch)

function modifier_Stone_Touch:OnCreated ()
	Debug.Log('I was created? Cool!')
end

function modifier_Stone_Touch:GetStatus ()
	return {
		UNIT_STATUS_STUN
	}
end

return modifier_Stone_Touch