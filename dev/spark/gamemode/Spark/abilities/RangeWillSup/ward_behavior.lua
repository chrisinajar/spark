require "scripts.core.ability"

ult_ward_behaviour = class(Ability)

function ult_ward_behaviour:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function ult_ward_behaviour:GetModifiers ()
	return {
		"modifier_building_behavior",
		"modifier_ward_behavior"
	}
end

return ult_ward_behaviour
