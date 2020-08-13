require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_armor"
require "gamemode.Spark.modifiers.modifier_strength"
require "gamemode.Spark.modifiers.modifier_will"
require "gamemode.Spark.modifiers.modifier_damage"
require "gamemode.Spark.modifiers.modifier_aura_take_damage";

BlightedCuirass = class(Item)

function BlightedCuirass:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
	
	local caster = self:GetCaster()
	
	caster:AddNewModifier(caster, self, "modifier_aura_take_damage", {})
end

function BlightedCuirass:GetModifiers ()
	return {
		"modifier_armor",
		"modifier_strength",
		"modifier_will",
		"modifier_damage"
		--"modifier_aura_take_damage"
	}
end

return BlightedCuirass