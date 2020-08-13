require "scripts.modifier";

modifier_hooking = class(Modifier)

LinkLuaModifier("modifier_hooking", modifier_hooking)

function modifier_hooking:GetStatus ()
	return {
		UNIT_STATUS_ROOTED
	}
end

return modifier_hooking
