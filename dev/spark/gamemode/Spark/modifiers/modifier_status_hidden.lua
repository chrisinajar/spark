require "scripts.modifier";

modifier_status_hidden = class(Modifier)

LinkLuaModifier("modifier_status_hidden", modifier_status_hidden)

function modifier_status_hidden:GetStatus ()
	return {
		UNIT_STATUS_HIDDEN
	}
end

return modifier_status_hidden
