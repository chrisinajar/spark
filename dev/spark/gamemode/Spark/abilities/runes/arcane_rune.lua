require "scripts.core.ability"
require "gamemode.Spark.modifiers.modifier_arcane";

local arcane_rune = class(Ability)


function arcane_rune:OnCreated ()
	self:RegisterVariable("max_range", 0.5)
	self:RegisterVariable("cast_time", 0.1)
	self:RegisterVariable("cast_point", 0.0)

	self:SetCastingBehavior(CastingBehavior(CastingBehavior.UNIT_TARGET));
	self:SetLevel(1)
end

function arcane_rune:OnSpellStart ()    
	self.arcaneModifier = self:GetCaster():AddNewModifier(self:GetCaster(), self, "modifier_arcane", {duration = self:GetSpecialValue("arcane_duration")})

	GameManagerRequestBus.Broadcast.DestroyEntity(GetId(self:GetCursorTarget()))
	self:DetachAndDestroy();
end

return arcane_rune
