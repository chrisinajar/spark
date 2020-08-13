require "scripts.core.ability"
require "gamemode.Spark.modifiers.modifier_thrill_seeker";

Thrill_Seeker = class(Ability)

function Thrill_Seeker:OnCreated ()
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE))
	
	self.UnitsNotificationsHandler = UnitsNotificationBus.Connect(self)
end

function Thrill_Seeker:GetModifiers ()
	return {
		"modifier_thrill_seeker"
	}
end

return Thrill_Seeker