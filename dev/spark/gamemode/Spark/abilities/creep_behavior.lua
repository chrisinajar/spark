require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_creep_behavior";

creep_behavior = class(Item)

function creep_behavior:OnCreated ()
    self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
    Debug.Log("creep_behavior:OnCreated")
    self:SetLevel(1);
end

function creep_behavior:GetModifiers ()
    Debug.Log("creep_behavior:GetModifiers")
	return {
		"modifier_creep_behavior"
	}
end

return creep_behavior
