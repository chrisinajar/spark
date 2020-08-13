require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_reduce_attack_speed"
require "gamemode.Spark.modifiers.modifier_hp"
require "gamemode.Spark.modifiers.modifier_mana"
require "gamemode.Spark.modifier.modifier_movement_speed"

Arctic_Talon = class(Item)

function Arctic_Talon:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
	self:ListenToAttackEvent("OnAttackLanded");
end

function modifier_life_steal:OnAttackLanded(AttackInfo)
	if AttackInfo.attacked ~= self:GetParent():GetId() and AttackInfo.attacked~=nil then
		AttackInfo.attacked:AddNewModifier(AttackInfo.attacked, self, "modifier_movement_speed", {duration = self:GetSpecialValue("duration")})
		AttackInfo.attacked:AddNewModifier(AttackInfo.attacked, self, "modifier_reduce_attack_speed", {duration = self:GetSpecialValue("duration")})
	end
end

function Arctic_Talon:GetModifiers ()
	return {
		"modifier_attack_speed",
		"modifier_hp",
		"modifier_mana"
	}
end

return Arctic_Talon