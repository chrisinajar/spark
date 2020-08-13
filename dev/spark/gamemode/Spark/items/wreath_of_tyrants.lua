require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_status_resistance"
require "gamemode.Spark.modifiers.modifier_strength"
require "gamemode.Spark.modifiers.modifier_hp"
require "gamemode.Spark.modifiers.modifier_status_disarm"


Wreath_Of_Tyrants = class(Item)

function Wreath_Of_Tyrants:OnCreated ()		
	self:RegisterVariable("max_range",500)
	self:RegisterVariable("cast_time",0.1)
	self:RegisterVariable("cast_point",0.1);
	
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.UNIT_TARGET,CastingBehavior.PASSIVE));
end

function Wreath_Of_Tyrants:OnSpellStart()
	local caster = self:GetCaster()
	
	self.attachedUnit = self:GetCursorTarget()

	self.stoneModifier = self.attachedUnit:AddNewModifier(caster, self, "modifier_status_disarm", {duration = self:GetSpecialValue("duration")})
end

function Wreath_Of_Tyrants:GetModifiers ()
	return {
		"modifier_status_resistance",
		"modifier_strength",
		"modifier_hp"
	}
end

return Wreath_Of_Tyrants