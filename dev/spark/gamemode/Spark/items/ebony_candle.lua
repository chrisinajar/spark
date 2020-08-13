require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_intelligence"
require "gamemode.Spark.modifiers.modifier_mana_regen"
require "gamemode.Spark.modifiers.modifier_mana"
require "gamemode.Spark.modifiers.modifier_status_silence"


Ebony_Candle = class(Item)

function Ebony_Candle:OnCreated ()		
	self:RegisterVariable("max_range",500)
	self:RegisterVariable("cast_time",0.1)
	self:RegisterVariable("cast_point",0.1);
	
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.UNIT_TARGET,CastingBehavior.PASSIVE));
end

function Ebony_Candle:OnSpellStart()
	local caster = self:GetCaster()
	
	self.attachedUnit = self:GetCursorTarget()

	self.stoneModifier = self.attachedUnit:AddNewModifier(caster, self, "modifier_status_silence", {duration = self:GetSpecialValue("duration")})
end

function Ebony_Candle:GetModifiers ()
	return {
		"modifier_intelligence",
		"modifier_mana_regen",
		"modifier_mana"
	}
end

return Wreath_Of_Tyrants