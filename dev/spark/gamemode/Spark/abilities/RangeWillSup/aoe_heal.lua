require "scripts.core.ability"
require "gamemode.Spark.modifiers.modifier_aura_heal"
require "gamemode.Spark.modifiers.modifier_drain_mana_per_second";

Aoe_Heal = class(Ability)

function Aoe_Heal:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.IMMEDIATE))
	
	self.Toggled = false;
end

function Aoe_Heal:OnSpellStart ()
	local caster = self:GetCaster()
	if self.Toggled == false then
		self.Toggled = true
		self.healEffect = caster:AddNewModifier(caster, self, "modifier_aura_heal", {})
		self.manaDrainEffect = caster:AddNewModifier(caster, self, "modifier_drain_mana_per_second", {})
	elseif self.Toggled == true then
		self.Toggled = false
		if self.healEffect then
			self.healEffect:Destroy()
			self.healEffect = nil
		end
		if self.manaDrainEffect then
			self.manaDrainEffect:Destory()
			self.manaDrainEffect = nil
		end
	end
end

return Aoe_Heal
