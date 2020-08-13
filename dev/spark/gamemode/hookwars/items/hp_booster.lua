require "scripts.core.item"
require "gamemode.hookwars.modifiers.modifier_hp_booster";

HpBooster = class(Item)

function HpBooster:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function HpBooster:GetModifiers ()
	return {
		"modifier_hp_booster"
	}
end

return HpBooster