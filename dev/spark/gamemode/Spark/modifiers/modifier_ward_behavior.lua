require "scripts.modifier";

modifier_ward_behavior = class(Modifier)

LinkLuaModifier("modifier_ward_behavior", modifier_ward_behavior)

function modifier_ward_behavior:GetStatus ()
	return {
		UNIT_STATUS_STUN,
		UNIT_STATUS_STEALTH
	}
end

function modifier_ward_behavior:IsDispellable(dispel)
    return false
end


return modifier_ward_behavior
