require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_life_steal"
require "gamemode.Spark.modifiers.modifier_agility"
require "gamemode.Spark.modifiers.modifier_will"
require "gamemode.Spark.modifiers.modifier_damage"

Dread_Masque = class(Item)

function Dread_Masque:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.IMMEDIATE,CastingBehavior.PASSIVE));
end

function Dread_Masque:OnSpellStart()
	local caster = self:GetCaster()
	
	caster:AddNewModifier(caster, self, "modifier_life_steal", {duration = self:GetSpecialValue("duration")})
end

function Dread_Masque:GetModifiers ()
	return {
		"modifier_life_steal",
		"modifier_agility",
		"modifier_will",
		"modifier_damage"
	}
end

return Dread_Masque