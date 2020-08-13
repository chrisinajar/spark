require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_hp_regen"
require "gamemode.Spark.modifiers.modifier_mana_regen"

Bottle = class(Item)

function Bottle:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.IMMEDIATE));
end

function Bottle:OnSpellStart()
	local caster = self:GetCaster()

	self.healModifier = caster:AddNewModifier(caster, self, "modifier_hp_regen", {duration = self:GetSpecialValue("duration")})
	self.manaModifier = caster:AddNewModifier(caster, self, "modifier_mana_regen", {duration = self:GetSpecialValue("duration")})
end

return Bottle
