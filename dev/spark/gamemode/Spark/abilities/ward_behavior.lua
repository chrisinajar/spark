require "scripts.core.ability"

ward_behavior = class(Ability)

function ward_behavior:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function ward_behavior:GetModifiers ()
	return {
		"modifier_building_behavior",
		"modifier_ward_behavior"
	}
end

return ward_behavior
