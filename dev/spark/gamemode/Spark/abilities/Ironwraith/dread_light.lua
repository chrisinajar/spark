require "scripts.core.ability"

DreadLight = class(Ability)

function DreadLight:OnCreated ()
	self:RegisterVariable("max_range", self:GetSpecialValue("range"))
	self:RegisterVariable("range", self:GetSpecialValue("range"))

	self:SetCastingBehavior(CastingBehavior(CastingBehavior.IMMEDIATE));
	
	self.Toggled = false;
end

function DreadLight:OnSpellStart()
	local caster = self:GetCaster()
	if self.Toggled == false then
		self.Toggled = true
		self.damageEffect = caster:AddNewModifier(caster, self, "modifier_ironwraith_dread_light", {})		
	elseif self.Toggled == true then
		self.Toggled = false
		if self.damageEffect then
			self.damageEffect:Destroy()
			self.damageEffect = nil
		end		
	end
end

return DreadLight
