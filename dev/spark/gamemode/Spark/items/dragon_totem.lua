require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_hp"
require "gamemode.Spark.modifiers.modifier_all_stats"
require "gamemode.Spark.modifiers.modifier_mana"

Dragon_Totem = class(Item)

function Dragon_Totem:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
	-- need to add talent stuff when in here !
end

function Dragon_Totem:GetModifiers ()
	return {
		"modifier_hp",
		"modifier_all_stats",
		"modifier_mana"
	}
end

return Dragon_Totem