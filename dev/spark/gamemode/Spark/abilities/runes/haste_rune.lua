require "scripts.core.ability"
require "gamemode.Spark.modifiers.modifier_hasted";

local haste_rune = class(Ability)


function haste_rune:OnCreated ()
	self:RegisterVariable("max_range", 0.5)
	self:RegisterVariable("cast_time", 0.1)
	self:RegisterVariable("cast_point", 0.0)

	self:SetCastingBehavior(CastingBehavior(CastingBehavior.UNIT_TARGET));
	self:SetLevel(1)
end

function haste_rune:OnSpellStart ()    
	self.hasteModifier = self:GetCaster():AddNewModifier(self:GetCaster(), self, "modifier_hasted", {duration = self:GetSpecialValue("haste_duration")})

	GameManagerRequestBus.Broadcast.DestroyEntity(GetId(self:GetCursorTarget()))
	self:DetachAndDestroy();
end

return haste_rune
