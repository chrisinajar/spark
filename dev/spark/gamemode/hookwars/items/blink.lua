require "scripts.core.item"

Blink = class(Item)

function Blink:OnCreated()
	self:RegisterVariable("cast_time", 0.0)
	self:RegisterVariable("cast_point", 0.0)
	self:RegisterVariable("max_range", self:GetSpecialValue("range"))
	self:RegisterVariable("range", self:GetSpecialValue("range"))
	Debug.Log(self:GetName().." Blink:OnCreated!");
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.POINT_TARGET))
end

function Blink:OnSpellStart()
	--AudioRequestBus.Broadcast.PlaySound("Play_sfx_blink_vanish");
	Debug.Log('Firing Blink!')
	local target = self:GetCursorPosition()
	Debug.Log("Destination is " .. tostring(target))
	local caster = self:GetCaster()
	--Debug.Log("get casting is : " .. tostring(self:GetCaster():GetId()) .. caster:GetName())
	
	local origin = self:GetCaster():GetPosition()
	Debug.Log('Origin of Blink is ' .. tostring(origin) .. ' : ' .. tostring(self:GetCaster():GetId()) .. caster:GetName())
	
	caster:SetPosition(target)
	AudioRequestBus.Broadcast.PlaySound("Play_sfx_blink_appear");
end

return Blink
