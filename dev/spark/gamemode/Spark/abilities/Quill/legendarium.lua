require "scripts.core.ability"
require "scripts.core.projectile"
--require "gamemode.Spark.modifiers.modifier_runic_burst";

local DebugRenderer = Require("debugRenderer")

Legendarium = class(Ability)

function Legendarium:OnCreated ()
	self:RegisterVariable("max_range", self:GetSpecialValue("range"))
	self:RegisterVariable("cast_time", 0.0)
	self:RegisterVariable("cast_point", 1.5)
	self:RegisterVariable("range", self:GetSpecialValue("range"))
	
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.POINT_TARGET))
	
	self.UnitsNotificationsHandler = UnitsNotificationBus.Connect(self)
end

function Legendarium:OnSpellStart()
	local target = self:GetCursorPosition()
	local caster = self:GetCaster()
	local origin = self:GetCursorPosition()
	
	--[[CreateTimer(function()
		local duration = self:GetSpecialValue("duration")
		local radius = self:GetSpecialValue("aoe")
		
		self.thinker = ModifiersHandler:CreateModifierThinker(caster,self,"modifier_legendarium",{duration=duration,radius=20},target)
		
		AttachParticleSystem(self.thinker, "heroes.astromage.w_quasar")
		DebugRenderer.DrawAura(self.thinker,Color(1,0,0,0.4),duration)
		
		self.TriggerArea = TriggerAreaNotificationBus.Connect(self, GetId(self.thinker))
	end,0.5);]]

end

function Legendarium:OnTriggerAreaEntered(unitId)

end

function Legendarium:OnTriggerAreaExited(entityId)
	if entityId == self:GetCaster():GetId() then
		self.thinker:Destroy()
		GameEntityContextRequestBus.Broadcast.DestroyGameEntityAndDescendants(GetId(self.thinker))
		self.thinker = nil
		self.TriggerArea:Disconnect()
		self.TriggerArea = nil
		Debug.Log("LOOK HERE FOR ON TRIGGER AREA EXITED !!")
	end
end

function Legendarium:OnDeactivate()
	if self.TriggerArea then
		self.TriggerArea:Disconnect()
		self.TriggerArea = nil
	end
end


return Legendarium