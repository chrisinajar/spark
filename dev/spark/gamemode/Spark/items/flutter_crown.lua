require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_attack_speed";

Flutter_Crown = class(Item)

function Flutter_Crown:OnCreated()
	self:RegisterVariable("cast_time", 0.0)
	self:RegisterVariable("cast_point", 0.0)
	self:RegisterVariable("max_range", self:GetSpecialValue("range"))
	self:RegisterVariable("range", self:GetSpecialValue("range"))
	Debug.Log(self:GetName().." Flutter_Crown:OnCreated!");
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.POINT_TARGET))
end

function Flutter_Crown:OnSpellStart()
	--AudioRequestBus.Broadcast.PlaySound("Play_sfx_blink_vanish");
	local target = self:GetCursorPosition()
	local caster = self:GetCaster()	
	local origin = self:GetCaster():GetPosition()
	
	caster:SetPosition(target)
	AudioRequestBus.Broadcast.PlaySound("Play_sfx_blink_appear");
	
	local caster = self:GetCaster()
	self.attackSpeedModifier = caster:AddNewModifier(caster, self, "modifier_attack_speed", {duration = self:GetSpecialValue("duration")})
end

return Flutter_Crown
