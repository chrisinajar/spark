require "scripts.core.ability"
require "gamemode.hookwars.modifiers.modifier_hook_damage";

HookDamage = class(Ability)

function HookDamage:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function HookDamage:GetModifiers ()
	return {
		"modifier_hook_damage"
	}
end

return HookDamage
