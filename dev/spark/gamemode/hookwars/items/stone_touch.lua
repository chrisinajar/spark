require "scripts.core.item"
require "gamemode.hookwars.modifiers.modifier_Stone_Touch";

Stone_Touch = class(Item)

function Stone_Touch:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.UNIT_TARGET));
	
	self:RegisterVariable("max_range",1000)
	self:RegisterVariable("cast_time",0.1)
	self:RegisterVariable("cast_point",0.1);
end

function Stone_Touch:OnSpellStart()
	local caster = self:GetCaster()
	
	self.attachedUnit = self:GetCursorTarget()

	self.stoneModifier = self.attachedUnit:AddNewModifier(caster, self, "modifier_Stone_Touch", {duration = self:GetSpecialValue("stun_duration")})
end

--[[function Stone_Touch:GetModifiers ()
	return {
		"modifier_Stone_Touch"
	}
end]]

return Stone_Touch