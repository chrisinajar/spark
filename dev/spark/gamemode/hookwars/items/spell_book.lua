require "scripts.core.item"
require "gamemode.hookwars.modifiers.modifier_Spell_Book";

Spell_Book = class(Item)

function Spell_Book:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function Spell_Book:GetModifiers ()
	return {
		"modifier_Spell_Book"
	}
end

return Spell_Book