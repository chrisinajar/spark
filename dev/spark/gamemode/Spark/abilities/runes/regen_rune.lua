require "scripts.core.ability"
require "gamemode.Spark.modifiers.modifier_regen_rune";

local double_damage_rune = class(Ability)


function double_damage_rune:OnCreated ()
	self:RegisterVariable("max_range", 0.5)
	self:RegisterVariable("cast_time", 0.1)
	self:RegisterVariable("cast_point", 0.0)

	self:SetCastingBehavior(CastingBehavior(CastingBehavior.UNIT_TARGET));
	self:SetLevel(1)
end

function double_damage_rune:OnSpellStart ()    
	self.regenModifier = self:GetCaster():AddNewModifier(self:GetCaster(), self, "modifier_regen_rune", {duration = self:GetSpecialValue("regen_duration")})

	GameManagerRequestBus.Broadcast.DestroyEntity(GetId(self:GetCursorTarget()))
	self:DetachAndDestroy();
end

return double_damage_rune
