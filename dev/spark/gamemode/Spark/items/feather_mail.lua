require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_aura_movement_speed"
require "gamemode.Spark.modifiers.modifier_aura_heal"
require "gamemode.Spark.modifiers.modifier_armor"
require "gamemode.Spark.modifiers.modifier_strength"

FeatherMail = class(Item)

function FeatherMail:OnCreated ()
	self:RegisterVariable("max_range", 200)
	self:RegisterVariable("range", self:GetSpecialValue("range"))	

	self:SetCastingBehavior(CastingBehavior(CastingBehavior.IMMEDIATE,CastingBehavior.PASSIVE));
	
	self.Toggled = false;
end

function FeatherMail:OnSpellStart()
	local caster = self:GetCaster()
	if self.Toggled == false then
		self.Toggled = true
		if self.moveSpeedEffect then
			self.moveSpeedEffect:Destroy()
			self.moveSpeedEffect = nil
		end	
		self.healEffect = caster:AddNewModifier(caster, self, "modifier_aura_heal", {})		
	elseif self.Toggled == true then
		self.Toggled = false
		if self.healEffect then
			self.healEffect:Destroy()
			self.healEffect = nil
		end	
		self.moveSpeedEffect = caster:AddNewModifier(caster, self, "modifier_aura_movement_speed", {})		
	end
end

function FeatherMail:GetModifiers ()
	return {
		"modifier_armor",
		"modifier_strength"
	}
end

return FeatherMail
