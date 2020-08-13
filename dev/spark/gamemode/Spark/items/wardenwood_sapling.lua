require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_hp_regen"
require "gamemode.Spark.modifiers.modifier_armor"
require "gamemode.Spark.modifiers.modifier_mana_regen"
require "gamemode.Spark.modifiers.modifier_magic_armor"

Wardenwood_Sapling = class(Item)

function Wardenwood_Sapling:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.POINT_TARGET,CastingBehavior.PASSIVE));
	-- need to add ward stuff when in here !
end

function Wardenwood_Sapling:GetModifiers ()
	return {
		"modifier_hp_regen",
		"modifier_armor",
		"modifier_mana_regen",
		"modifier_magic_armor"
	}
end

return Wardenwood_Sapling