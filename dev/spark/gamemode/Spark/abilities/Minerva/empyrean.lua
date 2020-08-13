require "scripts.core.ability"
--require "gamemode.Spark.modifiers.modifier_"

Empyrean = class(Ability)

function Empyrean:OnCreated ()
	self:RegisterVariable("cast_time", 0.0)
	self:RegisterVariable("cast_point", 0.0)
	
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.IMMEDIATE, CastingBehavior.PASSIVE));
end

function Empyrean:OnSpellStart ()
	Debug.Log('Firing Empyrean!')

	local caster = self:GetCaster()
	
	caster:AddNewModifier(caster, self, "modifier_empyrean", {duration = self:GetSpecialValue("duration")})
end

function Empyrean:GetModifiers ()
	return {
		--"modifier_empyrean"
	}
end

return Empyrean