require "scripts.modifier";

modifier_status_root = class(Modifier)

LinkLuaModifier("modifier_status_root", modifier_status_root)

function modifier_status_root:GetStatus ()
	return {
		UNIT_STATUS_ROOTED
	}
end

return modifier_status_root
