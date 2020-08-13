require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_life_steal"
require "gamemode.Spark.modifiers.modifier_agility"
require "gamemode.Spark.modifiers.modifier_will"

Parasite_Dynamo = class(Item)

function Parasite_Dynamo:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function Parasite_Dynamo:GetModifiers ()
	return {
		"modifier_life_steal",
		"modifier_agility",
		"modifier_will"
	}
end

return Parasite_Dynamo