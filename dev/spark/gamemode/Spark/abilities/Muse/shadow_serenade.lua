require "scripts.core.ability"
--require "gamemode.Spark.modifiers.modifier_reduce_cooldown";

ShadowSerenade = class(Ability)

function ShadowSerenade:OnCreated()
	self:RegisterVariable("cast_time", 0.0)
	self:RegisterVariable("cast_point", 0.0)
	self:RegisterVariable("max_range", 1.0)
	
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.IMMEDIATE))
end

function ShadowSerenade:OnSpellStart()
	local caster = self:GetCaster()
	-- Do Invis
	self.shadowModifier = caster:AddNewModifier(caster, self, "modifier_shadow_serenade",{duration = self:GetSpecialValue("duration")})
end

return ShadowSerenade
