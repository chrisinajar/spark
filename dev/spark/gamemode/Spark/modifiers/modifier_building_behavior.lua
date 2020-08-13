require "scripts.modifier";

modifier_building_behavior = class(Modifier)

LinkLuaModifier("modifier_building_behavior", modifier_building_behavior)

function modifier_building_behavior:GetStatus ()
	return {
		UNIT_STATUS_ROOTED,
		UNIT_STATUS_SPELL_IMMUNITY
	}
end

function modifier_building_behavior:IsDispellable(dispel)
    return false
end


return modifier_building_behavior
