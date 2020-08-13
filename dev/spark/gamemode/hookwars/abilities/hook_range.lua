require "scripts.core.ability"
require "gamemode.hookwars.modifiers.modifier_hook_range";

HookRange = class(Ability)

function HookRange:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function HookRange:GetModifiers ()
	return {
		"modifier_hook_range"
	}
end

return HookRange