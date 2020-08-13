require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_push"
require "gamemode.Spark.modifiers.modifier_mana_regen"
require "gamemode.Spark.modifiers.modifier_intelligence"
require "gamemode.Spark.modifiers.modifier_spell_life_steal"

Tempest_Satchel = class(Item)

function Tempest_Satchel:OnCreated ()
	self:RegisterVariable("cast_time", 0.0)
	self:RegisterVariable("cast_point", 0.0)
	self:RegisterVariable("range", 200)
	self:RegisterVariable("max_range", self:GetValue("range"))		
	self:RegisterVariable("speed", 40)
	self:RegisterVariable("push_distance", self:GetSpecialValue("push_distance"))
	
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.UNIT_TARGET))
end

function Tempest_Satchel:OnSpellStart ()
	local caster = self:GetCaster()
	local target = self:GetCursorTarget()
	local speed = self:GetValue('speed')

	target:AddNewModifier(caster, self, "modifier_push", {
		duration = self:GetValue('push_distance') / speed,
		speed = speed
	})
end

function Tempest_Satchel:GetModifiers ()
	return {
		"modifier_mana_regen",
		"modifier_intelligence",
		"modifier_spell_life_steal"		
	}
end

return Tempest_Satchel
