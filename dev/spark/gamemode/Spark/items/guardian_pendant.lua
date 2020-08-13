require "scripts.core.item"
require "gamemode.Spark.modifiers.modifier_magic_armor";

GuardianPendant = class(Item)

function GuardianPendant:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE));
end

function GuardianPendant:GetModifiers ()
	return {
		"modifier_magic_armor"
	}
end

return GuardianPendant