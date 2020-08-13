require "scripts.core.ability"
require "gamemode.Spark.modifiers.modifier_ability_power";

local spell_power_rune = class(Ability)


function spell_power_rune:OnCreated ()
	self:RegisterVariable("max_range", 0.5)
	self:RegisterVariable("cast_time", 0.1)
	self:RegisterVariable("cast_point", 0.0)

	self:SetCastingBehavior(CastingBehavior(CastingBehavior.UNIT_TARGET));
	self:SetLevel(1)
end

function spell_power_rune:OnSpellStart ()    
	self.abilityModifier = self:GetCaster():AddNewModifier(self:GetCaster(), self, "modifier_ability_power", {duration = self:GetSpecialValue("spell_power_duration")})

	GameManagerRequestBus.Broadcast.DestroyEntity(GetId(self:GetCursorTarget()))
	self:DetachAndDestroy();
end

return spell_power_rune
