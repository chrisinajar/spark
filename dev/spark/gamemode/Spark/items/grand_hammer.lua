require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_damage"

Grand_Hammer = class(Item)

function Grand_Hammer:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function Grand_Hammer:GetModifiers ()
	return {
		"modifier_damage"		
	}
end

return Grand_Hammer