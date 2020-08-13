require "scripts.core.item"
require "gamemode.hookwars.modifiers.modifier_push";

PushStick = class(Item)

function PushStick:OnCreated ()
	self:RegisterVariable("cast_time", 0.0)
	self:RegisterVariable("cast_point", 0.0)
	self:RegisterVariable("max_range", self:GetSpecialValue("range"))
	self:RegisterVariable("range", self:GetSpecialValue("range"))	
	self:RegisterVariable("speed", self:GetSpecialValue("speed"))
	self:RegisterVariable("push_distance", self:GetSpecialValue("push_distance"))
	
	self:SetCastingBehavior(CastingBehavior(CastingBehavior.UNIT_TARGET))
end

function PushStick:OnSpellStart ()
	local caster = self:GetCaster()
	local target = self:GetCursorTarget()
	local speed = self:GetValue('speed')

	target:AddNewModifier(caster, self, "modifier_push", {
		duration = self:GetValue('push_distance') / speed,
		speed = speed
	})
end

-- function PushStick:OnTick (deltaTime)
-- 	if not self.attachedUnit or not self.attachedUnit:IsAlive() then
-- 		self.tickHandler:Disconnect()
-- 		self.tickHandler = nil
-- 		self.attachedUnit = nil
-- 		self.hookedModifier = nil
-- 		return
-- 	end
-- 	Debug.Log(tostring(deltaTime))
-- 	Debug.Log(tostring(self.attachedUnit:GetPosition()))
-- 	Debug.Log(tostring(self:GetValue("speed")))
-- 	Debug.Log(tostring(self.direction))
-- 	local origin = self.attachedUnit:GetPosition()
-- 	--origin.z = origin.z - Z_OFFSET 
-- 	origin.x = (origin.x + self:GetValue("speed")*self.direction.x*deltaTime)
-- 	origin.y = (origin.y + self:GetValue("speed")*self.direction.y*deltaTime)
-- 	--local update = origin + (self:GetValue("speed"))
-- 	Debug.Log(tostring(origin))
-- 	self.attachedUnit:SetPosition(origin)
	
-- 	if origin >= self.destination then
-- 		self.tickHandler:Disconnect()
-- 		self.tickHandler = nil
-- 		self.attachedUnit = nil
-- 		self.pushModifier:Destroy()
-- 		self.hookingModifier = nil
-- 	end
-- end

return PushStick
