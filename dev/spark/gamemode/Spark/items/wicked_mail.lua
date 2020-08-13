require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_armor"
require "gamemode.Spark.modifiers.modifier_strength";
require "gamemode.Spark.modifiers.modifier_agility";
require "gamemode.Spark.modifiers.modifier_damage_return";

WickedMail = class(Item)

function WickedMail:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.IMMEDIATE,CastingBehavior.PASSIVE));
end

function WickedMail:OnSpellStart()
	local caster = self:GetCaster()
	
	caster:AddNewModifier(caster, self, "modifier_damage_return", {duration = self:GetSpecialValue("duration")})
end

function WickedMail:GetModifiers ()
	return {
		"modifier_armor",
		"modifier_strength",
		"modifier_agility"
	}
end

return WickedMail