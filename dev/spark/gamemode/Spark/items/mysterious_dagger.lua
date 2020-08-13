require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_attack_speed"
require "gamemode.Spark.modifiers.modifier_damage"

Mysterious_Dagger = class(Item)

function Mysterious_Dagger:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
	self:ListenToAttackEvent("OnAttackLanded");
end

function Mysterious_Dagger:OnAttackLanded(AttackInfo)
	if AttackInfo.attacked ~= self:GetParent():GetId() and AttackInfo.attacked~=nil then
		--Put chance for magic damage on attack here
	end
end

function Mysterious_Dagger:GetModifiers ()
	return {
		"modifier_damage",
		"modifier_attack_speed"		
	}
end

return Mysterious_Dagger