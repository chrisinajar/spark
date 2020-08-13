require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_hp_potion";

Health_Potion = class(Item)

function Health_Potion:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.UNIT_TARGET));
	
	self:RegisterVariable("max_range",1000)
	self:RegisterVariable("cast_time",0.1)
	self:RegisterVariable("cast_point",0.1);
end

function Health_Potion:OnSpellStart()
	local caster = self:GetCaster()
	
	self.attachedUnit = self:GetCursorTarget()
	
	self.potionModifier = self.attachedUnit:AddNewModifier(caster, self, "modifier_hp_potion", {duration = self:GetSpecialValue("duration")})

	self:DetachAndDestroy()
end


return Health_Potion
