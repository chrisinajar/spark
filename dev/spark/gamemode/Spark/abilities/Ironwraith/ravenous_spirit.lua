require "scripts.core.ability"
require "gamemode.Spark.modifiers.modifier_ravenous_spirit";

RavenousSpirit = class(Ability)

local Z_OFFSET = 3

function RavenousSpirit:OnCreated ()
	self:RegisterVariable("max_range", self:GetSpecialValue("range"))
	self:RegisterVariable("range", self:GetSpecialValue("range"))
	self:RegisterVariable("bonus_strength", self:GetSpecialValue("bonus_strength"))	
	
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.PASSIVE))

	self.currentStack = nil;
end

function RavenousSpirit:OnAttached()
	self.Caster = self:GetCaster()
	
	self.UnitsNotificationsHandler = UnitsNotificationBus.Connect(self)
end

function RavenousSpirit:OnUnitDeath(UnitId)
	if self:GetCaster():FindModifierByTypeId("modifier_ravenous_spirit") and HasTag(UnitId,"hero") and UnitRequestBus.Event.GetTeamId(UnitId) ~= UnitRequestBus.Event.GetTeamId(self:GetCaster():GetId())then
		local modifier = self:GetCaster():FindModifierByTypeId("modifier_ravenous_spirit")
		local unit=Unit({entityId=UnitId})
		local origin = self:GetCaster():GetPosition()
		local direction = (unit:GetPosition() - origin)
		direction.z = 0
		origin.z = origin.z + Z_OFFSET
		direction = direction:GetNormalized()
		
		local destination = origin + (direction * self:GetValue("range"))		
		if direction.x * self:GetValue("range") <= self:GetValue("range") or direction.y * self:GetValue("range") <= self:GetValue("range") then

			self.currentStack = modifier:GetValue("current_stacked_amount")
			modifier:SetValue("current_stacked_amount", self.currentStack + 1)			
		end		
	end
end

function RavenousSpirit:GetModifiers ()
	return {
		"modifier_ravenous_spirit"
	}
end

return RavenousSpirit