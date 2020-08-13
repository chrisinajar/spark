require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_armor"
require "gamemode.Spark.modifiers.modifier_will"
require "gamemode.Spark.modifiers.modifier_intelligence"

Hallowed_Bridandine = class(Item)

function Hallowed_Bridandine:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function Hallowed_Bridandine:GetModifiers ()
	return {
		"modifier_armor",
		"modifier_will",
		"modifier_intelligence"
	}
end

return Hallowed_Bridandine