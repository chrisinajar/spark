require "scripts.core.ability"

building_behavior = class(Ability)

function building_behavior:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function building_behavior:GetModifiers ()
	return {
		"modifier_building_behavior"
	}
end

return building_behavior
