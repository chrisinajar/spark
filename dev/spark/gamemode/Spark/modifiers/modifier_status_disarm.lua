require "scripts.modifier";

modifier_status_disarm = class(Modifier)

LinkLuaModifier("modifier_status_disarm", modifier_status_disarm)

function modifier_status_disarm:GetStatus ()
	return {
		UNIT_STATUS_DISARM
	}
end

return modifier_status_disarm
