require "scripts.core.ability"
require "gamemode.Spark.modifiers.modifier_critical_strike";

Murder = class(Ability)

function Murder:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.IMMEDIATE));
end

function Murder:OnSpellStart()
	local caster = self:GetCaster()
	
	if not caster:FindModifierByTypeId("modifier_murder") then
		self.critModifier = caster:AddNewModifier(caster, self, "modifier_murder", {})
	end
end

return Murder
