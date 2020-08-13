require "scripts.core.ability"
require "gamemode.hookwars.modifiers.modifier_hook_speed";

HookSpeed = class(Ability)

function HookSpeed:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function HookSpeed:GetModifiers ()
	return {
		"modifier_hook_speed"
	}
end

return HookSpeed