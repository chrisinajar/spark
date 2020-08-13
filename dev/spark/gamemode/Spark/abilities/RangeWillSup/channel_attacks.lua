require "scripts.core.ability"
Require("GameUtils")

-- Stun Bolt ability
ChannelAttacks = class(Ability)

function ChannelAttacks:OnCreated ()
	self:RegisterVariable("max_range", 100)
	self:RegisterVariable("cast_time", 0.0)
	self:RegisterVariable("cast_point", 0.6)
	self:RegisterVariable("range", 100)

	self:SetCastingBehavior(CastingBehavior(CastingBehavior.POINT_TARGET));
end

function ChannelAttacks:OnSpellStart ()
	Debug.Log("placing ult")
	local position = self:GetCursorPosition()
	local caster   = self:GetCaster()
	local ward = CreateUnit("ult_ward")
	
	--TransformBus.Event.SetLocalRotation(GetId(ward),TransformBus.Event.GetWorldRotation(GetId(caster)))
	ward:SetRotationZ(caster:GetRotationZ())
	ward:SetPosition(position)
	ward:SetTeamId(caster:GetTeamId())

end


return ChannelAttacks
