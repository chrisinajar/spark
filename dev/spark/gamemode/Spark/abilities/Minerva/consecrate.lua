require "scripts.core.ability"
require "scripts.core.projectile"
require "gamemode.Spark.modifiers.modifier_damage_block"

Consecrate = class(Ability)

function Consecrate:OnCreated ()
	self:RegisterVariable("max_range", self:GetSpecialValue("range"))
	self:RegisterVariable("cast_time", 0.0)
	self:RegisterVariable("cast_point", 0.6)
	self:RegisterVariable("range", self:GetSpecialValue("range"))

	self:SetCastingBehavior(CastingBehavior(CastingBehavior.UNIT_TARGET));

end

function Consecrate:OnSpellStart ()
	Debug.Log('Firing Consecrate!')

	local target = self:GetCursorTarget()
	local caster = self:GetCaster()
	
	if UnitRequestBus.Event.GetTeamId(target:GetId()) == UnitRequestBus.Event.GetTeamId(caster:GetId()) then
		-- hard dispell
		target:ApplyDispel(Dispel())
		target:AddNewModifier(caster, self, "modifier_damage_block", {duration = self:GetSpecialValue("duration")})
	end
end

return Consecrate