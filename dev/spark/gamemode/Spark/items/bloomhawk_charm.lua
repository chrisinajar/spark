require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_hp"
require "gamemode.Spark.modifiers.modifier_attack_speed"
require "gamemode.Spark.modifiers.modifier_movement_speed"

Bloomhawk_Charm = class(Item)

function Bloomhawk_Charm:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
	self:ListenToAttackEvent("OnAttackLanded");
end

function Bloomhawk_Charm:OnAttackLanded(AttackInfo)
	if AttackInfo.attacked ~= self:GetParent():GetId() and AttackInfo.attacked~=nil then
		AttackInfo.attacked:AddNewModifier(AttackInfo.attacked, self, "modifier_movement_speed", {duration = self:GetSpecialValue("duration")})
	end
end

function Bloomhawk_Charm:GetModifiers ()
	return {
		"modifier_hp",
		"modifier_attack_speed"		
	}
end

return Bloomhawk_Charm