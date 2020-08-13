require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_armor"
require "gamemode.Spark.modifiers.modifier_all_stats";
require "gamemode.Spark.modifiers.modifier_attack_speed";
require "gamemode.Spark.modifiers.modifier_aura_movement_speed";

AegisOfTheHound = class(Item)

function AegisOfTheHound:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE,CastingBehavior.IMMEDIATE));
end

function AegisOfTheHound:OnSpellStart()
	local caster = self:GetCaster()
	
	caster:AddNewModifier(caster, self, "modifier_aura_movement_speed", {duration = self:GetSpecialValue("duration")})
end

function AegisOfTheHound:GetModifiers ()
	return {
		"modifier_armor",
		"modifier_all_stats",
		"modifier_aura_attack_speed"
	}
end

return AegisOfTheHound